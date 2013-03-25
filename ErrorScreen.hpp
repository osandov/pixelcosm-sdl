#ifndef PARTICLES_ERRORSCREEN_HPP
#define PARTICLES_ERRORSCREEN_HPP

namespace PixelCosm
{

class ErrorScreenFunctions : public tetra::Sdl::FunctionSet
{
	PixelCosmApp* app_;
	SDL_Surface* message_;
	int scrollY_ = 0;
	int scrollAmount_;

public:
	ErrorScreenFunctions(tetra::Sdl::Application* app, tetra::Sdl::MouseInfo& mouseInfo);
	~ErrorScreenFunctions();

	void renderGUI();
	bool handleEvent(const SDL_Event& e);
};

}


#endif // PARTICLES_ERRORSCREEN_HPP

