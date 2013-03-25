#include "Particle.hpp"
#include "Interaction.hpp"
#include "GameField.hpp"

namespace PixelCosm
{

const Element* Particle::none = nullptr;

Particle::Particle(const tetra::point<float>& pos, const Element& elem,
				   const tetra::point<float>& veloc, float life,
				   GameField<WIDTH, HEIGHT>& field) :
	pos(pos), veloc(veloc),
	elem(&elem),
	life(life),
	target((elem.intertowards.size() && elem.decays) ? field.find_nearest(pos, elem.intertowards) : nullptr),
	nexttarget(genNextTarget()),
	field_(field)
{
	field_.occupy_pos(this, pos);
}

Particle::Particle(const tetra::point<float>& pos, const Element& elem,
				   GameField<WIDTH, HEIGHT>& field) :
	Particle(pos, elem,
			{genInit(elem.initforce), genInit(elem.initforce)},
			(elem.decays) ? genLife(elem.lifespan) : 1.0,
			field) {}

Particle::~Particle()
{
	field_.unoccupy_pos(pos);
}

void Particle::setNoneElement(const Element* p)
{
    none = p;
}

bool Particle::update()
{
	if (elem->id == 0)
		return true;

	if (elem->decays) {
		if ((life -= decayAmount) <= 0.f) {
			changeElement(Element::withId(elem->decaysto));
			return false;
		}
	}

	if (elem->intertowards.size())
		nexttarget -= decayAmount;

	Particle* other;
#ifndef NO_INTERACTIONS
	int k = 0, count = elem->intercount;
	tetra::point<int> interPos;
	do {
		if (elem->intertowards.empty())
			dirGen_.generate(elem->interpattern);
		else {
			if (target && ((elem->towardsrange == 0.0) ?
				true : pos.distance(target->pos) < elem->towardsrange))
			{
				if (tetra::contains(elem->intertowards.begin(), elem->intertowards.end(), target->elem->id))
					dirGen_.generate(elem->interpattern, pos, target->pos);
				else
					target = nullptr;
			} else {
				if (nexttarget < 0.0) {
					target = field_.find_nearest(pos, elem->intertowards);
					nexttarget = genNextTarget();
				}
				dirGen_.generate(elem->interpattern);
			}
		}

		for (int i = 0; i < 4; ++i) {
			interPos = pos + Direction::dir(dirGen_[i]);
			if (!field_.outOfBounds(interPos)) {
				other = field_.particle_address_at(interPos);
				if (other != nullptr) {
					if (InteractionHandler::interact(*this, *other))
						break;
				} else if (elem->interactsWithNone) {
					field_.add_particle(interPos, *none);
					if (InteractionHandler::interact(*this, field_.back_particle()))
						break;
					field_.pop_back_particle();
				}
			}
		}
	} while (++k < count);
#endif

	if (elem->mass < 90 && field_.gravity) {
		if (veloc.y < 2.f && elem->mass > none->mass)
			veloc.y += gravForce / elem->motionk;
		else if (elem->mass < none->mass) {
			if (elem->mass && veloc.y > -0.5f)
				veloc.y -= ((none->mass - elem->mass) * riseForce) / elem->motionk;
			else if (veloc.y > -2.f && !elem->mass)
				veloc.y -= 16.f * riseForce;
		}
	}

	if (doRotate(elem->entropy)) {
		float angle = genAngle(elem->entropy);
		float sine = sinf(angle), cosine = cosf(angle);
		float x = veloc.x;
		veloc.x = x * cosine - veloc.y * sine;
		veloc.y = x * sine + veloc.y * cosine;
	}

	veloc += field_.force_at(pos);
	veloc *= elem->motionk;

	float mag = fmaxf(fabsf(veloc.x), fabsf(veloc.y));
	tetra::point<float> unitVeloc = (mag) ?
		tetra::point<float>{veloc / mag} :
		tetra::point<float>{0.f, 0.f};

	tetra::point<int> remaining
		{(unitVeloc.x) ? int(veloc.x * 2.f / unitVeloc.x) : -1,
		(unitVeloc.y) ? int(veloc.y * 2.f / unitVeloc.y) : -1};

	bool cont = true;

	field_.unoccupy_pos(pos);
	do {
		if (remaining.x < 1 && remaining.y < 1) {
			unitVeloc.x = fmod(veloc.x * 2.f, unitVeloc.x);
			unitVeloc.y = fmod(veloc.y * 2.f, unitVeloc.y);
			cont = false;
		}

		if (remaining.x > -1) {
			pos.x += unitVeloc.x;
			--remaining.x;
			if (pos.x >= field_.width() - 1) {
				if (field_.borderFlags & (char)Border::Right) {
					pos.x = field_.width() - 2;
					veloc.x *= -tetra::square(elem->motionk * 0.5f);
					break;
				} else {
					elem = none;
					return false;
				}
			} else if (pos.x < 1) {
				if (field_.borderFlags & (char)Border::Left) {
					pos.x = 1;
					veloc.x *= -tetra::square(elem->motionk * 0.5f);
					break;
				} else {
					elem = none;
					return false;
				}
			}

			other = field_.particle_address_at(pos);
			if (other) {
				if (elem->mass <= other->elem->mass) {
					pos.x -= unitVeloc.x;
					if (elem->mass == other->elem->mass) {
						veloc *= elem->motionk;
						std::swap(veloc, other->veloc);
						veloc *= other->elem->motionk;
					}
					remaining.x = -1;
				} else {
					veloc.x *= 0.5f;
					field_.unoccupy_pos(other->pos);
					other->pos.x = pos.x - unitVeloc.x;
					field_.occupy_pos(other, other->pos);
				}
			}
		}

		if (remaining.y > -1) {
			pos.y += unitVeloc.y;
			--remaining.y;
			if (pos.y >= field_.height() - 1) {
				if (field_.borderFlags & (char)Border::Bottom) {
					pos.y = field_.height() - 2;
					if (field_.gravity)
						veloc.y = 0.f;
					else
						veloc.y *= -tetra::square(elem->motionk * 0.5f);
					break;
				} else {
					elem = none;
					return false;
				}
			} else if (pos.y < 1) {
				if (field_.borderFlags & (char)Border::Top) {
					pos.y = 1;
					veloc.y *= -tetra::square(elem->motionk * 0.5f);
					break;
				} else {
					elem = none;
					return false;
				}
			}

			other = field_.particle_address_at(pos);
			if (other) {
				if (elem->mass <= other->elem->mass) {
					pos.y -= unitVeloc.y;
					if (elem->mass == other->elem->mass) {
						veloc *= elem->motionk;
						std::swap(veloc, other->veloc);
						veloc *= other->elem->motionk;
					}
					remaining.y = -1;
				} else {
					veloc.y *= 0.5f;
					field_.unoccupy_pos(other->pos);
					other->pos.y = pos.y - unitVeloc.y;
					field_.occupy_pos(other, other->pos);
				}
			}
		}
	} while (cont);

	field_.occupy_pos(this, pos);
	return false;
}

void Particle::swap(Particle& other) {
	tetra::point<float> newPos = other.pos;
	field_.occupy_pos(&other, pos);
	other.pos = pos;
	pos = newPos;
	field_.occupy_pos(this, pos);
}

void Particle::repel(float force) {
	field_.repel_point(pos, force);
}

void Particle::attract(float force) {
	field_.attract_point(pos, force);
}

void Particle::angle(float angle, float force) {
	field_.angle_force(pos, force, angle);
}

void Particle::cancel_force() {
	field_.cancel_point(pos);
}

}
