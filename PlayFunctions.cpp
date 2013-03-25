#include "PixelCosm.hpp"

namespace PixelCosm
{

PlayFunctions::PlayFunctions(tetra::Sdl::Application* app, tetra::Sdl::MouseInfo& mouseInfo) :
	FunctionSet(app, mouseInfo),
	app_(static_cast<PixelCosmApp*>(app)),
	overlay_(SDL_CreateRGBSurface(SDL_SWSURFACE, ZOOM_RADIUS, ZOOM_RADIUS, 32, 0, 0, 0, 0))
{
	SDL_SetColorKey(overlay_, SDL_SRCCOLORKEY, app_->drawer_.mapRgb(1, 1, 1));
	app_->musicGen_.activateNoise();
}

PlayFunctions::~PlayFunctions()
{
	app_->musicGen_.halt();
}

void PlayFunctions::togglePause()
{
	paused_ ^= true;
}

bool PlayFunctions::isPaused()
{
	return paused_;
}

void PlayFunctions::magnify(const SDL_Rect& rect)
{
	tetra::point<int> center{rect.x + rect.w / 2, rect.y + rect.h / 2};
	for (int x = 0; x < rect.w; ++x) {
		for (int y = 0; y < rect.h; ++y) {
			tetra::Sdl::Pixel p = (center.distance({x + rect.x, y + rect.y}) < rect.w / 2) ?
				tetra::Sdl::getPixel({x + rect.x, y + rect.y}, app_->screen_) : app_->drawer_.mapRgb(1, 1, 1);
			tetra::Sdl::putPixel({x * 2, y * 2}, overlay_, p);
			tetra::Sdl::putPixel({x * 2 + 1, y * 2}, overlay_, p);
			tetra::Sdl::putPixel({x * 2, y * 2 + 1}, overlay_, p);
			tetra::Sdl::putPixel({x * 2 + 1, y * 2 + 1}, overlay_, p);
		}
	}
}

void PlayFunctions::renderGUI()
{
	app_->appendToCaption(std::to_string(int(app_->fps_)));

	if (mouse_.pos.x < DISPLAY_WIDTH && mouse_.pos.y < DISPLAY_HEIGHT) {
		if (app_->tool == Tool::Angle)
			app_->drawer_.line(mouse_.pos,
				mouse_.pos + tetra::point<float>{cosf(app_->penAngle), -sinf(app_->penAngle)} * 20.0,
				app_->menu_.highlightPixel);
		else if (app_->tool == Tool::Inspect) {
			if (mouse_.button[2]) {
				app_->drawer_.line(mouse_.pos - tetra::point<int>{0, COLLECT_RADIUS * DISPLAY_FACTOR},
					mouse_.pos - tetra::point<int>{0, (COLLECT_RADIUS * DISPLAY_FACTOR) / 2},
					app_->menu_.highlightPixel);

				app_->drawer_.line(mouse_.pos + tetra::point<int>{(COLLECT_RADIUS * DISPLAY_FACTOR) / 2, 0},
					mouse_.pos + tetra::point<int>{COLLECT_RADIUS * DISPLAY_FACTOR, 0},
					app_->menu_.highlightPixel);

				app_->drawer_.line(mouse_.pos + tetra::point<int>{0, COLLECT_RADIUS * DISPLAY_FACTOR},
					mouse_.pos + tetra::point<int>{0, (COLLECT_RADIUS * DISPLAY_FACTOR) / 2},
					app_->menu_.highlightPixel);

				app_->drawer_.line(mouse_.pos - tetra::point<int>{(COLLECT_RADIUS * DISPLAY_FACTOR) / 2, 0},
					mouse_.pos - tetra::point<int>{COLLECT_RADIUS * DISPLAY_FACTOR, 0},
					app_->menu_.highlightPixel);
			}

			if (!(mouse_.button[0] || mouse_.button[1] || mouse_.button[2])) {
				magnify(SDL_Rect{Sint16(mouse_.pos.x - overlay_->w / 4), Sint16(mouse_.pos.y - overlay_->h / 4),
								 Uint16(overlay_->w / 2), Uint16(overlay_->h / 2)});

				app_->drawer_.applySurface(mouse_.pos - tetra::point<int>{overlay_->w / 2, overlay_->h / 2}, overlay_);
			}
		}

		if (!mouse_.button[2])
			app_->drawer_.circle(mouse_.pos, app_->penSize * DISPLAY_FACTOR,
							 Element::withId(app_->curElemId).pixel);
	}

	app_->drawer_.rect({0, 0}, {DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1}, app_->menu_.buttonPixel);
#ifdef DOUBLE_DISPLAY
	app_->drawer_.rect({1, 1}, {DISPLAY_WIDTH - 2, DISPLAY_HEIGHT - 2}, app_->menu_.highlightPixel);
#endif // DOUBLE_DISPLAY
	app_->menu_.render();
}

void PlayFunctions::renderPlay()
{
	app_->field_.render(app_->drawer_);
}

void PlayFunctions::stepGame()
{
	if (!paused_) {
		app_->musicGen_.step();
		app_->field_.update();
	} else
		app_->field_.eraseNone();

	static tetra::point<int> lastPos;

	if (lastPos.manhattan(mouse_.pos) > 3) {
		float angle;
		if (mouse_.pos.x != lastPos.x) {
			angle = atan(float(lastPos.y - mouse_.pos.y) /
						 float(mouse_.pos.x - lastPos.x)) +
						 ((mouse_.pos.x < lastPos.x) ? 3.0 * M_PI : 2.0 * M_PI);

			if (angle >= 2.0 * M_PI)
				angle -= 2.0 * M_PI;

			if (tetra::quadrant(angle) == 1 && tetra::quadrant(app_->penAngle) == 4)
				angle += 2.0 * M_PI;
			else if (tetra::quadrant(angle) == 4 && tetra::quadrant(app_->penAngle) == 1)
				angle -= 2.0 * M_PI;
		} else
			angle = (mouse_.pos.y < lastPos.y) ? M_PI_2 : 3.0 * M_PI_2;
		app_->penAngle = (app_->penAngle) * 0.75 + (angle) * 0.25;
		lastPos = mouse_.pos;
	}

	if (app_->penAngle < 0.0)
		app_->penAngle += 2.0 * M_PI;
	else if (app_->penAngle >= 2.0 * M_PI)
		app_->penAngle -= 2.0 * M_PI;

	app_->menu_.step();

	#ifdef DOUBLE_DISPLAY
		tetra::point<int> fieldPos(mouse_.pos / 2);
		tetra::point<int> prevFieldPos(mouse_.prevPos / 2);
	#else
		tetra::point<int> fieldPos(mouse_.pos);
		tetra::point<int> prevFieldPos(mouse_.prevPos);
	#endif

	if (!(mouse_.button[0] || mouse_.button[1] || mouse_.button[2])) {
		wasInPlayArea_ = false;
		if (app_->field_.dragger_.isOn())
			app_->field_.dragger_.deactivate();
		return;
	}

	if (wasInPlayArea_) {
		if (fieldPos.x >= WIDTH)
			fieldPos.x = WIDTH - 1;
		if (fieldPos.y >= HEIGHT)
			fieldPos.y = HEIGHT - 1;
		if (prevFieldPos.x >= WIDTH)
			prevFieldPos.x = WIDTH - 1;
		if (prevFieldPos.y >= HEIGHT)
			prevFieldPos.y = HEIGHT - 1;
	} else if ((fieldPos.x >= WIDTH || fieldPos.y >= HEIGHT) &&
				(prevFieldPos.x >= WIDTH || prevFieldPos.y >= HEIGHT))
		return;
	else
		wasInPlayArea_ = true;

	if (mouse_.button[1] || (mouse_.button[0] && SDL_GetModState() & KMOD_SHIFT)) {
		app_->field_.eraser.thick_line(prevFieldPos,
			fieldPos, app_->penSize, Element::withId(0));
	}

	if (mouse_.button[0] && !(SDL_GetModState() & KMOD_SHIFT)) {
		app_->field_.drawer.thick_line(prevFieldPos,
			fieldPos, app_->penSize, Element::withId(app_->curElemId));
	}

	if (mouse_.button[2]) {
		if (app_->field_.dragger_.isOn())
			app_->field_.dragger_.update(fieldPos);

		if (app_->tool == Tool::Repel)
			app_->field_.forceDrawer.changeSetter(&fieldRepel);
		else if (app_->tool == Tool::Drag) {
//			app_->field_.forceDrawer.changeSetter(&fieldAttract);
			if (!app_->field_.dragger_.isOn())
				app_->field_.dragger_.activate(fieldPos, app_->penSize);
		} else if (app_->tool == Tool::Angle) {
			app_->field_.forceDrawer.changeAngle(app_->penAngle);
			app_->field_.forceDrawer.changeSetter(&fieldAngle);
		} else if (app_->tool == Tool::Inspect)
			app_->field_.collect(fieldPos, COLLECT_RADIUS, app_->menu_);

		if (!(app_->tool == Tool::Drag ||
			  app_->tool == Tool::Inspect))
		{
			app_->field_.forceDrawer.line(prevFieldPos,
				fieldPos, app_->forceStrength);
			app_->musicGen_.renewNoise();
		}
	} else if (app_->field_.dragger_.isOn())
		app_->field_.dragger_.deactivate();
}

bool PlayFunctions::handleEvent(const SDL_Event& e)
{
	if (defaultHandleEvent(e))
		return true;
	if (e.type == SDL_MOUSEBUTTONDOWN) {
		void* beforeDropDownId = app_->menu_.dropDownId();
		if (mouse_.pos.x > DISPLAY_WIDTH || mouse_.pos.y > DISPLAY_HEIGHT)
			app_->menu_.handleClick(mouse_.pos,
				(e.button.button == SDL_BUTTON_LEFT && (SDL_GetModState() & KMOD_SHIFT)) ?
					SDL_BUTTON_MIDDLE : e.button.button);

		void* afterDropDownId = app_->menu_.dropDownId();

		if (app_->menu_.inDropDown(mouse_.pos)) {
			if (app_->menu_.tryDropDown(mouse_.pos, e.button.button))
				mouse_.button[0] = false;
			else if (beforeDropDownId == afterDropDownId)
				app_->menu_.clearDropDown();
		} else if (mouse_.pos.x < DISPLAY_WIDTH && mouse_.pos.y < DISPLAY_HEIGHT)
			app_->menu_.clearDropDown();

		if (e.button.button == SDL_BUTTON_WHEELUP) {
			if (SDL_GetModState() & KMOD_CTRL) {
				if (app_->forceStrength < app_->maxForceStrength)
					app_->forceStrength += app_->forceStep;
			} else {
				if (app_->penSize < app_->maxPenSize)
					++app_->penSize;
			}
		} else if (e.button.button == SDL_BUTTON_WHEELDOWN) {
			if (SDL_GetModState() & KMOD_CTRL) {
				if (app_->forceStrength > app_->forceStep)
					app_->forceStrength -= app_->forceStep;
			} else {
				if (app_->penSize > 0)
					--app_->penSize;
			}
		}
	} else if (e.type == SDL_KEYDOWN) {
		if (e.key.keysym.sym == SDLK_UP) {
			if (SDL_GetModState() & KMOD_CTRL) {
				if (app_->forceStrength < app_->maxForceStrength)
					app_->forceStrength += app_->forceStep;
			} else {
				if (app_->penSize < app_->maxPenSize)
					++app_->penSize;
			}
		} else if (e.key.keysym.sym == SDLK_DOWN) {
			if (SDL_GetModState() & KMOD_CTRL) {
				if (app_->forceStrength > app_->forceStep)
					app_->forceStrength -= app_->forceStep;
			} else {
				if (app_->penSize > 0)
					--app_->penSize;
			}
		} else if (e.key.keysym.sym == SDLK_SPACE) {
			togglePause();
			app_->penAngle = 3.0 * M_PI_2;
		}
	}
	return false;
}

}
