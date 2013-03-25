#include "PixelCosm.hpp"
#include "MenuButtons.hpp"

namespace PixelCosm
{

void MenuButton::renderBase(tetra::point<int> offset, bool useIcon)
{
	if (icon_ && useIcon)
		menu_.drawer_.applySurface(pos_ + offset, icon_);

	menu_.drawer_.rect(pos_ + offset, pos_ + offset + (dim_ - 1), menu_.buttonPixel);

	tetra::point<int>
		hl1 = pos_ + offset + tetra::point<int>{dim_.x - 2, 1},
		hl2 = pos_ + offset + (dim_ - 2),
		hl3 = pos_ + offset + tetra::point<int>{1, dim_.y - 2};

	menu_.drawer_.chain(menu_.highlightPixel, hl1, hl2, hl3);

	menu_.drawer_.putPoint(pos_ + offset + tetra::point<int>{1, 1}, menu_.buttonPixel);
	menu_.drawer_.putPoint(pos_ + offset + (dim_ - 1) - tetra::point<int>{1, 1}, menu_.buttonPixel);

	menu_.drawer_.putPoint(pos_ + offset, menu_.gridPixel);
	menu_.drawer_.putPoint(pos_ + offset + tetra::point<int>{1, 0}, menu_.gridPixel);
	menu_.drawer_.putPoint(pos_ + offset + tetra::point<int>{0, 1}, menu_.gridPixel);

	menu_.drawer_.putPoint(pos_ + offset + (dim_ - 1), menu_.gridPixel);
	menu_.drawer_.putPoint(pos_ + offset + (dim_ - 1) - tetra::point<int>{1, 0}, menu_.gridPixel);
	menu_.drawer_.putPoint(pos_ + offset + (dim_ - 1) - tetra::point<int>{0, 1}, menu_.gridPixel);

	if (hovered_)
		menu_.tooltip_ = tooltip_;
}

MenuButton::MenuButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim,
	   SDL_Surface* icon, SDL_Surface* tooltip) :
	   menu_(menu), pos_(pos), dim_(dim), icon_(icon),
	   tooltip_(tooltip), hovered_(false) {}

MenuButton::~MenuButton()
{
	SDL_FreeSurface(icon_);
	SDL_FreeSurface(tooltip_);
}

void MenuButton::render(const tetra::point<int>& offset)
{
	renderBase(offset);
}

void MenuButton::onClick(const tetra::point<int>& p, Uint8 button) {}

int MenuButton::top() const {return pos_.y;}

int MenuButton::bottom() const {return pos_.y + dim_.y;}

int MenuButton::left() const {return pos_.x;}

int MenuButton::right() const {return pos_.x + dim_.x;}

bool MenuButton::inButton(const tetra::point<int>& p) const
{
	return p.isInRect(pos_, pos_ + dim_);
}

void MenuButton::setHovered(bool hovered)
{
	hovered_ = hovered;
}

void ToggleButton::renderToggleBase(const tetra::point<int>& offset, bool useOne)
{
	if (useOne) {
		menu_.drawer_.applySurface(pos_ + offset, icon1_);
		tooltip_ = tooltip1_;
	} else {
		menu_.drawer_.applySurface(pos_ + offset, icon2_);
		tooltip_ = tooltip2_;
	}

	renderBase(offset);
}

ToggleButton::ToggleButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim,
			 SDL_Surface* icon1, SDL_Surface* icon2,
			 SDL_Surface* tooltip1, SDL_Surface* tooltip2) :
	MenuButton(menu, pos, dim), icon1_(icon1), icon2_(icon2),
	tooltip1_(tooltip1), tooltip2_(tooltip2) {}

ToggleButton::~ToggleButton()
{
	SDL_FreeSurface(icon1_);
	SDL_FreeSurface(icon2_);
	SDL_FreeSurface(tooltip1_);
	SDL_FreeSurface(tooltip2_);
	tooltip_ = nullptr;
}

BorderButton::BorderButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	MenuButton(menu, pos, dim,
		SDL_CreateRGBSurface(SDL_SWSURFACE,
			BUTTON_UNIT_SIZE * DISPLAY_FACTOR,
			BUTTON_UNIT_SIZE * DISPLAY_FACTOR,
			32, 0, 0, 0, 0),
		menu.writer_.createLine("Borders")) {}

char BorderButton::quadrantIn(const tetra::point<int>& p)
{
	if (!inButton(p))
		return 0;

	Border udFlag = (p.y - pos_.y) < (pos_.y + dim_.y - p.y) ? Border::Top : Border::Bottom;
	int udDif = (udFlag == Border::Top) ? (p.y - pos_.y) : (pos_.y + dim_.y - p.y);

	Border lrFlag = (p.x - pos_.x) < (pos_.x + dim_.x - p.x) ? Border::Left : Border::Right;
	int lrDif = (lrFlag == Border::Left) ? (p.x - pos_.x) : (pos_.x + dim_.x - p.x);

	return (lrDif < udDif) ? (char)lrFlag : (char)udFlag;
}

void BorderButton::updateIcon()
{
	SDL_FillRect(icon_, nullptr, 0);

	if (flags_ & (char)Border::Left) {
		for (tetra::point<int> p{3, 3}; p.y <= 16; ++p.y)
			tetra::Sdl::putPixel(p, icon_,
				(quad_ == (char)Border::Left) ? menu_.whitePixel : menu_.highlightPixel);
	} else {
		for (tetra::point<int> p{3, 3}; p.y <= 15; p.y += 3) {
			tetra::Sdl::putPixel(p, icon_,
				(quad_ == (char)Border::Left) ? menu_.whitePixel : menu_.highlightPixel);
			++p.y;
			tetra::Sdl::putPixel(p, icon_,
				(quad_ == (char)Border::Left) ? menu_.whitePixel : menu_.highlightPixel);
		}
	}

	if (flags_ & (char)Border::Right) {
		for (tetra::point<int> p{16, 3}; p.y <= 16; ++p.y)
			tetra::Sdl::putPixel(p, icon_,
				(quad_ == (char)Border::Right) ? menu_.whitePixel : menu_.highlightPixel);
	} else {
		for (tetra::point<int> p{16, 3}; p.y <= 15; p.y += 3) {
			tetra::Sdl::putPixel(p, icon_,
				(quad_ == (char)Border::Right) ? menu_.whitePixel : menu_.highlightPixel);
			++p.y;
			tetra::Sdl::putPixel(p, icon_,
				(quad_ == (char)Border::Right) ? menu_.whitePixel : menu_.highlightPixel);
		}
	}

	if (flags_ & (char)Border::Top) {
		for (tetra::point<int> p{3, 3}; p.x <= 16; ++p.x)
			tetra::Sdl::putPixel(p, icon_,
				(quad_ == (char)Border::Top) ? menu_.whitePixel : menu_.highlightPixel);
	} else {
		for (tetra::point<int> p{3, 3}; p.x <= 15; p.x += 3) {
			tetra::Sdl::putPixel(p, icon_,
				(quad_ == (char)Border::Top) ? menu_.whitePixel : menu_.highlightPixel);
			++p.x;
			tetra::Sdl::putPixel(p, icon_,
				(quad_ == (char)Border::Top) ? menu_.whitePixel : menu_.highlightPixel);
		}
	}

	if (flags_ & (char)Border::Bottom) {
		for (tetra::point<int> p{3, 16}; p.x <= 16; ++p.x)
			tetra::Sdl::putPixel(p, icon_,
				(quad_ == (char)Border::Bottom) ? menu_.whitePixel : menu_.highlightPixel);
	} else {
		for (tetra::point<int> p{3, 16}; p.x <= 15; p.x += 3) {
			tetra::Sdl::putPixel(p, icon_,
				(quad_ == (char)Border::Bottom) ? menu_.whitePixel : menu_.highlightPixel);
			++p.x;
			tetra::Sdl::putPixel(p, icon_,
				(quad_ == (char)Border::Bottom) ? menu_.whitePixel : menu_.highlightPixel);
		}
	}

#ifdef DOUBLE_DISPLAY
	tetra::Sdl::doubleSize(icon_, icon_, false);
#endif
}

void BorderButton::render(const tetra::point<int>& offset)
{
	char quad = quadrantIn(menu_.mousePos());
	if (flags_ != menu_.field_.borderFlags || quad_ != quad) {
		flags_ = menu_.field_.borderFlags;
		quad_ = quad;
		updateIcon();
	}
	renderBase(offset);
}

void BorderButton::onClick(const tetra::point<int>& p, Uint8 button)
{
	menu_.field_.borderFlags ^= quadrantIn(p);
}

TitleScreenButton::TitleScreenButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	MenuButton(menu, pos, dim,
		condDouble(menu.app_.loadImage("resources/menu/title_screen.bmp")),
		menu.writer_.createLine("Title Screen")) {}

void TitleScreenButton::render(const tetra::point<int>& offset)
{
	static const tetra::point<int> renderPos = tetra::point<int>{pos_.x + (dim_.x - icon_->w) / 2, pos_.y};
	menu_.drawer_.applySurface(renderPos + offset, icon_);
	renderBase(offset, false);
}

void TitleScreenButton::onClick(const tetra::point<int>& p, Uint8 button)
{
	menu_.app_.returnToTitle();
}

ToolButton::ToolButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	MenuButton(menu, pos, dim),
	icon1_(condDouble(menu.app_.loadImage("resources/menu/repel_tool.bmp"))),
	icon2_(condDouble(menu.app_.loadImage("resources/menu/drag_tool.bmp"))),
	icon3_(condDouble(menu.app_.loadImage("resources/menu/angle_tool.bmp"))),
	icon4_(condDouble(menu.app_.loadImage("resources/menu/inspect_tool.bmp"))),
	tooltip1_(menu.writer_.createLine("Repel Tool")),
	tooltip2_(menu.writer_.createLine("Drag Tool")),
	tooltip3_(menu.writer_.createLine("Angle Tool")),
	tooltip4_(menu.writer_.createLine("Inspect Tool")) {}

void ToolButton::render(const tetra::point<int>& offset)
{
//	static const tetra::point<int> center = pos_ + tetra::point<int>{dim_.x / 2, dim_.y / 2};
//	float radius = menu_.app_.forceStrength * 3.0;

	if (menu_.app_.tool == Tool::Repel) {
		menu_.drawer_.applySurface(pos_ + offset, icon1_);
		tooltip_ = tooltip1_;
	} else if (menu_.app_.tool == Tool::Drag) {
		menu_.drawer_.applySurface(pos_ + offset, icon2_);
		tooltip_ = tooltip2_;
	} else if (menu_.app_.tool == Tool::Angle) {
		menu_.drawer_.applySurface(pos_ + offset, icon3_);
		tooltip_ = tooltip3_;
	} else if (menu_.app_.tool == Tool::Inspect) {
		menu_.drawer_.applySurface(pos_ + offset, icon4_);
		tooltip_ = tooltip4_;
	}

//	if (menu_.app_.tool != Tool::Inspect)
//		menu_.drawer_.circle_in_rect(
//			center + offset,
//			int(radius),
//			pos_ + offset, pos_ + dim_ + offset,
//			menu_.highlightPixel);

	renderBase(offset);
}

void ToolButton::onClick(const tetra::point<int>& p, Uint8 button)
{
	if (button == SDL_BUTTON_LEFT)
		menu_.app_.nextTool();
	else if (button == SDL_BUTTON_RIGHT)
		menu_.app_.prevTool();
}

SaveButton::SaveButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	MenuButton(menu, pos, dim,
		condDouble(menu.app_.loadImage("resources/menu/save.bmp")),
			menu.writer_.createLine("Save")) {}

void SaveButton::onClick(const tetra::point<int>& p, Uint8 button)
{
	menu_.app_.save();
}

OpenButton::OpenButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	MenuButton(menu, pos, dim,
		condDouble(menu.app_.loadImage("resources/menu/load.bmp")),
		menu.writer_.createLine("Load")) {}

void OpenButton::onClick(const tetra::point<int>& p, Uint8 button)
{
	menu_.app_.load();
}

ResetButton::ResetButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	MenuButton(menu, pos, dim,
		condDouble(menu.app_.loadImage("resources/menu/new.bmp")),
		menu.writer_.createLine("Clear")) {}

void ResetButton::onClick(const tetra::point<int>& p, Uint8 button)
{
	if (button == SDL_BUTTON_LEFT)
		menu_.app_.reset();
	else if (button == SDL_BUTTON_RIGHT)
		menu_.app_.resetSecretElements();
}

PauseButton::PauseButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	ToggleButton(menu, pos, dim,
		condDouble(menu.app_.loadImage("resources/menu/play.bmp")),
		condDouble(menu.app_.loadImage("resources/menu/pause.bmp")),
		menu.writer_.createLine("Play"),
		menu.writer_.createLine("Pause")) {}

void PauseButton::render(const tetra::point<int>& offset)
{
	renderToggleBase(offset, menu_.app_.isPaused());
}

void PauseButton::onClick(const tetra::point<int>& p, Uint8 button)
{
	menu_.app_.togglePause();
}

MusicButton::MusicButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	MenuButton(menu, pos, dim,
		condDouble(menu.app_.loadImage("resources/menu/music.bmp")),
		menu.writer_.createLine("Music")) {}

void MusicButton::onClick(const tetra::point<int>& p, Uint8 button)
{
	menu_.activateMusicDropDown(pos_);
}

PauseMusicButton::PauseMusicButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	ToggleButton(menu, pos, dim,
		condDouble(menu.app_.loadImage("resources/menu/play.bmp")),
		condDouble(menu.app_.loadImage("resources/menu/pause.bmp")),
		menu.writer_.createLine("Play"),
		menu.writer_.createLine("Pause")) {}

void PauseMusicButton::render(const tetra::point<int>& offset)
{
	renderToggleBase(offset, !menu_.app_.musicEnabled());
}

void PauseMusicButton::onClick(const tetra::point<int>& p, Uint8 button)
{
	menu_.app_.toggleMusic();
}

SkipSongButton::SkipSongButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	MenuButton(menu, pos, dim,
		condDouble(menu.app_.loadImage("resources/menu/fast_forward.bmp")),
		menu.writer_.createLine("Skip Song")) {}

void SkipSongButton::onClick(const tetra::point<int>& p, Uint8 button)
{
	menu_.app_.skipSong();
}

GravityButton::GravityButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim) :
	ToggleButton(menu, pos, dim,
		condDouble(menu.app_.loadImage("resources/menu/gravity_off.bmp")),
		condDouble(menu.app_.loadImage("resources/menu/gravity_on.bmp")),
		menu.writer_.createLine("Turn Gravity Off"),
		menu.writer_.createLine("Turn Gravity On")) {}

void GravityButton::render(const tetra::point<int>& offset)
{
	renderToggleBase(offset, menu_.field_.gravity);
}

void GravityButton::onClick(const tetra::point<int>& p, Uint8 button)
{
	menu_.app_.toggleGravity();
}

GroupButton::GroupButton(GameMenu& menu, const tetra::point<int>& pos,
									   const tetra::point<int>& dim, const Group& group) :
		MenuButton(menu, pos, dim, nullptr,
				   menu.writer_.createLine(GameMenu::replaceChar(group.name, '_', ' '))),
		group_(group), numFrames_(group.image->h / (dim_.y)) {}

void GroupButton::render(const tetra::point<int>& offset)
{
	SDL_Rect clip = {0, Sint16(frame_ * numFrames_ / 60 * BUTTON_UNIT_SIZE * GROUP_UNITS),
					 Uint16(BUTTON_UNIT_SIZE * GROUP_UNITS), Uint16(BUTTON_UNIT_SIZE * GROUP_UNITS)};

	if (++++frame_ == 60)
		frame_ = 0;

	menu_.drawer_.applySurface(pos_ + offset, group_.image, clip);

	renderBase(offset);
}

void GroupButton::onClick(const tetra::point<int>& p, Uint8 button)
{
	menu_.activateGroup(pos_, group_.id);
}

SubgroupButton::SubgroupButton(GameMenu& menu, const tetra::point<int>& pos,
							const tetra::point<int>& dim, const Subgroup& subgroup) :
	MenuButton(menu, pos, dim, nullptr,
			   menu.writer_.createLine(GameMenu::replaceChar(subgroup.name, '_', ' '))),
	subgroup_(subgroup) {}

void SubgroupButton::render(const tetra::point<int>& offset)
{
	menu_.drawer_.applySurface(pos_ + offset + tetra::point<int>{0, (dim_.y - subgroup_.image->h) / 2},
							   subgroup_.image);

	renderBase(offset);
}

ElementButton::ElementButton(GameMenu& menu, const tetra::point<int>& pos,
					const tetra::point<int>& dim, const Element& elem) :
			MenuButton(menu, pos, dim, nullptr,
					   menu.writer_.createLine(GameMenu::replaceChar(elem.name, '_', ' '))),
			elem(elem), member(Group::withId(elem.groupid).memberWithId(elem.id)) {}

void ElementButton::render(const tetra::point<int>& offset)
{
	static const tetra::point<int> linetop = tetra::point<int>{dim_.x / 2, 0};
	static const tetra::point<int> linebottom = tetra::point<int>{dim_.x / 2, dim_.x - 1};

	static const tetra::point<int> lineleft = tetra::point<int>{0, dim_.y / 2};
	static const tetra::point<int> lineright = tetra::point<int>{dim_.x - 1, dim_.y / 2};

	menu_.drawer_.line(pos_ + linetop + offset,
					   pos_ + linebottom + offset, menu_.gridPixel);

	menu_.drawer_.line(pos_ + lineleft + offset,
					   pos_ + lineright + offset, menu_.gridPixel);

	tetra::point<int> prevPoint{0, 0};
	for (int i = 0; i < dim_.x; ++i) {
		tetra::point<int> endPoint =
			{i, duneHeight(i, member.count * BUTTON_UNIT_SIZE / REQUIRED_PARTICLES)};
		menu_.drawer_.line(pos_ + offset + tetra::point<int>{i, dim_.y - 1},
						   pos_ + offset + endPoint,
						   elem.pixel);
		menu_.drawer_.line(pos_ + offset + prevPoint,
						   pos_ + offset + endPoint, menu_.gridPixel);
		prevPoint = endPoint;
	}

	renderBase(offset);

	if (menu_.app_.curElemId == elem.id) {
		menu_.drawer_.rect(pos_ + offset + 1, pos_ + offset + (dim_ - 2), menu_.highlightPixel);
		menu_.drawer_.rect(pos_ + offset + 2, pos_ + offset + (dim_ - 3), menu_.highlightPixel);
	} else if (member.count < REQUIRED_PARTICLES) {
		menu_.drawer_.rect(pos_ + offset + 1, pos_ + offset + (dim_ - 2), menu_.errorPixel);
	}
}

void ElementButton::onClick(const tetra::point<int>& p, Uint8 button)
{
	if (button == SDL_BUTTON_LEFT && !(member.count < REQUIRED_PARTICLES))
		menu_.app_.curElemId = elem.id;
	else if (button == SDL_BUTTON_MIDDLE)
		menu_.removeQuickElement(elem);
	else if (button == SDL_BUTTON_RIGHT)
	{
		menu_.removeQuickElement(elem);
		menu_.addQuickElement(elem);
//		if (SDL_GetModState() & KMOD_CTRL && member.secret)
//			member.count = 0;
	}
}

CollectionProgressButton::CollectionProgressButton(GameMenu& menu, const tetra::point<int>& pos,
				const tetra::point<int>& dim, const Element& elem) :
		MenuButton(menu, pos, dim,
				   menu.writer_.createLine(GameMenu::replaceChar(elem.name, '_', ' '))),
		elem(elem), member(Group::withId(elem.groupid).memberWithId(elem.id)),
		time(3.0) {}

void CollectionProgressButton::render(const tetra::point<int>& offset)
{
	static const tetra::point<int> labelpos = tetra::point<int>{2, 2};
	int fillx = (dim_.x - 1) * member.count / REQUIRED_PARTICLES;
	menu_.drawer_.filled_rect(pos_ + offset,
							  pos_ + offset + tetra::point<int>{fillx, dim_.y - 1},
							  elem.pixel);
	menu_.drawer_.line(pos_ + offset + tetra::point<int>{fillx, 0},
					   pos_ + offset + tetra::point<int>{fillx, dim_.y - 1},
					   menu_.highlightPixel);
	menu_.drawer_.applySurface(pos_ + labelpos + offset, icon_);
	renderBase(offset, false);

	if (member.count < REQUIRED_PARTICLES)
		menu_.drawer_.rect(pos_ + offset + 1, pos_ + offset + (dim_ - 2), menu_.errorPixel);
	else
		menu_.drawer_.rect(pos_ + offset + 1, pos_ + offset + (dim_ - 2), menu_.highlightPixel);
}

}
