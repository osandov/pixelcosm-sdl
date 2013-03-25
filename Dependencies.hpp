#ifndef PARTICLES_DEPENDENCIES_HPP
#define PARTICLES_DEPENDENCIES_HPP

#include <vector>
#include <array>
#include <deque>
#include <list>
#include <map>
#include <random>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <cassert>

#ifdef __unix__
#include <unistd.h>
#endif // __unix__

#include "tetra/algorithm_utils.hpp"
#include "tetra/point.hpp"

#define INCLUDE_WRITER
#include "tetra/Sdl/Application.hpp"

#include <SDL/SDL_mixer.h>

#ifndef QVGA
#define DOUBLE_DISPLAY
#endif // QVGA

// For debugging
//#define RENDER_FORCES
//#define NO_RENDER_PARTICLES
//#define NO_PARTICLE_UPDATE
//#define NO_FORCE_FIELD_UPDATE
//#define NO_INTERACTIONS

namespace PixelCosm
{

constexpr const char* GAME_NAME = "PixelCosm";
constexpr const char* VERSION = "Alpha 1.0";

#ifdef QVGA
constexpr int WIDTH = 280, HEIGHT = 220;
#else
constexpr int WIDTH = 400, HEIGHT = 256;
#endif // QVGA

#ifdef DOUBLE_DISPLAY
constexpr int DISPLAY_FACTOR = 2;
#else
constexpr int DISPLAY_FACTOR = 1;
#endif // DISPLAY_FACTOR

constexpr int DISPLAY_WIDTH = WIDTH * DISPLAY_FACTOR, DISPLAY_HEIGHT = HEIGHT * DISPLAY_FACTOR;

constexpr float FRAMERATECAP = 30.0;
constexpr int FONT_SIZE = 10;
constexpr int ERROR_FONT_SIZE = 12;

constexpr int noneMass = 10;

constexpr std::size_t PARTICLECAP = 50000;

constexpr unsigned char REQUIRED_PARTICLES = 255;
constexpr unsigned char MAX_ELEM_NAME = 255;
constexpr const char* SAVE_EXTENSION = ".sed";


constexpr int COLLECT_RADIUS = 5;
constexpr int ZOOM_RADIUS = COLLECT_RADIUS * 10;

extern std::random_device random_device;

extern std::default_random_engine random_engine;

class Direction {
public:
	static constexpr tetra::point<int> up{0, -1};
	static constexpr tetra::point<int> left{-1, 0};
	static constexpr tetra::point<int> down{0, 1};
	static constexpr tetra::point<int> right{1, 0};

private:
	static constexpr const tetra::point<int>* dirs_[4] = {&up, &left, &down, &right};

public:
	static constexpr tetra::point<int> dir(int which) {
		return *dirs_[which];
	}
};

typedef tetra::point<float> PointForce;

/** Double size if DOUBLE_DISPLAY is defined, otherwise, do nothing */
inline SDL_Surface* condDouble(SDL_Surface* surface)
{
#ifdef DOUBLE_DISPLAY
	return tetra::Sdl::doubleSize(surface, true);
#else
	return surface;
#endif
}

}

#endif /* PARTICLES_DEPENDENCIES_HPP */
