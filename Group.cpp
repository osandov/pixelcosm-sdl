#include "Group.hpp"
#include "PixelCosm.hpp"

namespace PixelCosm
{

Group::Group(const param_type& params) :
	id(groups_.size()), name(params.name),
	image(params.image)
{
	SDL_Surface* subImage;
	try {
		SDL_Surface* temp = PixelCosmApp::loadImage(std::string("resources/groups/subgroups/standard.bmp"));
		subImage = SDL_CreateRGBSurface(SDL_SWSURFACE, BUTTON_UNIT_SIZE, BUTTON_UNIT_SIZE, 32, 0, 0, 0, 0);
		SDL_FillRect(subImage, nullptr, SDL_MapRGB(subImage->format, 1, 1, 1));
		SDL_SetColorKey(subImage, SDL_SRCCOLORKEY, SDL_MapRGB(subImage->format, 1, 1, 1));
		SDL_Rect srcrect{0, 0, BUTTON_UNIT_SIZE, BUTTON_UNIT_SIZE};
		SDL_BlitSurface(temp, &srcrect, subImage, nullptr);
		SDL_FreeSurface(temp);
	} catch(const tetra::Sdl::Exception& e) {
		throw std::runtime_error(e.what());
	}

	subgroups.emplace_back(Subgroup::param_type{"Et al.", subImage}, 0);
}

Group& Group::withId(int id)
{
	return groups_[id];
}

Group& Group::withName(const std::string& name)
{
	auto found = std::find_if(groups_.begin(), groups_.end(),
		[&name](const Group& g) {return g.name == name;});
	if (found == groups_.end())
		throw std::runtime_error(
			std::string("No Group with name '") + name + "'");
	return *found;
}

/** Returns the identification number following the last generated
 * identification number
 */
int Group::getEndId()
{
	return groups_.size();
}

void Group::create(const param_type& params)
{
	auto found = std::find_if(groups_.begin(), groups_.end(),
		[&params](const Group& g) {return g.name == params.name;});
	if (found != groups_.end())
		throw std::runtime_error(
			std::string("Group with name ' ") + params.name + " ' already exists");
	groups_.emplace_back(params);
}

void Group::addSubgroup(const Subgroup::param_type& params)
{
	subgroups.emplace_back(params, subgroups.size());
}

Subgroup& Group::subWithName(const std::string& name)
{
	auto found = std::find_if(subgroups.begin(), subgroups.end(),
		[&name](const Subgroup& s) {return s.name == name;});
	if (found == subgroups.end())
		throw std::runtime_error(
			std::string("No Subgroup with name '") + name + "'");
	return *found;
}

void Group::addMember(int subid, int elemid, bool secret)
{
	members.emplace_back(subid, elemid, secret);
	std::sort(members.begin(), members.end());
}

std::vector<Group::Member>::difference_type Group::countAvailable() const
{
	return std::count_if(members.begin(), members.end(),
		[](const Member& mem) {return mem.count > 0;});
}

Group::Member& Group::memberWithId(int elemid)
{
	auto it = std::find_if(members.begin(), members.end(),
		[elemid](const Member& mem) {return elemid == mem.elemid;});
	if (it != members.end())
		return *it;
	else
		return *members.begin();
}

void Group::newAvailable(bool set)
{
	newAvailable_ = set;
}

bool Group::newAvailable() const
{
	return newAvailable_;
}

}
