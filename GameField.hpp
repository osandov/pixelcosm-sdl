#ifndef PARTICLES_GAMEFIELD_H
#define PARTICLES_GAMEFIELD_H

#include "Particle.hpp"
#include "Group.hpp"
#include "Element.hpp"
#include "GameMenu.hpp"
#include "Dependencies.hpp"

namespace PixelCosm
{

template <int W, int H>
class GameField;

enum class Border : char
{
	Top = 0x1,
	Left = 0x2,
	Bottom = 0x4,
	Right = 0x8
};

const Particle& getParticle(const tetra::point<int>& pos, GameField<WIDTH, HEIGHT>& particles);
void setParticle(const tetra::point<float>& pos, GameField<WIDTH, HEIGHT>& particles, const Element& elem);
void eraseParticle(const tetra::point<float>& pos, GameField<WIDTH, HEIGHT>& particles, const Element& elem);

class ParticleDrawer : public tetra::Drawer<decltype(setParticle),
											decltype(getParticle),
											Element, Particle>
{
	GameField<WIDTH, HEIGHT>& field_;

	/** The specialized set function */
	void set(const tetra::point<int>& p, const Element& elem)
	{
		setter_(p, field_, elem);
	}

	/** The specialized get function */
	const Particle& get(const tetra::point<int>& p)
	{
		return getter_(p, field_);
	}

public:
	ParticleDrawer(GameField<WIDTH, HEIGHT>& particles) :
		Drawer(&setParticle, &getParticle), field_(particles) {}
};

const PointForce& getForce(const tetra::point<int>& pos, GameField<WIDTH, HEIGHT>& field);
void fieldRepel(const tetra::point<float>& pos, GameField<WIDTH, HEIGHT>& field, float force, float);
void fieldAttract(const tetra::point<float>& pos, GameField<WIDTH, HEIGHT>& field, float force, float);
void fieldAngle(const tetra::point<float>& pos, GameField<WIDTH, HEIGHT>& field, float force, float angle);

class ForceDrawer : public tetra::Drawer<decltype(fieldRepel),
										 decltype(getForce),
										 float, PointForce>
{
	GameField<WIDTH, HEIGHT>& field_;

	/** The specialized set function */
	void set(const tetra::point<int>& p, const float& force)
	{
		setter_(p, field_, force, angle);
	}

	/** The specialized get function */
	const PointForce& get(const tetra::point<int>& p)
	{
		return getter_(p, field_);
	}

public:
	float angle;

	ForceDrawer(GameField<WIDTH, HEIGHT>& field) :
		Drawer(&fieldRepel, &getForce), field_(field),
		angle(M_PI_2) {}

	void changeAngle(float setAngle) {angle = setAngle;}
	void changeSetter(decltype(setter_) setter) {setter_ = setter;}
};

class ParticleEraser : public tetra::Drawer<decltype(eraseParticle),
									 decltype(getParticle),
									 Element, Particle>
{
	GameField<WIDTH, HEIGHT>& field_;

	/** The specialized set function */
	void set(const tetra::point<int>& p, const Element& elem)
	{
		setter_(p, field_, elem);
	}

	/** The specialized get function */
	const Particle& get(const tetra::point<int>& p)
	{
		return getter_(p, field_);
	}

public:
	ParticleEraser(GameField<WIDTH, HEIGHT>& particles) :
		Drawer(&eraseParticle, &getParticle), field_(particles) {}
};

template <int W, int H>
class GameField {
	// Particles
	std::list<Particle> particles_;
	Particle** particleBuffer_ ;

	friend class SaveHandler;

	// Forces
	static float genForce(float min, float max)
	{
		using Dist = std::uniform_real_distribution<float>;
		static Dist urd{};
		return urd(random_engine, Dist::param_type{min, max});
	}

	static float genForce(float mag)
	{
		using Dist = std::uniform_real_distribution<float>;
		static Dist urd{};
		return urd(random_engine, Dist::param_type{-mag, mag});
	}

	static constexpr float forceThreshold = 0.00001;
	static constexpr float spreadFactor = 0.245;

	PointForce* forceBuffer1_;
	PointForce* forceBuffer2_;

	PointForce* forces_;
	PointForce* forceBuffer_;

public:
	ParticleDrawer drawer;
	ParticleEraser eraser;
	ForceDrawer forceDrawer;

	bool gravity = true;

	char borderFlags = (char)Border::Bottom;

	GameField() :
		particleBuffer_(new Particle*[W * H]),
		forceBuffer1_(new PointForce[W * H]),
		forceBuffer2_(new PointForce[W * H]),
		forces_(forceBuffer1_), forceBuffer_(forceBuffer2_),
		drawer(*this), eraser(*this), forceDrawer(*this),
		dragger_(*this)
	{
		memset(particleBuffer_, 0, sizeof(Particle*) * W * H);
		memset(forces_, 0, W * H * sizeof(PointForce));
	}

	~GameField()
	{
		// Delete all of the Particles before particleBuffer_ gets deleted
		particles_.clear();
		delete[] particleBuffer_;

		delete[] forceBuffer1_;
		delete[] forceBuffer2_;
	}

	static bool outOfBounds(const tetra::point<int>& pos)
	{
		static constexpr int w = W - 1, h = H - 1;
		return pos.x >= w || pos.x <= 0 || pos.y >= h  || pos.y <= 0;
	}

	static bool outOfX(int x)
	{
		return x <= 0 || x >= (W - 1);
	}

	static bool outOfY(int y)
	{
		return y <= 0 || y >= (H - 1);
	}

	static constexpr int height() {return H;}
	static constexpr int width() {return W;}

	// Particles
	std::size_t count()
	{
		return particles_.size();
	}

	void add_particle(const tetra::point<float>& pos, const Element& elem)
	{
		if (!outOfBounds(pos) && !occupied_at(pos))
			particles_.emplace_back(pos, elem, *this);
	}

	void add_particle(const ParticleParams& params)
	{
		if (!outOfBounds(params.phys.pos) && !occupied_at(params.phys.pos))
			particles_.emplace_back(params.phys.pos,
				Element::withId(params.elemid),
				params.phys.veloc, params.phys.life, *this);
	}

	bool occupied_at(const tetra::point<int>& pos)
	{
		return *(particleBuffer_ + pos.x * H + pos.y);
	}

	Particle* particle_address_at(const tetra::point<int>& pos)
	{
		return *(particleBuffer_ + pos.x * H + pos.y);
	}

	bool can_add()
	{
		return particles_.size() < PARTICLECAP;
	}

	const Particle* find_nearest(const tetra::point<int>& pos, const std::vector<int>& ids)
	{
		const Particle* result = nullptr;
		static constexpr float maxmin = sqrt(float(WIDTH * WIDTH) + float(HEIGHT * HEIGHT));
		float min = maxmin;

		for (const Particle& p : particles_) {
			if (tetra::contains(ids.begin(), ids.end(), p.elem->id)) {
				float d = pos.distance(p.pos);
				if (d < min) {
					min = d;
					result = &p;
				}
			}
		}

		return result;
	}

	/** Only use when certain that a particle exists at a given point.
	 * Otherwise, use particle_address_at and check against nullptr */
	Particle& particle_at(const tetra::point<int>& pos)
	{
		return *(*(particleBuffer_ + pos.x * H + pos.y));
	}

	Particle** get_occupy()
	{
		return particleBuffer_;
	}

	PointForce* get_field()
	{
		return forces_;
	}

	void occupy_pos(Particle* p, const tetra::point<int>& pos)
	{
		*(particleBuffer_ + pos.x * H + pos.y) = p;
	}

	void unoccupy_pos(const tetra::point<int>& pos)
	{
		*(particleBuffer_ + pos.x * H + pos.y) = nullptr;
	}

	void remove_particle(const tetra::point<int>& pos)
	{
		if (!outOfBounds(pos) && occupied_at(pos))
			particle_at(pos).changeElement(Element::withId(0));
	}

	Particle& back_particle()
	{
		return particles_.back();
	}

	void pop_back_particle()
	{
		particles_.pop_back();
	}

	/** Used when paused */
	void eraseNone()
	{
		auto it = particles_.begin();
		while (it != particles_.end()) {
			if (it->elem->id == 0)
				it = particles_.erase(it);
			else
				++it;
		}
	}

	void updateParticles()
	{
		//~ particles_.sort(Particle::gravityComp);
		auto it = particles_.begin();
		while (it != particles_.end()) {
			if (it->update())
				it = particles_.erase(it);
			else
				++it;
		}
	}

	void updateField() noexcept
	{
		memset(forceBuffer_, 0, W * H * sizeof(PointForce));

		PointForce* pField = forces_ + H;
		PointForce* pRight = forceBuffer_ + (H + H);
		PointForce* pLeft = forceBuffer_;
		PointForce* pAbove = forceBuffer_ + (H - 1);
		PointForce* pBelow = forceBuffer_ + (H + 1);

		static constexpr int N = (W - 1) * H;

		for (int i = H; i < N; ++i) {
			if (i % H) {
				PointForce f = *pField * 0.24;
				*pRight += f;
				*pLeft += f;
				*pAbove += f;
				*pBelow += f;
			}
			++pField;

			++pRight;
			++pLeft;
			++pAbove;
			++pBelow;
		}

		std::swap(forces_, forceBuffer_);
	}

	PointForce* force_cell_at(const tetra::point<int>& pos)
	{
		return forces_ + pos.x * H + pos.y;
	}

	PointForce& force_at(const tetra::point<int>& pos)
	{
		return *(forces_ + pos.x * H + pos.y);
	}

	void repel_point(const tetra::point<int>& pos, float force)
	{
		if (outOfBounds(pos))
			return;

		float fraction = force * 0.25;
		force_at(pos) += {genForce(fraction), genForce(fraction)};
		force_at(pos + Direction::right) += {force, genForce(fraction)};
		force_at(pos + Direction::down) += {genForce(fraction), force};
		force_at(pos + Direction::left) += {-force, genForce(fraction)};
		force_at(pos + Direction::up) += {genForce(fraction), -force};
	}

	void attract_point(const tetra::point<int>& pos, float force)
	{
		if (outOfBounds(pos))
			return;

		float fraction = force * 0.25;
		force_at(pos) -= {genForce(fraction), genForce(fraction)};
		force_at(pos + Direction::right) -= {force, genForce(fraction)};
		force_at(pos + Direction::down) -= {genForce(fraction), force};
		force_at(pos + Direction::left) -= {-force, genForce(fraction)};
		force_at(pos + Direction::up) -= {genForce(fraction), -force};
	}

	void cancel_point(const tetra::point<int>& pos)
	{
		static constexpr PointForce zero{0.0, 0.0};
		force_at(pos) = zero;
		force_at(pos + Direction::right) = zero;
		force_at(pos + Direction::down) = zero;
		force_at(pos + Direction::left) = zero;
		force_at(pos + Direction::up) = zero;
	}

	void angle_force(const tetra::point<int>& pos, float force, float angle)
	{
		PointForce angleForce{cosf(angle) * force * 0.2f, -sinf(angle) * force * 0.2f};

		float fraction = force * 0.25;
		force_at(pos) += angleForce + PointForce{genForce(fraction), genForce(fraction)};
		force_at(pos + Direction::right) += angleForce + PointForce{genForce(fraction), genForce(fraction)};
		force_at(pos + Direction::down) += angleForce + PointForce{genForce(fraction), genForce(fraction)};
		force_at(pos + Direction::left) += angleForce + PointForce{genForce(fraction), genForce(fraction)};
		force_at(pos + Direction::up) += angleForce + PointForce{genForce(fraction), genForce(fraction)};
	}

	// Both
	void reset()
	{
		particles_.clear();
		memset(forces_, 0, W * H * sizeof(PointForce));
	}

	void update()
	{
#ifndef NO_FORCE_FIELD_UPDATE
		updateField();
#endif /* NO_FORCE_FIELD_UPDATE */
#ifndef NO_PARTICLE_UPDATE
		updateParticles();
#endif /* NO_PARTICLE_UPDATE */
	}

	void render(tetra::Sdl::ScreenDrawer& drawer)
	{
#ifdef RENDER_FORCES
		for (int x = 0; x < W; ++x) {
			for (int y = 0; y < H; ++y) {
				tetra::Sdl::Channel c = 255 * force_at({x, y}).distance() * 10.0;
#ifdef DOUBLE_DISPLAY
				if (occupied_at({x, y}))
					drawer.filled_rect({x * DISPLAY_FACTOR, y * DISPLAY_FACTOR},
									   {(x + 1) * DISPLAY_FACTOR, (y + 1) * DISPLAY_FACTOR}, 255);
				if (c > 0)
					drawer.filled_rect({x * DISPLAY_FACTOR, y * DISPLAY_FACTOR},
									   {(x + 1) * DISPLAY_FACTOR, (y + 1) * DISPLAY_FACTOR}, drawer.mapRgb(tetra::Sdl::Color{c, c, c}));
#else
				if (c > 0)
					drawer.putPoint({x, y}, drawer.mapRgb(tetra::Sdl::Color{c, c, c}));
#endif /* DOUBLE_DISPLAY */
			}
		}
#endif /* RENDER_FORCES */

#ifndef NO_RENDER_PARTICLES
		for (const Particle& p : particles_)
			p.render(drawer);
#endif /* NO_RENDER_PARTICLES */
	}

	void collect(const tetra::point<int>& pos, int radius, GameMenu& menu)
	{
		static std::vector<const Particle*> collected;
		collected.clear();

		tetra::point<int> p, begin(std::max(pos.x - radius, 1), std::max(pos.y - radius, 1)),
			end(std::min(pos.x + radius, int(W - 1)), std::min(pos.y + radius, int(H - 1)));

		for (p.x = begin.x; p.x <= end.x; ++p.x) {
			for (p.y = begin.y; p.y < end.y; ++p.y) {
				if (p.distance(pos) < radius && occupied_at(p)) {
					const Particle& part = particle_at(p);
					if (part.elem->groupid)
						collected.push_back(&part);
				}
			}
		}

		if (collected.empty())
			return;

		std::sort(collected.begin(), collected.end(),
				  [](const Particle* lhs, const Particle* rhs)
					{return lhs->elem->id < rhs->elem->id;});

		int id = 0;
		Group::Member* member = nullptr;

		auto it = collected.begin();

		while (it != collected.end()) {
			if ((*it)->elem->id != id) {
				id = (*it)->elem->id;
				member = &Group::withId((*it)->elem->groupid).memberWithId(id);
				menu.addCollectionProgress(*((*it)->elem));
			}

			if (member->count < REQUIRED_PARTICLES) {
				++member->count;
				Group::withId((*it)->elem->groupid).newAvailable(true);
				remove_particle((*it)->pos);
			}
			++it;
		}
	}

	class Dragger
	{
		GameField<W, H>& field_;
		std::list<Particle*> particles_;
		bool on_ = false;

	public:
		Dragger(GameField<W, H>& field) : field_(field) {}

		bool isOn() const
		{
			return on_;
		}

		void activate(const tetra::point<int>& pos, int radius)
		{
			on_ = true;

			tetra::point<int> start{std::max(pos.x - radius, 1), std::max(pos.y - radius, 1)};
			tetra::point<int> end{std::min(pos.x + radius + 1, W - 1),
								  std::min(pos.y + radius + 1, H - 1)};

			tetra::point<int> i;
			for (i.x = start.x; i.x < end.x; ++i.x) {
				for (i.y = start.y; i.y < end.y; ++i.y) {
					if (i.distance(pos) <= radius) {
						Particle* particle = field_.particle_address_at(i);
						if (particle)
							particles_.push_back(particle);
					}
				}
			}
		}

		void deactivate()
		{
			on_ = false;
			particles_.clear();
		}

		void update(const tetra::point<float>& pos)
		{
			auto it = particles_.begin();
			while (it != particles_.end()) {
				if ((*it)->elem->id) {
					if ((*it)->pos.x == pos.x) {
						if ((*it)->pos.y < pos.y)
							(*it)->veloc.y = 1.0;
						else if ((*it)->pos.y > pos.y)
							(*it)->veloc.y = -1.0;
					} else {
						float angle = atan2((*it)->pos.y - pos.y, (*it)->pos.x - pos.x);
						(*it)->veloc.x = -cosf(angle);
						(*it)->veloc.y = -sinf(angle);
					}
					++it;
				} else
					it = particles_.erase(it);
			}
		}
	} dragger_;
};

inline const Particle& getParticle(const tetra::point<int>& pos, GameField<WIDTH, HEIGHT>& particles)
{
	return particles.particle_at(pos);
}

inline void setParticle(const tetra::point<float>& pos, GameField<WIDTH, HEIGHT>& particles, const Element& elem)
{
//	if (particles.can_add())
	particles.add_particle(pos, elem);
}

inline void eraseParticle(const tetra::point<float>& pos, GameField<WIDTH, HEIGHT>& particles, const Element& elem)
{
	particles.remove_particle(pos);
}

inline const PointForce& getForce(const tetra::point<int>& pos, GameField<WIDTH, HEIGHT>& field)
{
	return field.force_at(pos);
}

inline void fieldRepel(const tetra::point<float>& pos, GameField<WIDTH, HEIGHT>& field, float force, float)
{
	field.repel_point(pos, force);
}

inline void fieldAttract(const tetra::point<float>& pos, GameField<WIDTH, HEIGHT>& field, float force, float)
{
	field.attract_point(pos, force);
}

inline void fieldAngle(const tetra::point<float>& pos, GameField<WIDTH, HEIGHT>& field, float force, float angle)
{
	field.angle_force(pos, force, angle);
}

}

#endif /* PARTICLES_GAMEFIELD_H */
