#ifndef PARTICLES_TITLESCREEN_HPP
#define PARTICLES_TITLESCREEN_HPP

namespace PixelCosm
{

class TitleFunctions;

template <typename T>
class DisplayTriangle
{
	tetra::point<T> mid_;
	T h_, theta_;
	tetra::Sdl::Pixel pixel_;

public:
	DisplayTriangle() : mid_(0, 0), h_(0), theta_(0), pixel_(0) {}
	DisplayTriangle(const tetra::point<T> mid, T h, T theta, tetra::Sdl::Pixel pixel) :
		mid_(mid), h_(h), theta_(theta), pixel_(pixel) {}

	tetra::point<T>& mid() {return mid_;}
	const tetra::point<T>& mid() const {return mid_;}

	T& angle() {return theta_;}
	const T& angle() const {return theta_;}

	T& height() {return h_;}

	tetra::Sdl::Pixel& pixel() {return pixel_;}

	void rotate(float m)
	{
		theta_ += M_PI * m;
	}

	tetra::point<T> a() const
	{
		return mid_ + tetra::point<T>{h_ * cosf(theta_ + M_PI_2), -h_ * sinf(theta_ + M_PI_2)};
	}

	tetra::point<T> b() const
	{
		return mid_ + tetra::point<T>{h_ * cosf(theta_ + 7.0 * M_PI / 6.0), -h_ * sinf(theta_ + 7.0 * M_PI / 6.0)};
	}

	tetra::point<T> c() const
	{
		return mid_ + tetra::point<T>{h_ * cosf(theta_ - M_PI / 6.0), -h_ * sinf(theta_ - M_PI / 6.0)};
	}

	void render(tetra::Sdl::ScreenDrawer& drawer) const
	{
		drawer.polygon(pixel_, a(), b(), c());
	}
};

class TitleButton
{
	const tetra::point<int> pos_;
	SDL_Surface* image_;

public:
	std::function<void ()> select;

	TitleButton(TitleFunctions* instance, const tetra::point<int>& pos, tetra::Sdl::Writer& writer,
				const std::string& text, void (TitleFunctions::*selectFunction)());
	~TitleButton();

	void render(tetra::Sdl::ScreenDrawer& drawer);

	int top();
	int bottom();
	int left();
	int right();

	int width();
	int height();

	tetra::point<int> selectPos();
};

class TitleButtonSet
{
	TitleFunctions& instance_;
	const tetra::point<int> pos_;
	tetra::Sdl::Writer writer_;
	std::vector<TitleButton> buttons_;
	std::size_t selected_ = 0;
	DisplayTriangle<float> selectTri_;

public:
	TitleButtonSet(TitleFunctions& instance, const tetra::point<int>& pos);

	void reserve(std::size_t n);
	void addButton(const std::string& text, void (TitleFunctions::*selectFunction)());
	void closeFont();

	int maxWidth();

	bool checkMouse(const tetra::point<int>& pos);
	void stepForward();
	void stepBack();

	void selectButton();

	void render(tetra::Sdl::ScreenDrawer& drawer);
};

class TitleFunctions : public tetra::Sdl::FunctionSet
{
	PixelCosmApp* app_;
	SDL_Surface *title_, *logo_;

	TitleButtonSet buttons_;
	std::vector<DisplayTriangle<float>> tris_;

	SDL_Surface* transSurface_ = nullptr;
	bool transing_ = false;
	float transTime_ = 1.0;

public:
	TitleFunctions(tetra::Sdl::Application* app, tetra::Sdl::MouseInfo& mouseInfo);
	~TitleFunctions();

	void renderGUI();
	void stepGame();
	bool handleEvent(const SDL_Event& e);

	tetra::Sdl::ScreenDrawer& getDrawer();

	void singlePlayer();
	void multiPlayer();
	void instructions();
	void options();
	void update();
	void quit();
};

void createTriangles(std::vector<DisplayTriangle<float>>& triangles, std::size_t N, tetra::Sdl::ScreenDrawer& drawer);

}

#endif // PARTICLES_TITLESCREEN_HPP

