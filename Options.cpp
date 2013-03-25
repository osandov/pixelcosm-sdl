#include "PixelCosm.hpp"

namespace PixelCosm
{

Option::Option(OptionFunctions& func, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	func_(func), drawer_(func.getDrawer()), pos_(pos), dim_(dim) {}

void Option::renderButton()
{
	static tetra::Sdl::Pixel
		gridPixel = drawer_.mapRgb(64, 64, 64),
		buttonPixel = drawer_.mapRgb(128, 128, 128),
		highlightPixel = drawer_.mapRgb(160, 160, 160);

	drawer_.rect(pos_, pos_ + (dim_ - 1), buttonPixel);

	tetra::point<int>
		hl1 = pos_ + tetra::point<int>{dim_.x - 2, 1},
		hl2 = pos_ + (dim_ - 2),
		hl3 = pos_ + tetra::point<int>{1, dim_.y - 2};

	drawer_.chain(highlightPixel, hl1, hl2, hl3);

	drawer_.putPoint(pos_ + tetra::point<int>{1, 1}, buttonPixel);
	drawer_.putPoint(pos_ + (dim_ - 1) - tetra::point<int>{1, 1}, buttonPixel);

	drawer_.putPoint(pos_,  gridPixel);
	drawer_.putPoint(pos_ + tetra::point<int>{1, 0}, gridPixel);
	drawer_.putPoint(pos_ + tetra::point<int>{0, 1}, gridPixel);

	drawer_.putPoint(pos_ + (dim_ - 1), gridPixel);
	drawer_.putPoint(pos_ + (dim_ - 1) - tetra::point<int>{1, 0}, gridPixel);
	drawer_.putPoint(pos_ + (dim_ - 1) - tetra::point<int>{0, 1}, gridPixel);
}

TitleScreenOption::TitleScreenOption(OptionFunctions& func, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	Option(func, pos, dim),
	icon_(condDouble(tetra::Sdl::Application::loadImage("resources/menu/title_screen.bmp"))) {}

TitleScreenOption::~TitleScreenOption()
{
	SDL_FreeSurface(icon_);
}

void TitleScreenOption::render()
{
	static const tetra::point<int> renderPos = tetra::point<int>{pos_.x + (dim_.x - icon_->w) / 2, pos_.y};
	static SDL_Rect rect{Sint16(pos_.x), Sint16(pos_.y), Uint16(dim_.x), Uint16(dim_.y)};
	SDL_FillRect(drawer_.getSurface(), &rect, 0);
	drawer_.applySurface(renderPos, icon_);
	renderButton();
}

void TitleScreenOption::handle(const SDL_Event& e)
{
	if (e.type == SDL_MOUSEBUTTONDOWN) {
		if (e.button.button == SDL_BUTTON_LEFT) {
			if (inOption({e.button.x, e.button.y}))
				func_.title();
		}
	}
}

VolumeOption::VolumeOption(OptionFunctions& func, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	Option(func, pos, dim), label_(func_.getWriter().createLine("VOLUME", tetra::Sdl::TextStyle::Blended)),
	minPos(pos_.x + label_->w + 15), maxPos(minPos + MIX_MAX_VOLUME) {}

VolumeOption::~VolumeOption()
{
	SDL_FreeSurface(label_);
}

void VolumeOption::render()
{
	int volume = func_.getVolume();
	int volumePos = volume;

	drawer_.applySurface(pos_, label_);
	std::stringstream ss;
	ss << volume;

	func_.getWriter().writeLine({maxPos + 10, pos_.y}, ss.str(),
								 tetra::Sdl::TextStyle::Blended);

	drawer_.filled_rect({minPos, pos_.y},
						{minPos + volumePos, pos_.y + label_->h - 5},
						drawer_.mapRgb(255, 255, 255));

	drawer_.rect({minPos, pos_.y},
				 {maxPos, pos_.y + label_->h - 5},
				 drawer_.mapRgb(64, 64, 64));
}

void VolumeOption::handle(const SDL_Event& e)
{
	if (e.type == SDL_MOUSEBUTTONDOWN) {
		if (inOption({e.button.x, e.button.y})) {
			if (e.button.button == SDL_BUTTON_LEFT) {
				if (e.button.x > minPos && e.button.x < maxPos)
				{
					wasClicked_ = true;
					func_.setVolume(e.button.x - minPos);
				}
			} else if (e.button.button == SDL_BUTTON_WHEELDOWN)
				func_.setVolume(func_.getVolume() - MIX_MAX_VOLUME / 16);
			else if (e.button.button == SDL_BUTTON_WHEELUP)
				func_.setVolume(func_.getVolume() + MIX_MAX_VOLUME / 16);
		}
	} else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
		wasClicked_ = false;
	else if (e.type == SDL_MOUSEMOTION && wasClicked_) {
		if (e.motion.x < minPos)
			func_.setVolume(0);
		else if (e.motion.x > maxPos)
			func_.setVolume(MIX_MAX_VOLUME);
		else
			func_.setVolume(e.motion.x - minPos);
	}
}

FullscreenOption::FullscreenOption(OptionFunctions& func, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	Option(func, pos, dim), label_(func_.getWriter().createLine("FULLSCREEN", tetra::Sdl::TextStyle::Blended)),
#ifndef FORCE_FULLSCREEN
	windowed_(tetra::Sdl::Application::loadImage("resources/menu/windowed.bmp")),
	fullscreen_(tetra::Sdl::Application::loadImage("resources/menu/fullscreen.bmp"))
#else
	windowed_(nullptr),
	fullscreen_(nullptr)
#endif
	{}

FullscreenOption::~FullscreenOption()
{
	SDL_FreeSurface(label_);
}

void FullscreenOption::render()
{
	drawer_.applySurface(pos_ + tetra::point<int>{label_->w + 15, -16},
						(func_.getFullscreen()) ? fullscreen_ : windowed_);
	drawer_.applySurface(pos_, label_);
}

void FullscreenOption::handle(const SDL_Event& e)
{
	if (e.type == SDL_MOUSEBUTTONDOWN
		&& inOption({e.button.x, e.button.y}))
			func_.toggleFullscreen();
}

OptionFunctions::OptionFunctions(tetra::Sdl::Application* app, tetra::Sdl::MouseInfo& mouseInfo) :
	FunctionSet(app, mouseInfo),
	app_(static_cast<PixelCosmApp*>(app)),
	writer_(app_->drawer_)
{
	writer_.openFont("resources/font2.ttf", 12 * DISPLAY_FACTOR);
	options_.emplace_back(new TitleScreenOption(*this,
		{DISPLAY_WIDTH, DISPLAY_HEIGHT},
		{BUTTON_UNIT_SIZE * GROUP_UNITS, BUTTON_UNIT_SIZE * 2}));

	options_.emplace_back(new VolumeOption(*this,
		{BUTTON_UNIT_SIZE * 2, BUTTON_UNIT_SIZE * 2 * DISPLAY_FACTOR},
		{BUTTON_UNIT_SIZE * 15, BUTTON_UNIT_SIZE * 2}));

#ifndef FORCE_FULLSCREEN
	options_.emplace_back(new FullscreenOption(*this,
		{BUTTON_UNIT_SIZE * 2, BUTTON_UNIT_SIZE * 4 * DISPLAY_FACTOR},
		{BUTTON_UNIT_SIZE * 15, BUTTON_UNIT_SIZE * 2}));
#endif

	createTriangles(tris_, 25, app_->drawer_);
}

OptionFunctions::~OptionFunctions()
{

}

void OptionFunctions::renderGUI()
{
	for (const DisplayTriangle<float>& tri : tris_)
		tri.render(app_->drawer_);

	static SDL_Surface* optionsLabel = writer_.createLine("OPTIONS", tetra::Sdl::TextStyle::Blended);
	app_->drawer_.applySurface({BUTTON_UNIT_SIZE, BUTTON_UNIT_SIZE}, optionsLabel);

	app_->drawer_.rect(
		{5, 5}, {app_->screenWidth() - 5, app_->screenHeight() - 5},
		app_->drawer_.mapRgb(64, 64, 64));

	for (auto& option : options_)
		option->render();
}

void OptionFunctions::stepGame()
{
	for (DisplayTriangle<float>& tri : tris_)
			tri.rotate(-1.0 / 20.0);

	if (returnToTitle_)
		app_->setFunctions<TitleFunctions>();
}

bool OptionFunctions::handleEvent(const SDL_Event& e)
{
	if (e.type == SDL_QUIT)
		return true;
	else {
		for (auto& option : options_)
			option->handle(e);
	}
	return false;
}

tetra::Sdl::ScreenDrawer& OptionFunctions::getDrawer()
{
	return app_->drawer_;
}

tetra::Sdl::Writer& OptionFunctions::getWriter()
{
	return writer_;
}

void OptionFunctions::setVolume(int volume)
{
	return app_->musicGen_.setVolume(volume);
}

int OptionFunctions::getVolume()
{
	return app_->musicGen_.getVolume();
}

bool OptionFunctions::getFullscreen() const
{
	return app_->getFullscreen();
}

void OptionFunctions::toggleFullscreen()
{
		app_->toggleFullscreen();
}

void OptionFunctions::title()
{
	returnToTitle_ = true;
}

}
