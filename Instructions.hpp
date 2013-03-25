#ifndef PARTICLES_INSTRUCTIONPAGE_HPP
#define PARTICLES_INSTRUCTIONPAGE_HPP

namespace PixelCosm
{

class InstructionFunctions;

class InstructionNav
{
public:
	enum class NavType
	{
		Back,
		Home,
		Forward
	};

private:
	InstructionFunctions& func_;
	SDL_Surface* const image_;
	const tetra::point<int> pos_;
	const tetra::point<int> dim_;

	 NavType type_;

public:
	InstructionNav(InstructionFunctions& func, const tetra::point<int>& pos, NavType type);
	~InstructionNav();

	void render(tetra::Sdl::ScreenDrawer& drawer) const;
	void handleClick(const tetra::point<int>& pos);
};


class InstructionObject
{
	const tetra::point<int> pos_;
	SDL_Surface* const image_;

public:
	InstructionObject(const tetra::point<int>& pos, SDL_Surface* image);
	~InstructionObject();

	int left();
	int right();
	int top();
	int bottom();

	void render(tetra::Sdl::ScreenDrawer& drawer) const;
};

class InstructionPage
{
	InstructionFunctions& func_;
	tetra::Sdl::ScreenDrawer& drawer_;
	std::vector<InstructionObject> objects_;
	InstructionNav back_, home_, forward_;

public:
	InstructionPage(InstructionFunctions& func);

	int left();
	int right();
	int top();
	int bottom();

	void reserve(std::size_t size);
	void add_object(const tetra::point<int>& pos, SDL_Surface* image);

	void render() const;
	void handleClick(const tetra::point<int>& pos);
};

class InstructionFunctions : public tetra::Sdl::FunctionSet
{
	PixelCosmApp* app_;
	std::vector<InstructionPage> pages_;
	std::size_t currentPage_ = 0;

public:
	InstructionFunctions(tetra::Sdl::Application* app, tetra::Sdl::MouseInfo& mouseInfo);

	tetra::Sdl::ScreenDrawer& getDrawer();
	int screenWidth();
	int screenHeight();

	std::size_t currentPage();
	std::size_t pages();

	void renderGUI();
	bool handleEvent(const SDL_Event& e);

	void back_page();
	void forward_page();
	void home();
};


}

#endif // PARTICLES_INSTRUCTIONPAGE_HPP
