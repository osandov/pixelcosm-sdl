#ifndef PARTICLES_PARTICLE_HPP
#define PARTICLES_PARTICLE_HPP

#include "Element.hpp"
#include "InterDir.hpp"
#include "Dependencies.hpp"

namespace PixelCosm
{

template <int W, int H>
class GameField;

class ParticleParams
{
public:
	int elemid;

	class Physical
	{
	public:
		tetra::point<float> pos, veloc;
		float life;
	} phys;
};

class Particle
{
	static float genInit(float initforce)
	{
		using Dist = std::uniform_real_distribution<float>;
		static Dist urd{};
		return urd(random_engine, Dist::param_type{-initforce, initforce});
	}

	static float genLife(const std::pair<float, float>& range)
	{
		using Dist = std::uniform_real_distribution<float>;
		static Dist urd{};
		return urd(random_engine, Dist::param_type{range.first, range.second});
	}

	static float genNextTarget()
	{
		using Dist = std::uniform_real_distribution<float>;
		static Dist urd{0.0, 3.0};
		return urd(random_engine);
	}

	static bool doRotate(float entropy)
	{
		using Dist = std::bernoulli_distribution;
		static Dist bd{};
		return bd(random_engine, Dist::param_type{entropy});
	}

	static float genAngle(float magnitude)
	{
		using Dist = std::uniform_real_distribution<float>;
		static Dist urd{};
		return urd(random_engine,
			Dist::param_type{-float(M_PI_2) * magnitude, float(M_PI_2) * magnitude});
	}

	static float genHorizontal(float magnitude)
	{
		using Dist = std::uniform_real_distribution<float>;
		static Dist urd{};
		return urd(random_engine,
			Dist::param_type{-magnitude, magnitude});
	}

	static InterDirGenerator dirGen_;

public:
	tetra::point<float> pos;
	PointForce veloc;

	const Element* elem;
	float life;

	const Particle* target;
	float nexttarget;

private:
	GameField<WIDTH, HEIGHT>& field_;

	static const Element* none;
	static constexpr float
        decayAmount = 1.f / FRAMERATECAP,
        gravForce = 1.f / FRAMERATECAP,
        riseForce = 0.25f / FRAMERATECAP;

public:

	Particle(const tetra::point<float>& pos, const Element& elem,
			 const tetra::point<float>& veloc, float life,
			 GameField<WIDTH, HEIGHT>& field);

	Particle(const tetra::point<float>& pos, const Element& elem,
			 GameField<WIDTH, HEIGHT>& field);

	~Particle();

	static void setNoneElement(const Element* p);

	void render(tetra::Sdl::ScreenDrawer& drawer) const
	{
#ifdef DOUBLE_DISPLAY
		tetra::point<int> pos1(pos * 2.0), pos2(pos.convert<int>() * 2);
		tetra::point<int> renderPos1(std::min(pos1.x, pos2.x), std::min(pos1.y, pos2.y));
		tetra::point<int> renderPos2(std::max(pos1.x, pos2.x) + 1, std::max(pos1.y, pos2.y) + 1);
		drawer.filled_rect(renderPos1, renderPos2, elem->pixel);
#else
		drawer.putPoint(pos, elem->pixel);
#endif
	}

	bool update();

	void swap(Particle& other);

	void changeElement(const Element& element)
	{
		if (elem != &element) {
			elem = &element;
			if (elem->initforce > 0.0)
				veloc = {genInit(elem->initforce), genInit(elem->initforce)};
			if (elem->decays)
				life = genLife(elem->lifespan);
		}
	}

	void repel(float force);
	void attract(float force);
	void angle(float angle, float force);
	void cancel_force();

	static bool gravityComp(const Particle& a, const Particle& b)
	{
		return a.pos.y > b.pos.y;
	}

	static bool noGravityComp(const Particle& a, const Particle& b)
	{
		return a.pos.y > b.pos.y;
	}
};

}

#endif /* PARTICLES_PARTICLE_HPP */

