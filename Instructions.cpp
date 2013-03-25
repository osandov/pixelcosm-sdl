#include "PixelCosm.hpp"

constexpr const char* playAreaInstructions =
" - Left-click to draw particles\n"
" - Right-click to use the selected tool (e.g. Repel, Inspect)\n"
" - Middle-click (or left-click while holding Shift) to erase\n"
" - Use the scroll-wheel or arrow keys to change pen size\n"
" - The play area will be saved between instances of the game\n"
" - It can also be manually saved using the in-game buttons";

constexpr const char* groupsMenuInstructions =
" - Elements are sorted into Groups, and within them, Subgroups\n"
" - More Elements are added to the menu as they are discovered\n"
" - Left-click on a Group in order to expand or retract it\n"
" - Left-click on an Element in order to select it\n"
" - The currently selected Element will be highlighted\n"
" - You may also right-click to add the Element to the Quick Bar or "
"middle-click (again, left-click while holding Shift is equivalent) to remove it";

constexpr const char* toolInstructions =
"Tools allow for different manners of interaction with the play area "
"and are used by right-clicking. They are scrolled through by left- and right-"
"clicking the Tool button.";

constexpr const char* repelToolInstructions =
"The Repel Tool generates an outward force";

constexpr const char* dragToolInstructions =
"The Drag Tool grabs and drags nearby particles";

constexpr const char* angleToolInstructions =
"The Angle Tool creates a linear force in the direction of the pointer";

constexpr const char* inspectToolInstructions =
"The Inspect Tool magnifies a small radius under the mouse, identifies "
"Elements and can be used to discover new ones (see next page)";

constexpr const char* discoveryInstructions =
" - Although the initial selection of Elements is basic, many "
"more can be created through interactions between them\n"
" - These new Elements can then be collected and used\n"
" - In order to collect new Elements, set your tool to Inspect";

constexpr const char* discoveryInstructions_2 =
" - Then, hold right-click and drag it over the new Element\n"
" - A progress bar will appear in the top-left corner of the screen\n"
" - When the bar fills up, you can find the Element in the Groups menu and use it";

namespace PixelCosm
{

InstructionNav::InstructionNav(InstructionFunctions& func, const tetra::point<int>& pos, NavType type) :
	func_(func),
	image_((type == NavType::Back) ?
		condDouble(tetra::Sdl::Application::loadImage("resources/menu/back_page.bmp")) :
		(type == NavType::Forward) ?
			condDouble(tetra::Sdl::Application::loadImage("resources/menu/forward_page.bmp")) :
			condDouble(tetra::Sdl::Application::loadImage("resources/menu/title_screen.bmp"))),
	pos_(pos),
	dim_(image_->w, image_->h),
	type_(type) {}

InstructionNav::~InstructionNav()
{
	SDL_FreeSurface(image_);
}

void InstructionNav::render(tetra::Sdl::ScreenDrawer& drawer) const
{
	drawer.applySurface(pos_, image_);
}

void InstructionNav::handleClick(const tetra::point<int>& pos)
{
	if ((pos.x >= pos_.x) && (pos.x < pos_.x + dim_.x) &&
		(pos.y >= pos_.y) && (pos.y < pos_.y + dim_.y))
	{
		if (type_ == NavType::Back)
			func_.back_page();
		else if (type_ == NavType::Home)
			func_.home();
		else if (type_ == NavType::Forward)
			func_.forward_page();
	}
}

InstructionObject::InstructionObject(const tetra::point<int>& pos, SDL_Surface* image) :
	pos_(pos), image_(image) {}

InstructionObject::~InstructionObject()
{
	SDL_FreeSurface(image_);
}

int InstructionObject::left()
{
	return pos_.x;
}

int InstructionObject::right()
{
	return pos_.x + image_->w;
}

int InstructionObject::top()
{
	return pos_.y;
}

int InstructionObject::bottom()
{
	return pos_.y + image_->h;
}

void InstructionObject::render(tetra::Sdl::ScreenDrawer& drawer) const
{
	drawer.applySurface(pos_, image_);
}

InstructionPage::InstructionPage(InstructionFunctions& func) :
	func_(func), drawer_(func.getDrawer()),
	back_(func, {func.screenWidth() - int(67.5 * DISPLAY_FACTOR), DISPLAY_HEIGHT - 5}, InstructionNav::NavType::Back),
	home_(func, {func.screenWidth() - 45 * DISPLAY_FACTOR, DISPLAY_HEIGHT - 5}, InstructionNav::NavType::Home),
	forward_(func, {func.screenWidth() - int(22.5 * DISPLAY_FACTOR), DISPLAY_HEIGHT - 5}, InstructionNav::NavType::Forward) {}

int InstructionPage::left()
{
	if (objects_.empty())
		return 0;
	else
		return objects_.back().left();
}

int InstructionPage::top()
{
	if (objects_.empty())
		return 0;
	else
		return objects_.back().top();
}

int InstructionPage::right()
{
	if (objects_.empty())
		return 0;
	else
		return objects_.back().right();
}

int InstructionPage::bottom()
{
	if (objects_.empty())
		return 0;
	else
		return objects_.back().bottom();
}

void InstructionPage::reserve(std::size_t size)
{
	objects_.reserve(size);
}

void InstructionPage::add_object(const tetra::point<int>& pos, SDL_Surface* image)
{
	objects_.emplace_back(pos, image);
}

void InstructionPage::render() const
{
	for (auto& object : objects_)
		object.render(drawer_);
	if (func_.currentPage() > 0)
		back_.render(drawer_);
	home_.render(drawer_);
	if (func_.currentPage() < func_.pages() - 1)
		forward_.render(drawer_);
}

void InstructionPage::handleClick(const tetra::point<int>& pos)
{
	back_.handleClick(pos);
	home_.handleClick(pos);
	forward_.handleClick(pos);
}

InstructionFunctions::InstructionFunctions(tetra::Sdl::Application* app, tetra::Sdl::MouseInfo& mouseInfo) :
	FunctionSet(app, mouseInfo),
	app_(static_cast<PixelCosmApp*>(app))
{
	tetra::Sdl::Writer title(app_->drawer_, "resources/font.ttf", 12 * DISPLAY_FACTOR);
	tetra::Sdl::Writer body(app_->drawer_, "resources/font.ttf", 10 * DISPLAY_FACTOR);

	static constexpr int LEFT = 10;
	static constexpr int TOP = 10;

	pages_.reserve(4);

	// Play Area
	pages_.emplace_back(*this);
	pages_.back().reserve(3);
	pages_.back().add_object({100, 200},
		app_->loadImage("resources/instructions/play_area.bmp"));
	pages_.back().add_object({LEFT, TOP}, title.createLine("Play Area"));
	pages_.back().add_object({LEFT, pages_.back().bottom()},
		body.createWrapped(playAreaInstructions, DISPLAY_WIDTH));

	// Groups Menu
	pages_.emplace_back(*this);
	pages_.back().reserve(4);
	pages_.back().add_object({LEFT, TOP}, title.createLine("Groups Menu"));
	pages_.back().add_object({LEFT, pages_.back().bottom()},
		body.createWrapped(groupsMenuInstructions, DISPLAY_WIDTH));
	pages_.back().add_object({100, pages_.back().bottom() + 10},
		app_->loadImage("resources/instructions/groups_menu.bmp"));

	// Tools
	pages_.emplace_back(*this);
	pages_.back().reserve(10);
	pages_.back().add_object({LEFT, TOP}, title.createLine("Tools"));
	pages_.back().add_object({LEFT, pages_.back().bottom()},
		body.createWrapped(toolInstructions, DISPLAY_WIDTH));

	pages_.back().add_object({20, pages_.back().bottom() + 15},
		condDouble(app_->loadImage("resources/menu/repel_tool.bmp")));
	int temp = pages_.back().bottom();
	pages_.back().add_object({pages_.back().right() + 10, pages_.back().top()},
		body.createWrapped(repelToolInstructions, DISPLAY_WIDTH - 40));

	pages_.back().add_object({20, temp + 15},
		condDouble(app_->loadImage("resources/menu/drag_tool.bmp")));
	temp = pages_.back().bottom();
	pages_.back().add_object({pages_.back().right() + 10, pages_.back().top()},
		body.createWrapped(dragToolInstructions, DISPLAY_WIDTH - 40));

	pages_.back().add_object({20, temp + 15},
		condDouble(app_->loadImage("resources/menu/angle_tool.bmp")));
	temp = pages_.back().bottom();
	pages_.back().add_object({pages_.back().right() + 10, pages_.back().top()},
		body.createWrapped(angleToolInstructions, DISPLAY_WIDTH - 40));

	pages_.back().add_object({20, temp + 15},
		condDouble(app_->loadImage("resources/menu/inspect_tool.bmp")));
	temp = pages_.back().bottom();
	pages_.back().add_object({pages_.back().right() + 10, pages_.back().top()},
		body.createWrapped(inspectToolInstructions, DISPLAY_WIDTH - 40));

	// Element Discovery
	pages_.emplace_back(*this);
	pages_.back().reserve(5);
	pages_.back().add_object({LEFT, TOP}, title.createLine("Element Discovery"));
	pages_.back().add_object({LEFT, pages_.back().bottom()},
		body.createWrapped(discoveryInstructions, DISPLAY_WIDTH));
	pages_.back().add_object({100, pages_.back().bottom() + 5},
		condDouble(app_->loadImage("resources/menu/inspect_tool.bmp")));
	pages_.back().add_object({LEFT, pages_.back().bottom()},
		body.createWrapped(discoveryInstructions_2, DISPLAY_WIDTH));
	pages_.back().add_object({100, pages_.back().bottom() + 5},
		app_->loadImage("resources/instructions/collect.bmp"));
}

tetra::Sdl::ScreenDrawer& InstructionFunctions::getDrawer()
{
	return app_->drawer_;
}

int InstructionFunctions::screenWidth()
{
	return app_->screenWidth();
}

int InstructionFunctions::screenHeight()
{
	return app_->screenWidth();
}

std::size_t InstructionFunctions::currentPage()
{
	return currentPage_;
}

std::size_t InstructionFunctions::pages()
{
	return pages_.size();
}

void InstructionFunctions::renderGUI()
{
	app_->drawer_.rect(
		{5, 5}, {app_->screenWidth() - 5, app_->screenHeight() - 5},
		app_->drawer_.mapRgb(64, 64, 64));
	if (!pages_.empty())
		pages_[currentPage_].render();
}

bool InstructionFunctions::handleEvent(const SDL_Event& e)
{
	if (defaultHandleEvent(e) == true)
		return true;
	if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
		if (!pages_.empty())
			pages_[currentPage_].handleClick({e.button.x, e.button.y});
	return false;
}

void InstructionFunctions::back_page()
{
	if (currentPage_ > 0)
		--currentPage_;
}

void InstructionFunctions::forward_page()
{
	if (currentPage_ + 1 < pages_.size())
		++currentPage_;
}

void InstructionFunctions::home()
{
	app_->setFunctions<TitleFunctions>();
}

}
