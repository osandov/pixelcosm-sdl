#include "PixelCosm.hpp"

namespace PixelCosm
{

Element::Element(const param_type& params) :
	motionk(motionf(params.mass / 100.0)),
	id(elements_.size()), name(params.name), groupid(params.groupid),
	color(params.color), interpattern(params.interpattern),
	intercount(params.intercount), intertowards(params.intertowards),
	towardsrange(params.towardsrange), mass(params.mass),
	entropy(params.entropy), initforce(params.initforce),
	decays(params.decays), decaysto(params.decaysto),
	lifespan(params.lifespan) {}

const Element& Element::withId(int id) {
	return elements_[id];
}

Element& Element::withName(const std::string& name) {
	auto found = std::find_if(elements_.begin(), elements_.end(),
		[&name](const Element& e) {return e.name == name;});
	if (found == elements_.end())
		throw std::runtime_error(
			std::string("No Element with name ' ") + name + " '");
	return *found;
}

const Element& Element::withColor(const tetra::Sdl::Color& color) {
	auto found = std::find_if(elements_.begin(), elements_.end(),
		[&color](const Element& e) {return e.color == color;});
	if (found == elements_.end())
		throw std::runtime_error("No Element with the given color");
	return *found;
}

bool Element::withColorExists(const tetra::Sdl::Color& color) {
	auto found = std::find_if(elements_.begin(), elements_.end(),
		[&color](const Element& e) {return e.color == color;});
	return found != elements_.end();
}

/** Returns the identification number following the last generated
 * identification number
 */
int Element::getEndId() {
	return elements_.size();
}

void Element::create(const param_type& params) {
	auto found = std::find_if(elements_.begin(), elements_.end(),
		[&params](const Element& e) {return e.name == params.name;});
	if (found != elements_.end())
		throw std::runtime_error(
			std::string("Element with name ' ") + params.name + " ' already exists");
	elements_.emplace_back(params);
}

void Element::mapColors(const tetra::Sdl::ScreenDrawer& drawer) {
	for (Element& e : elements_)
		e.pixel = drawer.mapRgb(e.color);
}

bool Element::operator==(const Element& rhs) const
{
	return id == rhs.id;
}

bool Element::operator!=(const Element& rhs) const
{
	return id != rhs.id;
}

}
