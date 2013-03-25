#include "PixelCosm.hpp"

namespace PixelCosm
{

TitleButton::TitleButton(TitleFunctions* instance, const tetra::point<int>& pos, tetra::Sdl::Writer& writer,
						 const std::string& text, void (TitleFunctions::*selectFunction)()) :
	pos_(pos), image_(writer.createLine(text, tetra::Sdl::TextStyle::Blended))
{
	select = std::bind(selectFunction, instance);
}

TitleButton::~TitleButton()
{
	SDL_FreeSurface(image_);
}

void TitleButton::render(tetra::Sdl::ScreenDrawer& drawer)
{
	drawer.applySurface({left(), top()}, image_);
}

int TitleButton::top()
{
	return pos_.y;
}

int TitleButton::bottom()
{
	return pos_.y + image_->h;
}

int TitleButton::left()
{
	return pos_.x - image_->w / 2;
}

int TitleButton::right()
{
	return pos_.x + image_->w / 2;
}

int TitleButton::width()
{
	return image_->w;
}

int TitleButton::height()
{
	return image_->h;
}

tetra::point<int> TitleButton::selectPos()
{
	return {left() - 20, pos_.y + image_->h / 2};
}

TitleButtonSet::TitleButtonSet(TitleFunctions& instance, const tetra::point<int>& pos) :
	instance_(instance), pos_(pos), writer_(instance.getDrawer()),
	selectTri_({0, 0}, 7.5 * DISPLAY_FACTOR, 0.0, instance.getDrawer().mapRgb(255, 255, 255))
{
	writer_.openFont("resources/font2.ttf", 16 * DISPLAY_FACTOR);
}

void TitleButtonSet::reserve(std::size_t n)
{
	buttons_.reserve(n);
}

void TitleButtonSet::addButton(const std::string& text, void (TitleFunctions::*selectFunction)())
{
	if (buttons_.empty())
		buttons_.emplace_back(&instance_, pos_, writer_, text, selectFunction);
	else
		buttons_.emplace_back(&instance_,
			tetra::point<int>{pos_.x, buttons_.back().bottom() + 5}, writer_, text, selectFunction);
}

void TitleButtonSet::closeFont()
{
	writer_.closeFont();
}

int TitleButtonSet::maxWidth()
{
	int max = 0;
	for (auto& button : buttons_) {
		if (button.width() > max)
			max = button.width();
	}
	return max;
}

bool TitleButtonSet::checkMouse(const tetra::point<int>& pos)
{
	int w = maxWidth() / 2;
	for (std::size_t i = 0; i < buttons_.size(); ++i) {
		if (pos.isInRect({pos_.x - w, buttons_[i].top()},
						 {pos_.x + w, buttons_[i].bottom()}))
		{
			selected_ = i;
			return true;
		}
	}
	return false;
}

void TitleButtonSet::stepForward()
{
	if (++selected_ == buttons_.size()) // Intentional increment
		selected_ = 0;
}

void TitleButtonSet::stepBack()
{
	if (selected_ == 0)
		selected_ = buttons_.size();
	--selected_;
}

void TitleButtonSet::selectButton()
{
	buttons_[selected_].select();
}

void TitleButtonSet::render(tetra::Sdl::ScreenDrawer& drawer)
{
	for (auto& button : buttons_)
		button.render(drawer);

	selectTri_.mid() = buttons_[selected_].selectPos();
	selectTri_.render(drawer);
	selectTri_.rotate(1.0 / 20.0);
}

TitleFunctions::TitleFunctions(tetra::Sdl::Application* app, tetra::Sdl::MouseInfo& mouseInfo) :
	FunctionSet(app, mouseInfo),
	app_(static_cast<PixelCosmApp*>(app)),
#ifdef DOUBLE_DISPLAY
	title_(app_->loadImage("resources/title/title.bmp")),
#else
	title_(app_->loadImage("resources/title/title-small.bmp")),
#endif
	logo_(app_->loadImage("resources/title/logo.bmp")),
	buttons_(*this, {app_->screenWidth() / 2, 25 * DISPLAY_FACTOR + title_->h})
{
	buttons_.reserve(5);
	buttons_.addButton("SINGLEPLAYER", &TitleFunctions::singlePlayer);
//	buttons_.addButton("MULTIPLAYER", &TitleFunctions::multiPlayer);
	buttons_.addButton("INSTRUCTIONS", &TitleFunctions::instructions);
	buttons_.addButton("OPTIONS", &TitleFunctions::options);
	buttons_.addButton("UPDATE", &TitleFunctions::update);
	buttons_.addButton("QUIT", &TitleFunctions::quit);
	buttons_.closeFont();

	createTriangles(tris_, 25, app_->drawer_);

	if (!Mix_PlayingMusic())
		Mix_PlayMusic(app_->music_, -1);
}

TitleFunctions::~TitleFunctions()
{
	SDL_FreeSurface(title_);
	SDL_FreeSurface(logo_);
	SDL_FreeSurface(transSurface_);
}

void TitleFunctions::renderGUI()
{
	if (!transing_) {
		app_->drawer_.rect(
			{5, 5}, {app_->screenWidth() - 5, app_->screenHeight() - 5},
			app_->drawer_.mapRgb(64, 64, 64));
		for (const DisplayTriangle<float>& tri : tris_)
			tri.render(app_->drawer_);

		static tetra::point<int> titlePos{(app_->screenWidth() - title_->w) / 2, int(12.5 * DISPLAY_FACTOR)};
		static tetra::point<int> logoPos{app_->screenWidth() - logo_->w - 5,
										 app_->screenHeight() - logo_->h - 5};

		app_->drawer_.applySurface(titlePos, title_);
		app_->drawer_.applySurface(logoPos, logo_);
		buttons_.render(app_->drawer_);
	} else {
		SDL_SetAlpha(transSurface_, SDL_SRCALPHA, SDL_ALPHA_OPAQUE * transTime_);
		app_->drawer_.applySurface(transSurface_);

		static std::uniform_int_distribution<int> uid(0, 3);
		tetra::point<int> pos;
#ifdef DOUBLE_DISPLAY
		for (pos.x = 1; pos.x < app_->screenWidth() / 2 - 1; ++pos.x) {
			for (pos.y = 1; pos.y < app_->screenHeight() / 2 - 1; ++pos.y) {
				tetra::point<int> swapFrom = pos * 2;
				tetra::point<int> swapTo = (pos + Direction::dir(uid(random_engine))) * 2;
				std::swap(tetra::Sdl::rwPixel(swapFrom, transSurface_),
					tetra::Sdl::rwPixel(swapTo, transSurface_));

				++swapTo.x;
				++swapFrom.x;
				std::swap(tetra::Sdl::rwPixel(swapFrom, transSurface_),
					tetra::Sdl::rwPixel(swapTo, transSurface_));

				++swapTo.y;
				++swapFrom.y;
				std::swap(tetra::Sdl::rwPixel(swapFrom, transSurface_),
					tetra::Sdl::rwPixel(swapTo, transSurface_));

				--swapTo.x;
				--swapFrom.x;
				std::swap(tetra::Sdl::rwPixel(swapFrom, transSurface_),
					tetra::Sdl::rwPixel(swapTo, transSurface_));
			}
		}
#else
		for (pos.x = 1; pos.x < app_->screenWidth() - 1; ++pos.x) {
			for (pos.y = 1; pos.y < app_->screenHeight() - 1; ++pos.y) {
				std::swap(tetra::Sdl::rwPixel(pos, transSurface_),
					tetra::Sdl::rwPixel(pos + Direction::dir(uid(random_engine)), transSurface_));
			}
		}
#endif
	}
}

void TitleFunctions::stepGame()
{
	if (transing_) {
#ifndef QVGA
		if ((transTime_ -= 1.0 / FRAMERATECAP) < 0.0) // Intentional assignment
#endif // QVGA
			app_->setFunctions<PlayFunctions>();
	} else
		for (DisplayTriangle<float>& tri : tris_)
			tri.rotate(-1.0 / 20.0);
}

tetra::Sdl::ScreenDrawer& TitleFunctions::getDrawer()
{
	return app_->drawer_;
}

void TitleFunctions::singlePlayer()
{
	transing_ = true;
	transSurface_ =
		SDL_CreateRGBSurface(SDL_SWSURFACE, app_->screenWidth(), app_->screenHeight(),
			app_->screenDepth(), 0, 0, 0, 0);
	SDL_BlitSurface(app_->screen_, nullptr, transSurface_, nullptr);
	Mix_FadeOutMusic(1000 * transTime_);
}

void TitleFunctions::multiPlayer()
{

}

void TitleFunctions::instructions()
{
	app_->setFunctions<InstructionFunctions>();
}

void TitleFunctions::options()
{
	app_->setFunctions<OptionFunctions>();
}

void TitleFunctions::update()
{
	app_->setFunctions<UpdateFunctions>();
}

void TitleFunctions::quit()
{
	app_->quit_ = true;
}

bool TitleFunctions::handleEvent(const SDL_Event& e)
{
	if (e.type == SDL_QUIT)
		return true;
	else if (e.type == SDL_MOUSEMOTION)
		buttons_.checkMouse({e.motion.x, e.motion.y});
	else if (e.type == SDL_MOUSEBUTTONDOWN) {
		if (e.button.button == SDL_BUTTON_LEFT) {
			if (buttons_.checkMouse({e.button.x, e.button.y}))
				buttons_.selectButton();
		}
	} else if (e.type == SDL_KEYDOWN) {
		if (e.key.keysym.sym == SDLK_RETURN)
			buttons_.selectButton();
		if (e.key.keysym.sym == SDLK_DOWN)
			buttons_.stepForward();
		else if (e.key.keysym.sym == SDLK_UP)
			buttons_.stepBack();
	}
	return false;
}

void createTriangles(std::vector<DisplayTriangle<float>>& triangles, std::size_t N, tetra::Sdl::ScreenDrawer& drawer)
{
	triangles.reserve(N);
	using Dist = std::uniform_real_distribution<float>;
	Dist urd;
	using ColorDist = std::uniform_int_distribution<int>;
	ColorDist uid;

	for (std::size_t i = 0; i < N; ++i) {
		tetra::point<int> mid;
		float h = urd(random_engine, Dist::param_type{2.5, 12.5}) * DISPLAY_FACTOR;

		mid.x = int(urd(random_engine, Dist::param_type{0, DISPLAY_WIDTH}));
		mid.y = int(urd(random_engine, Dist::param_type{0, DISPLAY_HEIGHT}));

		float theta = urd(random_engine, Dist::param_type{0, 2.0 * M_PI});

		tetra::Sdl::Pixel pixel;
		switch (uid(random_engine, ColorDist::param_type{0, 5}))
		{
			case 0:
				pixel = drawer.mapRgb(0, 0, 128);
			break;
			case 1:
				pixel = drawer.mapRgb(0, 128, 0);
			break;
			case 2:
				pixel = drawer.mapRgb(128, 0, 0);
			break;
			case 3:
				pixel = drawer.mapRgb(128, 0, 255);
			break;
			case 4:
				pixel = drawer.mapRgb(0, 128, 255);
			break;
			case 5:
				pixel = drawer.mapRgb(240, 220, 120);
			break;
			default:
				pixel = drawer.mapRgb(255, 255, 255);
		}
		triangles.emplace_back(mid, h, theta, pixel);
	}
}

}
