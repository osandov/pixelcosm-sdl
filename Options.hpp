#ifndef PARTICLES_OPTIONS_HPP
#define PARTICLES_OPTIONS_HPP

namespace PixelCosm
{

class OptionFunctions;

class Option
{
protected:
	OptionFunctions& func_;
	tetra::Sdl::ScreenDrawer& drawer_;
	const tetra::point<int> pos_, dim_;

	void renderButton();

public:
	Option(OptionFunctions& func, const tetra::point<int>& pos, const tetra::point<int>& dim);

	bool inOption(const tetra::point<int>& pos)
	{
		return (pos.x >= pos_.x) && (pos.x < pos_.x + dim_.x) &&
			   (pos.y >= pos_.y) && (pos.y < pos_.y + dim_.y);
	}

	virtual void render()
	{
		renderButton();
	}

	virtual void handle(const SDL_Event& e) {}
};

class TitleScreenOption : public Option
{
	SDL_Surface* icon_;

public:
	TitleScreenOption(OptionFunctions& func, const tetra::point<int>& pos, const tetra::point<int>& dim);
	~TitleScreenOption();

	void render();
	void handle(const SDL_Event& e);
};

class VolumeOption : public Option
{
	SDL_Surface* label_;
	bool wasClicked_ = false;

	int minPos, maxPos;

public:
	VolumeOption(OptionFunctions& func, const tetra::point<int>& pos, const tetra::point<int>& dim);
	~VolumeOption();

	void render();
	void handle(const SDL_Event& e);
};

class FullscreenOption : public Option
{
	SDL_Surface *label_, *windowed_, *fullscreen_;

public:
	FullscreenOption(OptionFunctions& func, const tetra::point<int>& pos, const tetra::point<int>& dim);
	~FullscreenOption();

	void render();
	void handle(const SDL_Event& e);
};

class OptionFunctions : public tetra::Sdl::FunctionSet
{
	PixelCosmApp* app_;
	std::vector<DisplayTriangle<float>> tris_;
	std::vector<std::shared_ptr<Option>> options_;
	bool returnToTitle_ = false;
	tetra::Sdl::Writer writer_;

public:
	OptionFunctions(tetra::Sdl::Application* app, tetra::Sdl::MouseInfo& mouseInfo);
	~OptionFunctions();

	void renderGUI();
	bool handleEvent(const SDL_Event& e);
	void stepGame();

	tetra::Sdl::ScreenDrawer& getDrawer();
	tetra::Sdl::Writer& getWriter();
	void setVolume(int volume);
	int getVolume();

	bool getFullscreen() const;
	void toggleFullscreen();

	void title();
};

}


#endif // PARTICLES_OPTIONS_HPP
