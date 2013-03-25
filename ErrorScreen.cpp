#include "PixelCosm.hpp"

namespace PixelCosm
{

ErrorScreenFunctions::ErrorScreenFunctions(tetra::Sdl::Application* app, tetra::Sdl::MouseInfo& mouseInfo) :
	FunctionSet(app, mouseInfo),
	app_(static_cast<PixelCosmApp*>(app)),
	message_(app_->writer_.createWrapped(app_->errorStream_.str(), DISPLAY_WIDTH + GUI_WIDTH)),
	scrollAmount_(app_->writer_.fontHeight()) {}

ErrorScreenFunctions::~ErrorScreenFunctions()
{
	SDL_FreeSurface(message_);
}

void ErrorScreenFunctions::renderGUI()
{
	app_->drawer_.applySurface({4, scrollY_ + 1}, message_);
}

bool ErrorScreenFunctions::handleEvent(const SDL_Event& e)
{
	if (defaultHandleEvent(e) == true)
		return true;
	if (e.type == SDL_MOUSEBUTTONDOWN) {
		if (e.button.button == SDL_BUTTON_WHEELUP) {
			scrollY_ += scrollAmount_;
			if (scrollY_ > 0)
				scrollY_ = 0;
		} else if (e.button.button == SDL_BUTTON_WHEELDOWN) {
			scrollY_ -= scrollAmount_;
			if (scrollY_ + message_->h < DISPLAY_HEIGHT + GUI_HEIGHT)
				scrollY_ += scrollAmount_;
		}
	}
	return false;
}

}
