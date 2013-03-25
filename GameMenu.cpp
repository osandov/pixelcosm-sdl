#include "PixelCosm.hpp"
#include "GameMenu.hpp"

namespace PixelCosm
{

// Menu
GameMenu::GameMenu(PixelCosmApp& app) :
		app_(app), field_(app.field_),
		drawer_(app.drawer_), writer_(app.writer_),
		gridPixel(drawer_.mapRgb(64, 64, 64)),
		buttonPixel(drawer_.mapRgb(128, 128, 128)),
		highlightPixel(drawer_.mapRgb(160, 160, 160)),
		whitePixel(drawer_.mapRgb(255, 255, 255)),
		errorPixel(drawer_.mapRgb(128, 0, 0)) {}

const tetra::point<int>& GameMenu::mousePos()
{
	return app_.mouse_.pos;
}

void GameMenu::setupMenu()
{
	quickBar_.emplace_back(new TitleScreenButton(*this, {DISPLAY_WIDTH, DISPLAY_HEIGHT},
		{BUTTON_UNIT_SIZE * GROUP_UNITS, BUTTON_UNIT_SIZE * 2}));

	quickBar_.emplace_back(new PauseButton(*this, {0, DISPLAY_HEIGHT},
		{BUTTON_UNIT_SIZE * QUICK_UNITS, BUTTON_UNIT_SIZE * QUICK_UNITS}));

	quickBar_.emplace_back(new SaveButton(*this, {quickBar_.back()->right(), DISPLAY_HEIGHT},
		{BUTTON_UNIT_SIZE * QUICK_UNITS, BUTTON_UNIT_SIZE * QUICK_UNITS}));

	quickBar_.emplace_back(new OpenButton(*this, {quickBar_.back()->right(), DISPLAY_HEIGHT},
		{BUTTON_UNIT_SIZE * QUICK_UNITS, BUTTON_UNIT_SIZE * QUICK_UNITS}));

	quickBar_.emplace_back(new ResetButton(*this, {quickBar_.back()->right(), DISPLAY_HEIGHT},
		{BUTTON_UNIT_SIZE * QUICK_UNITS, BUTTON_UNIT_SIZE * QUICK_UNITS}));

	quickBar_.emplace_back(new MusicButton(*this, {quickBar_.back()->right(), DISPLAY_HEIGHT},
		{BUTTON_UNIT_SIZE * QUICK_UNITS, BUTTON_UNIT_SIZE * QUICK_UNITS}));

	quickBar_.emplace_back(new Spacer(*this, {quickBar_.back()->right(), DISPLAY_HEIGHT},
		{BUTTON_UNIT_SIZE, BUTTON_UNIT_SIZE * QUICK_UNITS}));

	quickBar_.emplace_back(new ToolButton(*this, {quickBar_.back()->right(), DISPLAY_HEIGHT},
		{BUTTON_UNIT_SIZE * QUICK_UNITS, BUTTON_UNIT_SIZE * QUICK_UNITS}));

	quickBar_.emplace_back(new BorderButton(*this, {quickBar_.back()->right(), DISPLAY_HEIGHT},
		{BUTTON_UNIT_SIZE * QUICK_UNITS, BUTTON_UNIT_SIZE * QUICK_UNITS}));

	quickBar_.emplace_back(new GravityButton(*this, {quickBar_.back()->right(), DISPLAY_HEIGHT},
		{BUTTON_UNIT_SIZE * QUICK_UNITS, BUTTON_UNIT_SIZE * QUICK_UNITS}));

	quickBar_.emplace_back(new Spacer(*this, {quickBar_.back()->right(), DISPLAY_HEIGHT},
		{BUTTON_UNIT_SIZE, BUTTON_UNIT_SIZE * QUICK_UNITS}));

	firstQuickElemIndex_ = quickBar_.size();
	quickBar_.reserve(quickBar_.size() + maxQuickElems_);

	musicDropDown_.buttons.emplace_back(new PauseMusicButton(*this, {0, 0},
		{BUTTON_UNIT_SIZE * QUICK_UNITS, BUTTON_UNIT_SIZE * QUICK_UNITS}));
	musicDropDown_.buttons.emplace_back(new SkipSongButton(*this,
		{musicDropDown_.buttons.back()->right(), musicDropDown_.buttons.back()->top()},
		{BUTTON_UNIT_SIZE * QUICK_UNITS, BUTTON_UNIT_SIZE * QUICK_UNITS}));

	collectionProgress_.offset = {DISPLAY_FACTOR, DISPLAY_FACTOR};
}

void GameMenu::setupGroups()
{
	groupDropDowns_.resize(Group::getEndId());

	int bottom = 0;
	for (int i = 1; i < Group::getEndId(); ++i)
	{
		const Group& group = Group::withId(i);
		groupsBar_.emplace_back(new GroupButton(*this, {DISPLAY_WIDTH, bottom},
				{BUTTON_UNIT_SIZE * GROUP_UNITS, BUTTON_UNIT_SIZE * GROUP_UNITS},
				 group));
		bottom = groupsBar_.back()->bottom();

		setupGroupDropDown(group);
	}
}

void GameMenu::setupGroupDropDown(const Group& cgroup)
{
	Group& group = Group::withId(cgroup.id);
	group.newAvailable(false);
	std::vector<std::shared_ptr<MenuButton>>& buttons = groupDropDowns_[group.id].buttons;
	buttons.clear();

	int maxMembers = 0;
	for (std::size_t i = 0; i < group.subgroups.size(); ++i) {
		Subgroup& subgroup = group.subgroups[i];

		auto it = std::find_if(group.members.begin(), group.members.end(),
			[&subgroup](const Group::Member& member) {return member.subid == subgroup.id;});

		auto last = std::find_if(group.members.begin(), group.members.end(),
			[&subgroup](const Group::Member& member) {return member.subid > subgroup.id;});

		int memberCount = 0;
		for (; it != last; ++it) {
			if (it->count > 0)
				++memberCount;
		}

		if (memberCount > maxMembers)
			maxMembers = memberCount;
	}
	int maxRight = maxMembers * BUTTON_UNIT_SIZE * 2 + BUTTON_UNIT_SIZE;

	std::size_t i = 1;
	do {
		if (i == group.subgroups.size())
			i = 0;

		Subgroup& subgroup = group.subgroups[i];

		auto first = std::find_if(group.members.begin(), group.members.end(),
			[&subgroup](const Group::Member& member) {return member.subid == subgroup.id;});

		if (first == group.members.end())
			continue;

		auto last = std::find_if(group.members.begin(), group.members.end(),
			[&subgroup](const Group::Member& member) {return member.subid > subgroup.id;});

		int memberCount = 0;
		for (auto it = first; it != last; ++it) {
			if (it->count > 0)
				++memberCount;
		}

		int right = memberCount * BUTTON_UNIT_SIZE * 2 + BUTTON_UNIT_SIZE;

		if (buttons.empty())
			buttons.emplace_back(new SubgroupButton(*this, {maxRight - right, 0},
				{BUTTON_UNIT_SIZE, BUTTON_UNIT_SIZE * 2}, subgroup));
		else
			buttons.emplace_back(new SubgroupButton(*this, {maxRight - right, buttons.back()->bottom()},
				{BUTTON_UNIT_SIZE, BUTTON_UNIT_SIZE * 2}, subgroup));

		for (auto it = first; it != last; ++it) {
			if (it->count == 0)
				continue;
			buttons.emplace_back(new ElementButton(*this, {buttons.back()->right(), buttons.back()->top()},
				{BUTTON_UNIT_SIZE * 2, BUTTON_UNIT_SIZE * 2}, Element::withId(it->elemid)));
		}

		if (i == 0)
			break;

		++i;
	} while (true);
}

void GameMenu::render()
{
	if (dropDown_)
		dropDown_->render();

	SDL_Rect fill{DISPLAY_WIDTH, 0, GUI_WIDTH, DISPLAY_HEIGHT};
	SDL_FillRect(app_.screen_, &fill, 0);

	SDL_Rect fill2{0, DISPLAY_HEIGHT, DISPLAY_WIDTH + GUI_WIDTH, GUI_HEIGHT};
	SDL_FillRect(app_.screen_, &fill2, 0);

	for (auto& button : quickBar_)
		button->render({0, 0});

	for (auto& button : groupsBar_)
		button->render({0, 0});

	if (tooltip_) {
		if (app_.mouse_.pos.y > DISPLAY_HEIGHT)
			drawer_.applySurface(
				tetra::point<int>
				{app_.mouse_.pos.x - ((app_.mouse_.pos.x + tooltip_->w > app_.screenWidth_) ? tooltip_->w : 0),
				DISPLAY_HEIGHT - 3 - tooltip_->h}, tooltip_);
		else
			drawer_.applySurface(app_.mouse_.pos - tetra::point<int>
				{(app_.mouse_.pos.x + tooltip_->w > app_.screenWidth_) ? tooltip_->w : 0,
				tooltip_->h}, tooltip_);
	}
}

void GameMenu::step()
{
	tooltip_ = nullptr;

	for (auto& button : groupsBar_) {
		if (button->getGroup().newAvailable())
			setupGroupDropDown(button->getGroup());
		button->setHovered(button->inButton(app_.mouse_.pos));
	}

	for (auto& button : quickBar_)
		button->setHovered(button->inButton(app_.mouse_.pos));

	auto it = collectionProgress_.buttons.begin();
	while (it != collectionProgress_.buttons.end()) {
		if ((static_cast<CollectionProgressButton*>(it->get())->time -= 1.0 / FRAMERATECAP) < 0.0)
				it = collectionProgress_.buttons.erase(it);
		else
			++it;
	}

	if (dropDown_) {
		dropDown_->step();
		for (auto& button : dropDown_->buttons)
			button->setHovered(button->inButton(app_.mouse_.pos - dropDown_->offset));
	}
}

void GameMenu::handleClick(const tetra::point<int>& pos, Uint8 which)
{
	if (!(which == SDL_BUTTON_LEFT || which == SDL_BUTTON_MIDDLE ||
		which == SDL_BUTTON_RIGHT))
			return;

	for (auto& button : groupsBar_) {
		if (button->inButton(pos)) {
			button->onClick(pos, which);
			return;
		}
	}

	for (auto& button : quickBar_) {
		if (button->inButton(pos)) {
			bool hadDropDown = dropDown_;
			button->onClick(pos, which);
			if (hadDropDown)
				dropDown_ = nullptr;
			return;
		}
	}

	dropDown_ = nullptr;
}

void* GameMenu::dropDownId()
{
	return dropDown_;
}

bool GameMenu::inDropDown(const tetra::point<int>& pos)
{
	if (!dropDown_)
		return false;

	return dropDown_->inMenu(pos);
}

bool GameMenu::tryDropDown(const tetra::point<int>& pos, Uint8 which)
{
	if (!dropDown_ || (!(which == SDL_BUTTON_LEFT || which == SDL_BUTTON_MIDDLE ||
		which == SDL_BUTTON_RIGHT)) || !dropDown_->doneMoving())
		return false;

	for (auto& button : dropDown_->buttons) {
		if (button->inButton(pos - dropDown_->offset)) {
			button->onClick(pos - dropDown_->offset, which);
			return true;
		}
	}
	return false;
}

void GameMenu::clearDropDown()
{
	bool collectionTimeOut = true;
	for (auto& button : collectionProgress_.buttons) {
		if (static_cast<CollectionProgressButton*>(button.get())->time > 0.0) {
			collectionTimeOut = false;
			break;
		}
	}

	if (dropDown_ != &collectionProgress_ || collectionTimeOut)
		dropDown_ = nullptr;
}

void GameMenu::activateGroup(const tetra::point<int>& pos, int id)
{
	if (dropDown_ == &groupDropDowns_[id])
		dropDown_ = nullptr;
	else {
		dropDown_ = &groupDropDowns_[id];
		dropDown_->offset = pos;
		dropDown_->deltaOffset = {-20, 0};
		dropDown_->targetOffset = {pos.x - dropDown_->right(), pos.y};
	}
}

void GameMenu::activateMusicDropDown(const tetra::point<int>& pos)
{
	if (dropDown_ != &musicDropDown_) {
		dropDown_ = &musicDropDown_;
		dropDown_->offset = pos;
		dropDown_->deltaOffset = {0, -20};
		dropDown_->targetOffset = {pos.x, pos.y - dropDown_->bottom()};
	} else
		dropDown_ = nullptr;
}

void GameMenu::addQuickElement(const Element& elem)
{
	if (quickBar_.size() - firstQuickElemIndex_ == maxQuickElems_)
		return;

	for (auto it = quickBar_.begin() + firstQuickElemIndex_; it != quickBar_.end(); ++it)
		if (static_cast<ElementButton*>(it->get())->matches(elem))
			return;

	int x;

	if (quickBar_.empty())
		x = 0;
	else
	{
		x = quickBar_.back()->right();
		for (auto it = quickBar_.begin() + firstQuickElemIndex_; it != quickBar_.end(); ++it)
			if ((*it)->left() != (*(it - 1))->right())
			{
				x = (*(it - 1))->right();
				break;
			}
	}

	quickBar_.emplace_back(new ElementButton(*this, {x, DISPLAY_HEIGHT},
					{BUTTON_UNIT_SIZE * QUICK_UNITS, BUTTON_UNIT_SIZE * QUICK_UNITS}, elem));

	std::sort(quickBar_.begin() + firstQuickElemIndex_, quickBar_.end(),
			  [](const std::shared_ptr<MenuButton>& lhs, const std::shared_ptr<MenuButton>& rhs)
				{return lhs->left() < rhs->left();});
}

void GameMenu::removeQuickElement(const Element& elem)
{
	for (auto it = quickBar_.begin() + firstQuickElemIndex_; it != quickBar_.end(); ++it) {
		if (static_cast<ElementButton*>(it->get())->matches(elem)) {
			quickBar_.erase(it);
			return;
		}
	}
}

std::pair<std::vector<std::shared_ptr<MenuButton>>::iterator,
		  std::vector<std::shared_ptr<MenuButton>>::iterator>
GameMenu::getQuickElems()
{
	return {quickBar_.begin() + firstQuickElemIndex_, quickBar_.end()};
}

void GameMenu::addCollectionProgress(const Element& elem)
{
	dropDown_ = &collectionProgress_;

	int y = 0, prevY = 0;
	bool yFound = false;
	for (auto& button : collectionProgress_.buttons) {
		if (static_cast<CollectionProgressButton*>(button.get())->matches(elem))
			return;
		if (!yFound && button->top() != prevY) {
			y = button->top() - BUTTON_UNIT_SIZE;
			yFound = true;
		}
		prevY = button->bottom();
	}

	if (!yFound && collectionProgress_.buttons.size())
		y = collectionProgress_.buttons.back()->bottom();

	collectionProgress_.buttons.emplace_back(new CollectionProgressButton(*this, {0, y},
											 {BUTTON_UNIT_SIZE * 5, BUTTON_UNIT_SIZE}, elem));

	std::sort(collectionProgress_.buttons.begin(), collectionProgress_.buttons.end(),
		[](const std::shared_ptr<MenuButton>& lhs, const std::shared_ptr<MenuButton>& rhs)
			{return lhs->top() < rhs->top();});
}

}
