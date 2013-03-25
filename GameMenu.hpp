#ifndef PARTICLEMENU_HPP
#define PARTICLEMENU_HPP

#include "MenuButtons.hpp"

namespace PixelCosm
{

class PixelCosmApp;

constexpr int BUTTON_UNIT_SIZE = 20;

#ifdef DOUBLE_DISPLAY
constexpr int GROUP_UNITS = 3;
constexpr int QUICK_UNITS = 2;
#else
constexpr int GROUP_UNITS = 2;
constexpr int QUICK_UNITS = 1;
#endif /* DOUBLE_DISPLAY */
constexpr int GUI_WIDTH = BUTTON_UNIT_SIZE * GROUP_UNITS;
constexpr int GUI_HEIGHT = BUTTON_UNIT_SIZE * DISPLAY_FACTOR;

class GameMenu
{
	static std::string replaceChar(const std::string& str, char old_char, char new_char)
	{
		std::string r(str);
		std::replace(r.begin(), r.end(), old_char, new_char);
		return r;
	}

	class DropDown
	{
	public:
		tetra::point<int> offset;
		std::vector<std::shared_ptr<MenuButton>> buttons;
		tetra::point<int> deltaOffset, targetOffset;

		void render()
		{
			for (auto& button : buttons)
				button->render(offset);
		}

		bool inMenu(const tetra::point<int>& p) const
		{
			return p.isInRect(offset, offset + tetra::point<int>{right(), bottom()});
		}

		void step()
		{
			if (deltaOffset.x < 0) {
				if (offset.x > targetOffset.x)
					offset.x += deltaOffset.x;
				else
					offset.x = targetOffset.x;
			} else if (deltaOffset.x > 0) {
				if (offset.x < targetOffset.x)
					offset.x += deltaOffset.x;
				else
					offset.x = targetOffset.x;
			}

			if (deltaOffset.y < 0) {
				if (offset.y > targetOffset.y)
					offset.y += deltaOffset.y;
				else
					offset.y = targetOffset.y;
			} else if (deltaOffset.y > 0) {
				if (offset.y < targetOffset.y)
					offset.y += deltaOffset.y;
				else
					offset.y = targetOffset.y;
			}
		}

		bool doneMoving() const
		{
			return offset == targetOffset;
		}

		int top() const {return (buttons.empty()) ? 0 : buttons.front()->top();}
		int bottom() const {return (buttons.empty()) ? 0 : buttons.back()->bottom();}
		int left() const {return (buttons.empty()) ? 0 : buttons.front()->left();}
		int right() const {return (buttons.empty()) ? 0 : buttons.back()->right();}
	};

	PixelCosmApp& app_;
	GameField<WIDTH, HEIGHT>& field_;
	tetra::Sdl::ScreenDrawer& drawer_;
	tetra::Sdl::Writer& writer_;

	SDL_Surface* tooltip_ = nullptr;

	std::vector<DropDown> groupDropDowns_;

	DropDown musicDropDown_;

	DropDown collectionProgress_;

	DropDown* dropDown_ = nullptr;

	std::vector<std::shared_ptr<MenuButton>> quickBar_;

	std::size_t firstQuickElemIndex_;

	static constexpr std::size_t maxQuickElems_ = 8;

	std::vector<std::shared_ptr<GroupButton>> groupsBar_;

	friend class MenuButton;
	friend class Spacer;
	friend class ToggleButton;
	friend class TitleScreenButton;
	friend class ToolButton;
	friend class BorderButton;
	friend class SaveButton;
	friend class OpenButton;
	friend class ResetButton;
	friend class PauseButton;
	friend class MusicButton;
	friend class PauseMusicButton;
	friend class SkipSongButton;
	friend class GravityButton;
	friend class GroupButton;
	friend class SubgroupButton;
	friend class ElementButton;
	friend class CollectionProgressButton;

public:
	tetra::Sdl::Pixel gridPixel, buttonPixel, highlightPixel, whitePixel, errorPixel;

	GameMenu(PixelCosmApp& app);

	const tetra::point<int>& mousePos();

	void setupMenu();
	void setupGroups();
	void setupGroupDropDown(const Group& group);

	void render();

	void step();

	void handleClick(const tetra::point<int>& pos, Uint8 which);

	void* dropDownId();
	bool inDropDown(const tetra::point<int>& pos);
	bool tryDropDown(const tetra::point<int>& pos, Uint8 which);
	void clearDropDown();

	void activateGroup(const tetra::point<int>& pos, int id);

	void activateMusicDropDown(const tetra::point<int>& pos);

	void addQuickElement(const Element& elem);
	void removeQuickElement(const Element& elem);
	std::pair<std::vector<std::shared_ptr<MenuButton>>::iterator,
			  std::vector<std::shared_ptr<MenuButton>>::iterator>
	getQuickElems();

	void addCollectionProgress(const Element& elem);
};

}

#endif /* PARTICLEMENU_HPP */
