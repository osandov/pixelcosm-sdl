#ifndef PARTICLES_GAMEPLAYFUNCTIONS_HPP
#define PARTICLES_GAMEPLAYFUNCTIONS_HPP

namespace PixelCosm
{

class PlayFunctions : public tetra::Sdl::FunctionSet
{
	PixelCosmApp* app_;
	SDL_Surface* overlay_;
	bool wasInPlayArea_ = false;
	bool paused_ = false;

public:
	PlayFunctions(tetra::Sdl::Application* app, tetra::Sdl::MouseInfo& mouseInfo);
	~PlayFunctions();

	void togglePause();
	bool isPaused();

	void magnify(const SDL_Rect& rect);

	void renderGUI();
	void renderPlay();
	void stepGame();
	bool handleEvent(const SDL_Event& e);
};

}

#endif // PARTICLES_GAMEPLAYFUNCTIONS_HPP
