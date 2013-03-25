#include "PixelCosm.hpp"

namespace PixelCosm
{

UpdateFunctions::UpdateFunctions(tetra::Sdl::Application* app, tetra::Sdl::MouseInfo& mouseInfo) :
	FunctionSet(app, mouseInfo),
	app_(static_cast<PixelCosmApp*>(app)),
	start_({app_->screenWidth() / 2 - 3 * (BUTTON_UNIT_SIZE * GROUP_UNITS),
			DISPLAY_HEIGHT - BUTTON_UNIT_SIZE * GROUP_UNITS},
		   {BUTTON_UNIT_SIZE * GROUP_UNITS, BUTTON_UNIT_SIZE * QUICK_UNITS},
		   UpdateButton::Type::Start),
	cancel_({app_->screenWidth() / 2 + 2 * BUTTON_UNIT_SIZE * GROUP_UNITS,
			DISPLAY_HEIGHT - BUTTON_UNIT_SIZE * GROUP_UNITS},
			{BUTTON_UNIT_SIZE * GROUP_UNITS, BUTTON_UNIT_SIZE * QUICK_UNITS},
			UpdateButton::Type::Cancel),
	writer_(app_->drawer_)
{
#ifdef DOUBLE_DISPLAY
	writer_.openFont("resources/font.ttf", 16);
#else
	writer_.openFont("resources/font.ttf", 10);
#endif
	message_ = writer_.createWrapped(
		"Automatic updating is not yet supported. "
//		"Run update.bat (Windows) or update.sh (Linux) to update."
		"Go to 'www.sourceforge.net/projects/pixelcosm/' to manually download the newest version.",
		DISPLAY_WIDTH);
}

UpdateFunctions::~UpdateFunctions()
{
	SDL_FreeSurface(message_);
}

void UpdateFunctions::renderGUI()
{
	app_->drawer_.rect(
			{5, 5}, {app_->screenWidth() - 5, app_->screenHeight() - 5},
			app_->drawer_.mapRgb(64, 64, 64));

	app_->drawer_.applySurface({10, 10}, message_);

	start_.render(app_->drawer_);
	cancel_.render(app_->drawer_);
}

void UpdateFunctions::stepGame()
{

}

bool UpdateFunctions::handleEvent(const SDL_Event& e)
{
	if (e.type == SDL_QUIT)
		return true;
	else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
		if (start_.inButton({e.button.x, e.button.y}))
			start();
		else if (cancel_.inButton({e.button.x, e.button.y}))
			cancel();
	}
	return false;
}

void UpdateFunctions::start()
{
	updater_.update();
	app_->setFunctions<TitleFunctions>();
}

void UpdateFunctions::cancel()
{
	updater_.cancel();
	app_->setFunctions<TitleFunctions>();
}

void Updater::update()
{
}

void Updater::cancel()
{
}

UpdateButton::UpdateButton(const tetra::point<int>& pos, const tetra::point<int>& dim, Type type) :
	pos_(pos), dim_(dim), type_(type),
	image_((type_ == Type::Start) ?
		condDouble(tetra::Sdl::Application::loadImage("resources/menu/start.bmp")) :
		condDouble(tetra::Sdl::Application::loadImage("resources/menu/cancel.bmp"))) {}

void UpdateButton::render(tetra::Sdl::ScreenDrawer& drawer)
{
	const tetra::point<int> renderPos = tetra::point<int>{pos_.x + (dim_.x - image_->w) / 2, pos_.y};
	SDL_Rect rect{Sint16(pos_.x), Sint16(pos_.y), Uint16(dim_.x), Uint16(dim_.y)};

	SDL_FillRect(drawer.getSurface(), &rect, 0);
	drawer.applySurface(renderPos, image_);

	static tetra::Sdl::Pixel
		gridPixel = drawer.mapRgb(64, 64, 64),
		buttonPixel = drawer.mapRgb(128, 128, 128),
		highlightPixel = drawer.mapRgb(160, 160, 160);

	drawer.rect(pos_, pos_ + (dim_ - 1), buttonPixel);

	tetra::point<int>
		hl1 = pos_ + tetra::point<int>{dim_.x - 2, 1},
		hl2 = pos_ + (dim_ - 2),
		hl3 = pos_ + tetra::point<int>{1, dim_.y - 2};

	drawer.chain(highlightPixel, hl1, hl2, hl3);

	drawer.putPoint(pos_ + tetra::point<int>{1, 1}, buttonPixel);
	drawer.putPoint(pos_ + (dim_ - 1) - tetra::point<int>{1, 1}, buttonPixel);

	drawer.putPoint(pos_,  gridPixel);
	drawer.putPoint(pos_ + tetra::point<int>{1, 0}, gridPixel);
	drawer.putPoint(pos_ + tetra::point<int>{0, 1}, gridPixel);

	drawer.putPoint(pos_ + (dim_ - 1), gridPixel);
	drawer.putPoint(pos_ + (dim_ - 1) - tetra::point<int>{1, 0}, gridPixel);
	drawer.putPoint(pos_ + (dim_ - 1) - tetra::point<int>{0, 1}, gridPixel);
}

bool UpdateButton::inButton(const tetra::point<int>& pos)
{
	return (pos.x > pos_.x) && (pos.y > pos_.y) &&
		   (pos.x < pos_.x + dim_.x) && (pos.y < pos_.y + dim_.y);
}

}
