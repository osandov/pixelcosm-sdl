#include "PixelCosm.hpp"
#include "SandParser.hpp"

namespace PixelCosm
{

void SandParser::parseTokens()
{
	std::size_t lastpos = 0;
	while (true) {
		std::size_t spacepos = line_.find(" ", lastpos);
		if (spacepos != std::string::npos) {
			tokens_.emplace_back(line_.substr(lastpos, spacepos - lastpos));
			lastpos = spacepos + 1;
		} else if (lastpos != 0) {
			tokens_.emplace_back(line_.substr(lastpos));
			break;
		} else
			break;
	}
}

void SandParser::handleIncludeLine()
{
	if (tokens_.size() != 2) {
		errors_.add({curLine_, "Invalid number of tokens in Include line"});
		return;
	}

	parent_.addToQueue(tokens_.at(1));
}

void SandParser::handleGroupLine()
{
	if (tokens_.size() != 2) {
		errors_.add({curLine_, "Invalid number of tokens in Group line"});
		return;
	}

	Group::param_type params;

	params.name = tokens_.at(1);

	try {
		params.image = app_.loadImage(std::string("resources/groups/") + tokens_.at(1) + ".bmp");
#ifdef DOUBLE_DISPLAY
		SDL_Surface* newimage = tetra::Sdl::ratioSize(params.image, {3, 2});
		SDL_FreeSurface(params.image);
		params.image = newimage;
#endif
	} catch(const tetra::Sdl::Exception& e) {
		params.image = nullptr;
		errors_.add({curLine_, e.what()});
	}

	try {
		Group::create(params);
	} catch (const std::runtime_error& e) {
		errors_.add({curLine_, e.what()});
	}
}

void SandParser::handleSubgroupLine()
{
	if (tokens_.size() != 3) {
		errors_.add({curLine_, "Invalid number of tokens in Subgroup line"});
		return;
	}

	Subgroup::param_type params;

	std::size_t colonPos = tokens_.at(1).find(':');
	if (colonPos == std::string::npos) {
		errors_.add({curLine_, "No colon found in Subgroup descriptor"});
		return;
	}

	Group* group;
	try {
		group = &Group::withName(tokens_.at(1).substr(0, colonPos));
	} catch (const std::runtime_error& e) {
		errors_.add({curLine_, e.what()});
		return;
	}

	params.name = tokens_.at(1).substr(colonPos + 1);

	colonPos = tokens_.at(2).find(':');
	if (colonPos == std::string::npos) {
		errors_.add({curLine_, "No colon found in Subgroup image descriptor"});
		return;
	}

	try {
		SDL_Surface* temp =
			app_.loadImage(std::string("resources/groups/subgroups/")
							   + tokens_.at(2).substr(0, colonPos) + ".bmp");
		params.image = SDL_CreateRGBSurface(SDL_SWSURFACE, BUTTON_UNIT_SIZE, BUTTON_UNIT_SIZE, 32, 0, 0, 0, 0);
		SDL_FillRect(params.image, nullptr, SDL_MapRGB(params.image->format, 1, 1, 1));
		SDL_SetColorKey(params.image, SDL_SRCCOLORKEY, SDL_MapRGB(params.image->format, 1, 1, 1));
		SDL_Rect srcrect{0,
			Sint16(readInt(tokens_.at(2).substr(colonPos + 1), "Subgroup image position") * BUTTON_UNIT_SIZE),
			BUTTON_UNIT_SIZE, BUTTON_UNIT_SIZE};
		SDL_BlitSurface(temp, &srcrect, params.image, nullptr);
		SDL_FreeSurface(temp);
	} catch(const tetra::Sdl::Exception& e) {
		params.image = nullptr;
		errors_.add({curLine_, e.what()});
	}

	try {
		group->addSubgroup(params);
	} catch (const std::runtime_error& e) {
		errors_.add({curLine_, e.what()});
	}
}

void SandParser::handleElemLine()
{
	if (parsingElem_)
		createElem();

	elemLine_ = curLine_;
	elemIsSecret_ = false;
	elemSubid_ = 0;

	if (tokens_.size() < 2 || tokens_.size() > 4) {
		errors_.add({curLine_, "Invalid number of tokens in Element line"});
		return;
	} else if (tokens_.size() == 2) {
		elemparams_.groupid = 0;
	} else {
		std::size_t colonPos = tokens_.at(2).find(':');

		if (colonPos == std::string::npos) {
			try {
				elemparams_.groupid = Group::withName(tokens_.at(2)).id;
			} catch (const std::runtime_error& e) {
				elemparams_.groupid = 0;
				errors_.add({curLine_, e.what()});
			}
		} else {
			try {
				std::string groupName = tokens_.at(2).substr(0, colonPos),
					subName = tokens_.at(2).substr(colonPos + 1);
				elemparams_.groupid = Group::withName(groupName).id;
				elemSubid_ = Group::withName(groupName).subWithName(subName).id;
			} catch (const std::runtime_error& e) {
				elemparams_.groupid = 0;
				errors_.add({curLine_, e.what()});
			}
		}

		if (tokens_.size() == 4) {
			if (tokens_.at(3) == "(discover)")
				elemIsSecret_ = true;
			else
				errors_.add({curLine_, "Invalid token '", tokens_.at(3), "' after Group name"});
		}
	}


	if (tokens_.at(1).back() != ':')
		errors_.add({curLine_, "Expected ':' at end of Element name '", tokens_.at(1), "'"});

	if (tokens_.at(1).front() == '*')
		errors_.add({curLine_,
			"Element name '",
			tokens_.at(1).substr(0, tokens_.at(1).size() - 1),
			"' cannot begin with '*'"});

	parsingElem_ = true;

	elemparams_.name = tokens_.at(1).substr(0, tokens_.at(1).size() - 1);
	if (elemparams_.name.size() > MAX_ELEM_NAME)
		errors_.add({curLine_,
			"Element name '", elemparams_.name,
			"' is too long (limit is ",
			int(MAX_ELEM_NAME), " characters)"});

	elemparams_.color = {255, 255, 255};
	elemparams_.interpattern = InterPattern::Uniform;
	elemparams_.intercount = 1;
	elemparams_.intertowards.clear();
	elemparams_.towardsrange = 0.0;
	elemparams_.mass = -1;
	elemparams_.entropy = 0.0;
	elemparams_.initforce = 0.0;
	elemparams_.decays = false;
	elemparams_.decaysto = 0;
	elemparams_.lifespan = {-1.0, -1.0};
}

void SandParser::handleElemParam()
{
	if (tokens_.size() < 3) {
		errors_.add({curLine_, "Not enough tokens in parameter line"});
		return;
	}

	if (tokens_.at(1) != "=")
		errors_.add({curLine_, "Expected equals sign in parameter line"});

	tokens_.at(0).erase(tokens_.at(0).begin());

	// elemgroups
	if (tokens_.at(0) == "elemgroups") {
		for (std::size_t i = 2; i < tokens_.size() - 1; ++i) {
			if (tokens_.at(i).back() != ',')
				errors_.add({curLine_, "ElementGroups must be comma separated"});
			tokens_.at(i).erase(--tokens_.at(2).end());
		}
		for (std::size_t i = 2; i < tokens_.size(); ++i) {
			try {
				ElementGroup::withName(tokens_.at(i)).addId(Element::getEndId());
			} catch (const std::runtime_error& e) {
				errors_.add({curLine_, e.what()});
			}
		}
	// color
	} else if (tokens_.at(0) == "color") {
		if (tokens_.size() != 5) {
			errors_.add({curLine_, "Color requires R, G, and B values"});
			return;
		}

		if ((tokens_.at(2).back() != ',') || (tokens_.at(3).back() != ','))
			errors_.add({curLine_, "RGB color must be comma separated"});

		tokens_.at(2).erase(tokens_.at(2).end() - 1);
		tokens_.at(3).erase(tokens_.at(3).end() - 1);
		int r, g, b;

		try {
			r = readInt(tokens_.at(2), "red");
			g = readInt(tokens_.at(3), "green");
			b = readInt(tokens_.at(4), "blue");
		} catch (const Error& e) {
			errors_.add(e);
			return;
		}

		if (r < 0 || r > 255)
			errors_.add({curLine_, "Red channel must be in the range [0, 255]"});
		if (g < 0 || g > 255)
			errors_.add({curLine_, "Blue channel must be in the range [0, 255]"});
		if (b < 0 || b > 255)
			errors_.add({curLine_, "Green channel must be in the range [0, 255]"});

		tetra::Sdl::Channel R = r, G = g, B = b;

		elemparams_.color = {R, G, B};
	// interpattern
	} else if (tokens_.at(0) == "interpattern") {
		if (tokens_.at(2) == "uniform")
			elemparams_.interpattern = InterPattern::Uniform;
		else if (tokens_.at(2) == "up")
			elemparams_.interpattern = InterPattern::Up;
		else if (tokens_.at(2) == "down")
			elemparams_.interpattern = InterPattern::Down;
		else
			errors_.add({curLine_,
				"Invalid Interaction pattern '", tokens_.at(2), "'"});
	// intercount
	} else if (tokens_.at(0) == "intercount") {
		try {
			elemparams_.intercount = readInt(tokens_.at(2), tokens_.at(0));
		} catch (const Error& e) {
			errors_.add(e);
			return;
		}

		if (elemparams_.intercount <= 0 || elemparams_.intercount > 25)
			errors_.add({curLine_, "Interaction count must be in range (0, 25]"});
	// intertowards
	} else if (tokens_.at(0) == "intertowards") {
		TowardsList towards(elemparams_.name);
		for (std::size_t i = 2; i < tokens_.size() - 1; ++i)
			towards.names.push_back(tokens_.at(i).substr(0, tokens_.at(i).size() - 1));
		towards.names.push_back(tokens_.back());
		towardsListQueue_.push_back(towards);
	// towardsrange
	} else if (tokens_.at(0) == "towardsrange") {
		elemparams_.towardsrange = readDouble(tokens_.at(2), tokens_.at(0));
	// mass
	} else if (tokens_.at(0) == "mass") {
		elemparams_.mass = readInt(tokens_.at(2), tokens_.at(0));

		if (elemparams_.mass < 0 || elemparams_.mass > 100)
			errors_.add({curLine_, "Mass must be in range [0, 100]"});
	// entropy
	}  else if (tokens_.at(0) == "entropy") {
		try {
			elemparams_.entropy = readDouble(tokens_.at(2), tokens_.at(0));
		} catch (const Error& e) {
			errors_.add(e);
			return;
		}
	// initforce
	} else if (tokens_.at(0) == "initforce") {
		try {
			elemparams_.initforce = readDouble(tokens_.at(2), tokens_.at(0));
		} catch (const Error& e) {
			errors_.add(e);
			return;
		}
	// decays
	} else if (tokens_.at(0) == "decays") {
		if (tokens_.at(2) == "true")
			elemparams_.decays = true;
		else if (tokens_.at(2) == "false")
			elemparams_.decays = false;
		else
			errors_.add({curLine_,
				"Decays must be either true or false"});
	// decaysto
	} else if (tokens_.at(0) == "decaysto") {
		try {
			elemparams_.decaysto = Element::withName(tokens_.at(2)).id;
		} catch(const std::runtime_error& e) {
			errors_.add({curLine_, e.what()});
		}
	// lifespan
	} else if (tokens_.at(0) == "lifespan") {
		if (!elemparams_.decays) {
			errors_.add({curLine_, "Only element which decays can have a lifespan"});
			return;
		}

		try {
			if (tokens_.size() == 3)
				elemparams_.lifespan = {readDouble(tokens_.at(2), tokens_.at(0) + " single bound"),
										readDouble(tokens_.at(2), tokens_.at(0) + " single bound")};
			else if (tokens_.size() != 4) {
				errors_.add({curLine_,
					"Lifespan requires either single value or lower and upper bound"});
				return;
			} else
				elemparams_.lifespan = {readDouble(tokens_.at(2), tokens_.at(0) + " lower bound"),
										readDouble(tokens_.at(3), tokens_.at(0) + " upper bound")};
		} catch (const Error& e) {
			errors_.add(e);
			return;
		}

		if (elemparams_.lifespan.first < 0.0)
			errors_.add({curLine_,
				"Lower life bound must be non-negative"});
		if (elemparams_.lifespan.second < 0.0)
			errors_.add({curLine_,
				"Upper life bound must be non-negative"});
		if (elemparams_.lifespan.second < elemparams_.lifespan.first)
			errors_.add({curLine_,
				"Upper life bound cannot be less than lower bound"});
	} else
		errors_.add({curLine_,
			"Invalid parameter '", tokens_.at(0), "' for Element '", elemparams_.name, "'"});
}

void SandParser::createElem()
{
	parsingElem_ = false;

	if (elemparams_.mass < 0.0)
		errors_.add({curLine_ - 1,
			"Must specify mass for element '", elemparams_.name, "'"});

	if (elemparams_.decays && elemparams_.lifespan.first < 0.0)
		errors_.add({curLine_ - 1,
			"Must specify lifespan for decaying element '", elemparams_.name, "'"});

	if (Element::withColorExists(elemparams_.color)) {
		errors_.add({elemLine_, true,
			"Element with color given for '", elemparams_.name, "' already exists ('",
			Element::withColor(elemparams_.color).name, "')"});
		return;
	}

	ElementGroup::withName("any").addId(Element::getEndId());

	try {
		Element::create(elemparams_);
		Group::withId(elemparams_.groupid).addMember(
			elemSubid_, Element::getEndId() - 1, elemIsSecret_);
	} catch (const std::runtime_error& e) {
		errors_.add({elemLine_, e.what()});
	}
}

void SandParser::handleInterLine()
{
	for (auto it = tokens_.begin(); it != tokens_.end(); ++it) {
		if (it->find('(') != std::string::npos) {
			if (it->find(')') != std::string::npos)
				continue;
			auto it2 = it + 1;
			if (it2 == tokens_.end()) {
				errors_.add({curLine_, "Expected token after opening parenthesis in Interaction line"});
				return;
			}

			if (it2->find(')') == std::string::npos) {
				errors_.add({curLine_, "Expected closing parenthesis in Interaction line"});
				return;
			}

			*it = *it + ' ' + *it2;
			it = tokens_.erase(it2);
		}
	}

	if (tokens_.size() < 4) {
		errors_.add({curLine_,
			"Invalid number of tokens in Interaction line"});
		return;
	}

	// Read the interacting elements
	std::size_t pos = tokens_.at(1).find("::");

	if (pos == std::string::npos) {
		errors_.add({curLine_,
			"Did not find first '::' delimiter in Interaction line"});
		return;
	}

	std::string name1 = tokens_.at(1).substr(0, pos);
	std::string name2 = tokens_.at(1).substr(pos + 2);

	int wildcard = 0;
	const ElementGroup* elemGroup = nullptr;
	std::vector<int>::const_iterator elemGroupMem;

	try {
		if (name1.at(0) == '*') {
			wildcard += 1;
			elemGroup = &ElementGroup::withName(name1.substr(1));
			elemGroupMem = elemGroup->memberIds.begin();
		} else
			Element::withName(name1);
		if (name2.at(0) == '*') {
			wildcard += 2;
			elemGroup = &ElementGroup::withName(name2.substr(1));
			elemGroupMem = elemGroup->memberIds.begin();
		} else
			Element::withName(name2);
	} catch (const std::runtime_error& e) {
//		errors_.add({curLine_, "Invalid ElementGroup wildcard in Interaction line"});
		errors_.add({curLine_, e.what()});
		return;
	}

	if (wildcard == 3) {
		errors_.add({curLine_, "Wildcard cannot interact with wildcard"});
		return;
	}

	static std::vector<std::string> copyNames;
	bool copy = false;

	if (wildcard != 1) {
		copyNames.clear();
		for (auto& interCopy : interCopyQueue_) {
			if (interCopy.from == name1) {
				copyNames.emplace_back(interCopy.to);
				copy = true;
			}
		}
	}

	bool cont = wildcard;
	do {
		if (wildcard) {
			if (wildcard == 1)
				name1 = Element::withId(*elemGroupMem).name;
			else if (wildcard == 2)
				name2 = Element::withId(*elemGroupMem).name;

			GroupInterExclusion test{tokens_.at(1), *elemGroupMem};
			if (tetra::contains(exclusionQueue_.begin(), exclusionQueue_.end(), test)
				|| (elemGroup->name == "any" && name1 == name2))
			{
				if (++elemGroupMem == elemGroup->memberIds.end())
					break;
				else
					continue;
			}
			wildcardId_ = *elemGroupMem;
			if (++elemGroupMem == elemGroup->memberIds.end())
				cont = false;
		} else
			wildcardId_ = -1;

		addInteraction(name1, name2);
		if (copy)
			for (const std::string& copyName : copyNames)
				addInteraction(copyName, name2, name1);
	} while (cont);
}

void SandParser::addInteraction(const std::string& name1, const std::string& name2)
{
	Interaction::param_type params;
	try {
		params.interactIds.first = Element::withName(name1).id;
	} catch(const std::runtime_error& e) {
		errors_.add({curLine_, e.what()});
		return;
	}

	try {
		params.interactIds.second = Element::withName(name2).id;
		if (params.interactIds.second == 0)
			Element::withName(name1).interactsWithNone = true;
	} catch(const std::runtime_error& e) {
		errors_.add({curLine_, e.what()});
		return;
	}

	params.frequency = readDouble(tokens_.back(), "frequency");

	if (params.frequency <= 0.0 || params.frequency > 1.0)
		errors_.add({curLine_, "Frequency must be in range (0.0, 1.0]"});

	Interaction* parent = nullptr;
	Interaction* inter = nullptr;

	for (std::size_t i = 2; i < tokens_.size(); ++i) {
		// Determine what type of interaction and read it
		if (tokens_.at(i).find("::") != std::string::npos)
			inter = handleElemChangeInter(params, parent != nullptr, i);

		else if (tokens_.at(i).find("repel") == 0)
			inter = handleRepelInter(params, parent != nullptr, i);

		else if (tokens_.at(i).find("attract") == 0)
			inter = handleAttractInter(params, parent != nullptr, i);

		else if (tokens_.at(i).find("angle") == 0)
			inter = handleAngleInter(params, parent != nullptr, i);

		else if (tokens_.at(i).find("cancel_force") == 0)
			inter = handleCancelForceInter(params, parent != nullptr, i);

		else if (tokens_.at(i).find("swap") == 0)
			inter = handleSwapInter(params, parent != nullptr, i);

		else {
			errors_.add({curLine_,
					"Unrecognized interaction type '", tokens_.at(i), "'"});
			return;
		}

		if (!parent) {
			parent = inter;
		} else {
			parent->next.reset(inter);
			parent = inter;
		}

		++i;
		if (i >= tokens_.size()) {
			errors_.add({curLine_, "Error parsing Interaction line"});
			return;
		}

		if (i != tokens_.size() - 1 && tokens_.at(i) != "And") {
			errors_.add({curLine_, "Multiple interactions must be separated by 'And'"});
			return;
		}
	}
}

void SandParser::addInteraction(const std::string& name1, const std::string& name2, const std::string& orig1)
{
	Interaction::param_type params;
	try {
		params.interactIds.first = Element::withName(name1).id;
	} catch(const std::runtime_error& e) {
		errors_.add({curLine_, e.what()});
		return;
	}

	try {
		params.interactIds.second = Element::withName(name2).id;
		if (params.interactIds.second == 0)
			Element::withName(name1).interactsWithNone = true;
	} catch(const std::runtime_error& e) {
		errors_.add({curLine_, e.what()});
		return;
	}

	params.frequency = readDouble(tokens_.back(), "frequency");

	if (params.frequency <= 0.0 || params.frequency > 1.0)
		errors_.add({curLine_, "Frequency must be in range (0.0, 1.0]"});

	Interaction* parent = nullptr;
	Interaction* inter = nullptr;

	for (std::size_t i = 2; i < tokens_.size(); ++i) {
		// Determine what type of interaction and read it
		if (tokens_.at(i).find("::") != std::string::npos) {
			std::size_t pos = tokens_.at(i).find("::");

			std::string orig3 = tokens_.at(i).substr(0, pos);
			std::string orig4 = tokens_.at(i).substr(pos + 2);

			std::string name3 = orig3, name4 = orig4;

			if (name3 == orig1)
				name3 = name1;

			tokens_.at(i) = name3 + "::" + name4;

			inter = handleElemChangeInter(params, parent != nullptr, i);

			tokens_.at(i) = orig3 + "::" + orig4;
		}

		else if (tokens_.at(i).find("repel") == 0)
			inter = handleRepelInter(params, parent != nullptr, i);

		else if (tokens_.at(i).find("attract") == 0)
			inter = handleAttractInter(params, parent != nullptr, i);

		else if (tokens_.at(i).find("angle") == 0)
			inter = handleAngleInter(params, parent != nullptr, i);

		else if (tokens_.at(i).find("cancel_force") == 0)
			inter = handleCancelForceInter(params, parent != nullptr, i);

		else if (tokens_.at(i).find("swap") == 0)
			inter = handleSwapInter(params, parent != nullptr, i);

		else {
			errors_.add({curLine_,
					"Unrecognized interaction type '", tokens_.at(i), "'"});
			return;
		}

		if (!inter)
			return;

		if (!parent) {
			parent = inter;
		} else {
			parent->next.reset(inter);
			parent = inter;
		}

		++i;
		if (i >= tokens_.size()) {
			errors_.add({curLine_, "Error parsing Interaction line"});
			return;
		}

		if (i != tokens_.size() - 1 && tokens_.at(i) != "And") {
			errors_.add({curLine_, "Multiple interactions must be separated by 'And'"});
			return;
		}
	}
}

Interaction* SandParser::handleElemChangeInter(const Interaction::param_type& baseParams,
											   bool addition, int i)
{
	ElementChangeInteraction::param_type params(baseParams);

	std::size_t pos = tokens_.at(i).find("::");

	std::string name1 = tokens_.at(i).substr(0, pos);
	std::string name2 = tokens_.at(i).substr(pos + 2);

	try {
		if (name1 != "*") {
			params.newIds.first = Element::withName(name1).id;
		} else {
			if (wildcardId_ == -1) {
				errors_.add({curLine_,
					"Only interactions with ElementGroup can use wildcard"});
				return nullptr;
			}
			params.newIds.first = wildcardId_;
		}
	} catch(const std::runtime_error& e) {
		errors_.add({curLine_, e.what()});
		return nullptr;
	}

	try {
		if (name2 != "*") {
			params.newIds.second = Element::withName(name2).id;
		} else {
			if (wildcardId_ == -1) {
				errors_.add({curLine_,
					"Only interactions with ElementGroup can use wildcard"});
				return nullptr;
			}
			params.newIds.second = wildcardId_;
		}
	} catch(const std::runtime_error& e) {
		errors_.add({curLine_, e.what()});
		return nullptr;
	}

	if (addition)
		return new ElementChangeInteraction(params, InteractionHandler::musicGen());
	else
		return InteractionHandler::create<ElementChangeInteraction>(params);
}

Interaction* SandParser::handleRepelInter(const Interaction::param_type& baseParams,
										  bool addition, int i)
{
	RepelInteraction::param_type params(baseParams);

	std::size_t pos1 = tokens_.at(i).find("(");
	std::size_t pos2 = tokens_.at(i).find(")");

	if (pos1 == std::string::npos) {
		errors_.add({curLine_, "Did not find '(' in Interaction line"});
		return nullptr;
	}

	if (pos2 == std::string::npos) {
		errors_.add({curLine_, "Did not find ')' in Interaction line"});
		return nullptr;
	}

	if (pos2 < pos1) {
		errors_.add({curLine_, "'(' must be before ')' in Interaction line"});
		return nullptr;
	}

	std::string force_s = tokens_.at(i).substr(pos1 + 1, pos2 - pos1 - 1);
	if (force_s.find(',') != std::string::npos || force_s.find(' ') != std::string::npos) {
		errors_.add({curLine_, "'repel' only takes one argument"});
		return nullptr;
	}

	params.force = readDouble(force_s, "repel force");

	if (params.force <= 0.0)
		errors_.add({curLine_, "Repel force must be positive"});

	if (addition)
		return new RepelInteraction(params, InteractionHandler::musicGen());
	else
		return InteractionHandler::create<RepelInteraction>(params);
}

Interaction* SandParser::handleAttractInter(const Interaction::param_type& baseParams,
											bool addition, int i)
{
	AttractInteraction::param_type params(baseParams);

	std::size_t pos1 = tokens_.at(i).find("(");
	std::size_t pos2 = tokens_.at(i).find(")");

	if (pos1 == std::string::npos) {
		errors_.add({curLine_, "Did not find '(' in Interaction line"});
		return nullptr;
	}

	if (pos2 == std::string::npos) {
		errors_.add({curLine_, "Did not find ')' in Interaction line"});
		return nullptr;
	}

	if (pos2 < pos1) {
		errors_.add({curLine_, "'(' must be before ')' in Interaction line"});
		return nullptr;
	}

	std::string force_s = tokens_.at(i).substr(pos1 + 1, pos2 - pos1 - 1);
	if (force_s.find(',') != std::string::npos || force_s.find(' ') != std::string::npos) {
		errors_.add({curLine_, "'attract' only takes one argument"});
		return nullptr;
	}

	params.force = readDouble(force_s, "attract force");

	if (params.force <= 0.0)
		errors_.add({curLine_, "Attract force must be positive"});

	if (addition)
		return new AttractInteraction(params, InteractionHandler::musicGen());
	else
		return InteractionHandler::create<AttractInteraction>(params);
}

Interaction* SandParser::handleAngleInter(const Interaction::param_type& baseParams,
										  bool addition, int i)
{
	AngleInteraction::param_type params(baseParams);

	std::size_t pos1 = tokens_.at(i).find("(");
	std::size_t pos2 = tokens_.at(i).find(")");

	if (pos1 == std::string::npos) {
		errors_.add({curLine_, "Did not find '(' in Interaction line"});
		return nullptr;
	}

	if (pos2 == std::string::npos) {
		errors_.add({curLine_, "Did not find ')' in Interaction line"});
		return nullptr;
	}

	if (pos2 < pos1) {
		errors_.add({curLine_, "'(' must be before ')' in Interaction line"});
		return nullptr;
	}

	std::string args = tokens_.at(i).substr(pos1 + 1, pos2 - pos1 - 1);
	if (args.find(',') == std::string::npos || args.find(',') != args.rfind(',')) {
		errors_.add({curLine_, "'angle' takes two arguments"});
		return nullptr;
	}

	std::size_t posc = args.find(',');
	std::string angle_s = args.substr(0, posc);
	std::string force_s = args.substr(posc + 2);

	params.angle = readDouble(angle_s, "angle") * M_PI / 180.0;
	params.force = readDouble(force_s, "angle force");

	if (params.force <= 0.0)
		errors_.add({curLine_, "Angle force must be positive"});

	if (addition)
		return new AngleInteraction(params, InteractionHandler::musicGen());
	else
		return InteractionHandler::create<AngleInteraction>(params);
}

Interaction* SandParser::handleCancelForceInter(const Interaction::param_type& baseParams,
												bool addition, int i)
{
	CancelForceInteraction::param_type params(baseParams);

	std::size_t pos1 = tokens_.at(i).find("(");
	std::size_t pos2 = tokens_.at(i).find(")");

	if (pos1 == std::string::npos) {
		errors_.add({curLine_, "Did not find '(' in Interaction line"});
		return nullptr;
	}

	if (pos2 == std::string::npos) {
		errors_.add({curLine_, "Did not find ')' in Interaction line"});
		return nullptr;
	}

	if (pos2 < pos1) {
		errors_.add({curLine_, "'(' must be before ')' in Interaction line"});
		return nullptr;
	}

	if (pos1 + 1 != pos2) {
		errors_.add({curLine_, "'cancel_force' does not take arguments"});
		return nullptr;
	}

	if (addition)
		return new CancelForceInteraction(params, InteractionHandler::musicGen());
	else
		return InteractionHandler::create<CancelForceInteraction>(params);
}

Interaction* SandParser::handleSwapInter(const Interaction::param_type& baseParams,
										 bool addition, int i)
{
	SwapInteraction::param_type params(baseParams);

	std::size_t pos1 = tokens_.at(i).find("(");
	std::size_t pos2 = tokens_.at(i).find(")");

	if (pos1 == std::string::npos) {
		errors_.add({curLine_, "Did not find '(' in Interaction line"});
		return nullptr;
	}

	if (pos2 == std::string::npos) {
		errors_.add({curLine_, "Did not find ')' in Interaction line"});
		return nullptr;
	}

	if (pos2 < pos1) {
		errors_.add({curLine_, "'(' must be before ')' in Interaction line"});
		return nullptr;
	}

	if (pos1 + 1 == pos2) {
		params.newId = -1;
	 } else {
		std::string newName = tokens_.at(i).substr(pos1 + 1, pos2 - pos1 - 1);

		try {
			params.newId = Element::withName(newName).id;
		} catch(const std::runtime_error& e) {
			errors_.add({curLine_, e.what()});
			return nullptr;
		}
	}
	if (addition)
		return new SwapInteraction(params, InteractionHandler::musicGen());
	else
		return InteractionHandler::create<SwapInteraction>(params);
}

void SandParser::handleElementGroupLine()
{
	if (tokens_.size() != 2) {
		errors_.add({curLine_, "Invalid number of tokens in ElementGroup line"});
		return;
	}

	ElementGroup::param_type params;

	params.name = tokens_.at(1);

	try {
		ElementGroup::create(params);
	} catch (const std::runtime_error& e) {
		errors_.add({curLine_, e.what()});
	}
}

void SandParser::handleAddToElemGroupLine()
{
	if (tokens_.size() != 4) {
		errors_.add({curLine_, "Invalid number of tokens in Add line"});
		return;
	}

	int elemId;
	try {
		elemId = Element::withName(tokens_.at(1)).id;
	} catch(const std::runtime_error& e) {
		errors_.add({curLine_, e.what()});
		return;
	}

	if (tokens_.at(2) != "To")
		errors_.add({curLine_,
			"Syntax for Add line is 'Add elem-name To elemgroup-name'"});

	try {
		ElementGroup::withName(tokens_.at(3)).addId(elemId);
	} catch(const std::runtime_error& e) {
		errors_.add({curLine_, e.what()});
	}
}

void SandParser::handleExcludeLine()
{
	if (tokens_.size() != 4) {
		errors_.add({curLine_, "Invalid number of tokens in Exclude line"});
		return;
	}

	int id;
	try {
		id = Element::withName(tokens_.at(1)).id;
	} catch (const std::runtime_error& e) {
		errors_.add({curLine_, e.what()});
		return;
	}

	if (tokens_.at(2) != "From")
		errors_.add({curLine_,
			"Syntax for Exclude line is 'Exclude elem-name From interaction-key'"});

	std::size_t pos = tokens_.at(3).find("::");

	if (pos == std::string::npos) {
		errors_.add({curLine_,
			"Did not find '::' delimiter in Interaction key for Exclude line"});
		return;
	}

	std::string name1 = tokens_.at(3).substr(0, pos);
	std::string name2 = tokens_.at(3).substr(pos + 2);

	if (name1.at(0) == '*' && name2.at(0) == '*') {
		errors_.add({curLine_, "No Interaction between two ElementGroups is possible"});
		return;
	}

	exclusionQueue_.push_back(GroupInterExclusion{tokens_.at(3), id});
}

void SandParser::handleCopyLine()
{
	if (tokens_.size() != 4) {
		errors_.add({curLine_, "Invalid number of tokens in Copy line"});
		return;
	}

	if (tokens_.at(2) != "To")
		errors_.add({curLine_,
			"Syntax for Copy line is 'Copy elem-from To elem-to'"});

	interCopyQueue_.emplace_back(curLine_, tokens_.at(1), tokens_.at(3));
}

void SandParser::checkCopyQueue()
{
	for (auto it = interCopyQueue_.begin(); it != interCopyQueue_.end(); ++it) {
		try {
			it->fromId = Element::withName(it->from).id;
		} catch (const std::runtime_error& e) {
			errors_.add({it->line, "Invalid Element", it->from, "in copy line"});
			it = --interCopyQueue_.erase(it);
		}

		try {
			it->toId = Element::withName(it->to).id;
		} catch (const std::runtime_error& e) {
			errors_.add({it->line, "Invalid Element '", it->to, "' in copy line"});
			it = --interCopyQueue_.erase(it);
		}
	}
}

void SandParser::expandInterTowards(const TowardsList& l)
{
	for (const std::string& name : l.names) {
		if (name.at(0) == '*') {
			ElementGroup* group;
			try {
				group = &ElementGroup::withName(name.substr(1));
			} catch (const std::runtime_error& e) {
				errors_.add({curLine_, e.what()});
				return;
			}
			for (int id : group->memberIds)
				Element::withName(l.elemName).intertowards.push_back(id);
		} else {
			int id;
			try {
				id = Element::withName(name).id;
			} catch (const std::runtime_error& e) {
				errors_.add({curLine_, e.what()});
				return;
			}
			Element::withName(l.elemName).intertowards.push_back(id);
		}
	}
}

void SandParser::firstPassHandle()
{
	// If this is an empty line or a comment line
	if (tokens_.empty() || tokens_.at(0).at(0) == '#')
		return;

	if (tokens_.at(0) == "Element")
		handleElemLine();

	else if (tokens_.at(0).at(0) == '\t')
		if (parsingElem_)
			handleElemParam();
		else
			errors_.add({curLine_, "Unexpected indented line"});
	else {
		if (parsingElem_)
			createElem();

		if (tokens_.at(0) == "Group")
			handleGroupLine();

		else if (tokens_.at(0) == "Subgroup")
			handleSubgroupLine();

		else if (tokens_.at(0) == "ElementGroup")
			handleElementGroupLine();

		else if (tokens_.at(0) == "Add")
			handleAddToElemGroupLine();

		else if (tokens_.at(0) == "Exclude")
			handleExcludeLine();

		else if (tokens_.at(0) == "Copy")
			handleCopyLine();

		else if (tokens_.at(0) == "Include")
			handleIncludeLine();

		else if (tokens_.at(0) != "Interaction")
			errors_.add({curLine_,
				"Unrecognized token '", tokens_.at(0), "'"});
	}
}

void SandParser::secondPassHandle()
{
	// If this is an empty line or a comment line
	if (tokens_.empty() || tokens_.at(0).at(0) == '#')
		return;

	if (tokens_.at(0) == "Interaction")
		handleInterLine();
}

SandParser::SandParser(ParentParser& parent, const std::string& filename) :
	app_(parent.getApp()), parent_(parent), sandsrcDir_("sandsrc"), file_(sandsrcDir_ + '/' + filename), errors_(filename)
{
	if (!file_.is_open())
		throw Error(0, "Failed to open file '", filename, "'");
}

const SandParser::ErrorList& SandParser::parse()
{
	curLine_ = elemLine_ = 0;

	while (!file_.eof()) {
		getline(file_, line_);
		++curLine_;

		parseTokens();

		firstPassHandle();

		tokens_.clear();
	}

	if (parsingElem_)
		createElem();

	for (const TowardsList& l : towardsListQueue_)
		expandInterTowards(l);

	checkCopyQueue();

	file_.clear();
	file_.seekg(0, std::ios::beg);
	curLine_ = 0;

	while (!file_.eof()) {
		getline(file_, line_);
		++curLine_;

		parseTokens();

		secondPassHandle();

		tokens_.clear();
	}

	return errors_;
}

ParentParser::ParentParser(PixelCosmApp& app, const std::string& filename) :
	app_(app), queue_({filename}) {}

PixelCosmApp& ParentParser::getApp()
{
	return app_;
}

void ParentParser::addToQueue(const std::string& filename)
{
	queue_.push_back(filename);
}

ParentParser::ErrorLists& ParentParser::parse()
{
	do {
		try {
			SandParser parser(*this, queue_.front());
			errors_.emplace_back(parser.parse());
		} catch (const SandParser::Error& e) {
			errors_.emplace_back(queue_.front());
			errors_.back().add(e);
		}
		queue_.pop_front();
	} while (!queue_.empty());

	return errors_;
}

}
