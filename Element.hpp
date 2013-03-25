#ifndef PARTICLES_ELEMENT_HPP
#define PARTICLES_ELEMENT_HPP

#include "Dependencies.hpp"
#include "InterDir.hpp"

namespace PixelCosm
{

class Element {
	static std::vector<Element> elements_;

	static float motionf(float x) {
		return 1.0 - pow(x, 4.0);
	}

public:
	class param_type {
	public:
		std::string name;
		int groupid;
		tetra::Sdl::Color color;
		InterPattern interpattern;
		int intercount;
		std::vector<int> intertowards;
		float towardsrange;
		int mass;
		float entropy;
		float initforce;
		bool decays;
		int decaysto;
		std::pair<float, float> lifespan;
	};

	/** The motion constant of the Element, calculated from its mass */
	const float motionk;

	/** The internally generated identification number of the Element */
	const int id;

	/** The text name of the Element */
	const std::string name;

	/** The identification number of the Group to which the Element
	 * belongs
	 */
	const int groupid;

private:
	/** The RGB color of the Element. Use @c pixel for rendering */
	const tetra::Sdl::Color color;
public:

	/** The mapped color value of the element */
	tetra::Sdl::Pixel pixel;

	/** The Interaction pattern of the Element */
	const InterPattern interpattern;

	/** The number of times a Particle will try to interact in a single
	 * frame
	 */
	const int intercount;

	/** A list of Element identification numbers towards which a
	 * Particle will try to interact
	 */
	std::vector<int> intertowards;

	/** The maximum distance for towards reactions. A value of zero
	 * represents no restriction
	 */
	const float towardsrange;

	/** The mass of the element */
	const int mass;

	/** The entropy of the Element */
	const float entropy;

	/** The amount of force a particle with this Element is initialized
	 * with
	 */
	const float initforce;

	/** Whether the Element decays over time or instead only takes
	 * damage
	 */
	const bool decays;

	/** The id of the Element which the Element becomes upon decaying */
	const int decaysto;

	/** The possible range in the length of time a Particle which decays
	 * remains active
	 */
	const std::pair<float, float> lifespan;

	/** Whether the element interacts with the 'none' element */
	bool interactsWithNone = false;

	Element(const param_type& params);

	static const Element& withId(int id);
	static Element& withName(const std::string& name);
	static const Element& withColor(const tetra::Sdl::Color& color);
	static bool withColorExists(const tetra::Sdl::Color& color);

	/** Returns the identification number following the last generated
	 * identification number
	 */
	static int getEndId();

	static void create(const param_type& params);

	static void mapColors(const tetra::Sdl::ScreenDrawer& drawer);

	bool operator==(const Element& rhs) const;
	bool operator!=(const Element& rhs) const;
};

}

#endif /* PARTICLES_ELEMENT_HPP */
