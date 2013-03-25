#ifndef PARTICLES_INTERACTION_HPP
#define PARTICLES_INTERACTION_HPP

#include "Element.hpp"
#include "Particle.hpp"
#include "MusicGenerator.hpp"

namespace PixelCosm
{

class Interaction {
protected:
	MusicGenerator& musicGen_;

public:
	typedef std::pair<int, int> idPair;

	class param_type {
	public:
		idPair interactIds;
		float frequency;
	};

	idPair interactIds;
	float frequency;
	std::shared_ptr<Interaction> next;

	Interaction(const param_type& params, MusicGenerator& musicGen) :
		musicGen_(musicGen), interactIds(params.interactIds), frequency(params.frequency)
	{
		if (interactIds.first < interactIds.second)
			std::swap(interactIds.first, interactIds.second);
	}

	virtual void exec(Particle& first, Particle& second) = 0;
};

class ElementChangeInteraction : public Interaction {
public:
	class param_type : public Interaction::param_type {
	public:
		idPair newIds;

		param_type(const Interaction::param_type& base) :
			Interaction::param_type(base) {}
	};

	idPair newIds;

	ElementChangeInteraction(const param_type& params, MusicGenerator& musicGen) :
		Interaction(params, musicGen),
		newIds(params.newIds)
	{
		if (params.interactIds.first < params.interactIds.second)
			std::swap(newIds.first, newIds.second);
	}

	void exec(Particle& first, Particle& second)
	{
		if (second.elem->id)
			musicGen_.playRandomInKey(this);

		first.changeElement(Element::withId((newIds.first != -1) ? newIds.first : first.elem->id));
		second.changeElement(Element::withId((newIds.second != -1) ? newIds.second : second.elem->id));
	}
};

class RepelInteraction : public Interaction {
public:
	class param_type : public Interaction::param_type {
	public:
		float force;

		param_type(const Interaction::param_type& base) :
			Interaction::param_type(base) {}
	};

	float force;

	RepelInteraction(const param_type& params, MusicGenerator& musicGen) :
		Interaction(params, musicGen),
		force(params.force) {}

	void exec(Particle& first, Particle& second)
	{
		musicGen_.renewNoise();
		first.repel(force);
	}
};

class AttractInteraction : public Interaction {
public:
	class param_type : public Interaction::param_type {
	public:
		float force;

		param_type(const Interaction::param_type& base) :
			Interaction::param_type(base) {}
	};

	float force;

	AttractInteraction(const param_type& params, MusicGenerator& musicGen) :
		Interaction(params, musicGen),
		force(params.force) {}

	void exec(Particle& first, Particle& second)
	{
		musicGen_.renewNoise();
		first.attract(force);
	}
};

class AngleInteraction : public Interaction {
public:
	class param_type : public Interaction::param_type {
	public:
		float angle, force;

		param_type(const Interaction::param_type& base) :
			Interaction::param_type(base) {}
	};

	float angle, force;

	AngleInteraction(const param_type& params, MusicGenerator& musicGen) :
		Interaction(params, musicGen),
		angle(params.angle), force(params.force) {}

	void exec(Particle& first, Particle& second)
	{
		first.angle(angle, force);
	}
};

class CancelForceInteraction : public Interaction {
public:
	class param_type : public Interaction::param_type {
	public:

		param_type(const Interaction::param_type& base) :
			Interaction::param_type(base) {}
	};

	CancelForceInteraction(const param_type& params, MusicGenerator& musicGen) :
		Interaction(params, musicGen)
		{}

	void exec(Particle& first, Particle& second)
	{
		first.cancel_force();
	}
};

class SwapInteraction : public Interaction {
public:
	class param_type : public Interaction::param_type {
	public:
		int newId;

		param_type(const Interaction::param_type& base) :
			Interaction::param_type(base) {}
	};

	int newId;

	SwapInteraction(const param_type& params, MusicGenerator& musicGen) :
		Interaction(params, musicGen),
		newId(params.newId) {}

	void exec(Particle& first, Particle& second)
	{
		musicGen_.playRandomInKey(this);

		if (newId != -1)
			second.changeElement(Element::withId(newId));
		first.swap(second);
	}
};

class InteractionHandler {
	class compareIdPairs {
	public:
		bool operator()(const Interaction::idPair& lhs, const Interaction::idPair& rhs) const {
			return (lhs.first < rhs.first) || ((lhs.first == rhs.first) && (lhs.second < rhs.second)) ;
		}
	};

	static MusicGenerator* musicGen_;
	static std::multimap<Interaction::idPair, std::shared_ptr<Interaction>, compareIdPairs> map_;
	typedef std::multimap<Interaction::idPair, std::shared_ptr<Interaction>, compareIdPairs>::iterator map_iterator;

	static bool doInteract(float freq) {
		using Dist = std::bernoulli_distribution;
		static Dist bd{};
		return bd(random_engine, Dist::param_type{freq});
	}

public:
	static void musicGen(MusicGenerator* musicGen)
	{
		musicGen_ = musicGen;
	}

	static MusicGenerator& musicGen()
	{
		return *musicGen_;
	}

	template <typename T>
	static Interaction* create(const typename T::param_type& params)
	{
		Interaction* inter = new T(params, *musicGen_);
		map_.insert({inter->interactIds, std::shared_ptr<Interaction>(inter)});
		return inter;
	}

	static bool interact(Particle& first, Particle& second)
	{
		static Interaction::idPair prevKey{0, 0};
		static std::pair<map_iterator, map_iterator> range{map_.end(), map_.end()};

		Particle* first_p = &first;
		Particle* second_p = &second;

		if (first.elem->id < second.elem->id)
			std::swap(first_p, second_p);

		Interaction::idPair key{first_p->elem->id, second_p->elem->id};

		if (key != prevKey)
			range = map_.equal_range((prevKey = key)); // Intentional assignment

		bool did = false;
		for (auto it = range.first; it != range.second; ++it) {
			if (doInteract(it->second->frequency)) {
				did = true;
				Interaction* inter = it->second.get();
				do {
					inter->exec(*first_p, *second_p);
				} while ((inter = inter->next.get())); // Intentional assignment
			}
		}

		return did;
	}
};

}

#endif /* PARTICLES_INTERACTION_HPP */
