#ifndef PARTICLES_UPDATE_HPP
#define PARTICLES_UPDATE_HPP

namespace PixelCosm
{

class Updater
{

public:
	void update();
	void cancel();
};

class UpdateButton
{
public:
	enum class Type
	{
		Start,
		Cancel
	};

private:
	const tetra::point<int> pos_, dim_;
	Type type_;
	SDL_Surface* image_;

public:
	UpdateButton(const tetra::point<int>& pos, const tetra::point<int>& dim, Type type);

	void render(tetra::Sdl::ScreenDrawer& drawer);
	bool inButton(const tetra::point<int>& pos);
};

class UpdateFunctions : public tetra::Sdl::FunctionSet
{
	PixelCosmApp* app_;
	UpdateButton start_, cancel_;
	tetra::Sdl::Writer writer_;

	SDL_Surface* message_;

	Updater updater_;

public:
	UpdateFunctions(tetra::Sdl::Application* app, tetra::Sdl::MouseInfo& mouseInfo);
	~UpdateFunctions();

	void renderGUI();
	void stepGame();
	bool handleEvent(const SDL_Event& e);

	void start();
	void cancel();
};

}

#endif // PARTICLES_UPDATE_HPP
