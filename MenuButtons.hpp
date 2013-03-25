#ifndef MENUBUTTONS_HPP
#define MENUBUTTONS_HPP

namespace PixelCosm
{

class GameMenu;

class MenuButton
{
protected:
	GameMenu& menu_;
	const tetra::point<int> pos_;
	const tetra::point<int> dim_;
	SDL_Surface* icon_;
	SDL_Surface* tooltip_;
	bool hovered_;

	void renderBase(tetra::point<int> offset, bool useIcon = true);

public:
	MenuButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim,
		   SDL_Surface* icon = nullptr, SDL_Surface* tooltip = nullptr);

	virtual ~MenuButton();

	virtual void render(const tetra::point<int>& offset);

	virtual void onClick(const tetra::point<int>& p, Uint8 button);

	int top() const;
	int bottom() const;
	int left() const;
	int right() const;

	bool inButton(const tetra::point<int>& p) const;

	void setHovered(bool hovered);
};

class Spacer : public MenuButton
{
public:
	Spacer(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim) :
		MenuButton(menu, pos, dim) {}
	void render(const tetra::point<int>& offset) {}
	void onClick(const tetra::point<int>& p, Uint8 button) {}
};

class ToggleButton : public MenuButton
{
protected:
	SDL_Surface *icon1_, *icon2_;
	SDL_Surface *tooltip1_, *tooltip2_;

	void renderToggleBase(const tetra::point<int>& offset, bool useOne);

public:
	ToggleButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim,
				 SDL_Surface* icon1, SDL_Surface* icon2,
				 SDL_Surface* tooltip1 = nullptr, SDL_Surface* tooltip2 = nullptr);

	virtual ~ToggleButton();
};

class TitleScreenButton : public MenuButton
{
public:
	TitleScreenButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim);
	void render(const tetra::point<int>& offset);
	void onClick(const tetra::point<int>& p, Uint8 button);
};

class ToolButton : public MenuButton
{
	SDL_Surface *icon1_, *icon2_, *icon3_, *icon4_;
	SDL_Surface *tooltip1_, *tooltip2_, *tooltip3_, *tooltip4_;

public:
	ToolButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim);
	void render(const tetra::point<int>& offset);
	void onClick(const tetra::point<int>& p, Uint8 button);

	virtual ~ToolButton()
	{
		SDL_FreeSurface(icon1_);
		SDL_FreeSurface(icon2_);
		SDL_FreeSurface(icon3_);
		SDL_FreeSurface(icon4_);
		SDL_FreeSurface(tooltip1_);
		SDL_FreeSurface(tooltip2_);
		SDL_FreeSurface(tooltip3_);
		SDL_FreeSurface(tooltip4_);
		tooltip_ = nullptr;
	}
};

class BorderButton : public MenuButton
{
	char flags_ = -1, quad_ = 0;

	char quadrantIn(const tetra::point<int>& p);
	void updateIcon();

public:
	BorderButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim);
	void render(const tetra::point<int>& offset);
	void onClick(const tetra::point<int>& p, Uint8 button);
};

class SaveButton : public MenuButton
{
public:
	SaveButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim);
	void onClick(const tetra::point<int>& p, Uint8 button);
};

class OpenButton : public MenuButton
{
public:
	OpenButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim);
	void onClick(const tetra::point<int>& p, Uint8 button);
};

class ResetButton : public MenuButton
{
public:
	ResetButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim);
	void onClick(const tetra::point<int>& p, Uint8 button);
};

class PauseButton : public ToggleButton
{
public:
	PauseButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim);
	void render(const tetra::point<int>& offset);
	void onClick(const tetra::point<int>& p, Uint8 button);
};

class MusicButton : public MenuButton
{
public:
	MusicButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim);
	void onClick(const tetra::point<int>& p, Uint8 button);
};

class PauseMusicButton : public ToggleButton
{
public:
	PauseMusicButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim);
	void render(const tetra::point<int>& offset);
	void onClick(const tetra::point<int>& p, Uint8 button);
};

class SkipSongButton : public MenuButton
{
public:
	SkipSongButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim);
	void onClick(const tetra::point<int>& p, Uint8 button);
};

class GravityButton : public ToggleButton
{
public:
	GravityButton(GameMenu& menu, const tetra::point<int>& pos, const tetra::point<int>& dim);
	void render(const tetra::point<int>& offset);
	void onClick(const tetra::point<int>& p, Uint8 button);
};

class GroupButton : public MenuButton
{
	const Group& group_;
	int frame_ = 0;
	int numFrames_;

public:
	GroupButton(GameMenu& menu, const tetra::point<int>& pos,
				const tetra::point<int>& dim, const Group& group);

	void render(const tetra::point<int>& offset);
	void onClick(const tetra::point<int>& p, Uint8 button);
	const Group& getGroup() {return group_;}
};

class SubgroupButton : public MenuButton
{
	const Subgroup& subgroup_;

public:
	SubgroupButton(GameMenu& menu, const tetra::point<int>& pos,
				   const tetra::point<int>& dim, const Subgroup& subgroup);

	void render(const tetra::point<int>& offset);
};

class ElementButton : public MenuButton
{
	int duneHeight(int x, int h)
	{
		return dim_.y - 1 - int(float(h) * exp(-tetra::square<float>(x - dim_.x / 2) / (5.0 * dim_.x)));
	}

	const Element& elem;
	const Group::Member& member;

public:
	ElementButton(GameMenu& menu, const tetra::point<int>& pos,
				const tetra::point<int>& dim, const Element& elem);

	void render(const tetra::point<int>& offset);
	void onClick(const tetra::point<int>& p, Uint8 button);

	const Element& getElem() {return elem;}

	bool matches(const Element& other) {return elem == other;}
};

class CollectionProgressButton : public MenuButton
{
	const Element& elem;
	const Group::Member& member;

public:
	float time;

	CollectionProgressButton(GameMenu& menu, const tetra::point<int>& pos,
				const tetra::point<int>& dim, const Element& elem);

	void render(const tetra::point<int>& offset);
	void onClick(const tetra::point<int>& p, Uint8 button) {}

	bool matches(const Element& other) {return elem == other;}
};
}

#endif /* MENUBUTTONS_HPP */
