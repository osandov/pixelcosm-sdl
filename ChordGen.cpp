#include "MusicGenerator.hpp"

namespace PixelCosm
{

float ChordGen::weight(int x) const
{
	x = x - keyShift_;

	if (x < 24)
		return 0.0;

	while (!(x < 12))
		x -= 12;
	while (x < 0)
		x += 12;

	if (x == 0)
		return 5.0;
	else if (!(augmented_ || diminished_) && (x == 7))
		return 4.0;
	else if ((augmented_ && (x == 8)) || (diminished_ && (x == 6)))
		return 3.0;
	else if ((!minor_ && (x == 4)) || (minor_ && (x == 3)))
		return 3.0;
	else if ((seventh_ && (x == 10)) || (maj7_ && (x == 11)) || (dim7_ && x == 9))
		return 2.0;
	else
		return 0.0;
}

ChordGen::ChordGen(const std::string& chord, std::size_t octaves) :
	octaves_(octaves)
{
	std::size_t i = 0;
	if (!chord.empty()) {
		if (chord.front() == '*')
			++i;

		char root = chord[i];
		if (root == 'C')
			keyShift_ = 0;
		else if (root == 'D')
			keyShift_ = 2;
		else if (root == 'E')
			keyShift_ = 4;
		else if (root == 'F')
			keyShift_ = 5;
		else if (root == 'G')
			keyShift_ = 7;
		else if (root == 'A')
			keyShift_ = 9;
		else if (root == 'B')
			keyShift_ = 11;
		else
			throw MusicError("Invalid root '", chord[0], "'");
	} else
		throw MusicError("Empty chord");

	++i;
	if (chord.size() > 1) {
		char accidental = chord[i];
		if (accidental == 'b') {
			--keyShift_;
			++i;
		} else if (accidental == '#') {
			++keyShift_;
			++i;
		}

		if (i < chord.size() && chord[i] == 'm') {
			minor_ = true;
			++i;
		}

		if (i < chord.size() && chord[i] == 'o') {
			if (minor_)
				throw MusicError("Specifying minor for diminished chord '",
					chord, "' is redundant");

			minor_ = diminished_ = true;
			++i;
		}

		bool halfDim = false;
		if (i < chord.size() && chord[i] == 'x') {
			if (minor_)
				throw MusicError("Specifying minor for diminished 7th chord '",
					chord, "' is redundant");

			minor_ = diminished_ = seventh_ = true;
			halfDim = true;
			++i;
		}

		if (i < chord.size() && chord[i] == '+') {
			if (minor_)
				throw MusicError("Augmented chord '", chord,
					"' must be major");

			augmented_ = true;
			++i;
		}

		if (i < chord.size() && chord[i] == 'M') {
			++i;
			if (i < chord.size() && chord[i] == '7') {
				maj7_ = true;
				++i;
			} else
				throw MusicError("'M' (found in '", chord,
				"') is used to signify a major 7th chord; ",
				"it must be followed by '7'");
		}

		if (i < chord.size() && chord[i] == '7') {
			if (maj7_)
				throw MusicError("Chord '", chord,
					"' cannot be both 7th and major 7th");
			if (halfDim)
				throw MusicError("Specifying '7' for half-diminished ",
					"chord '", chord, "' is redundant");

			if (diminished_)
				dim7_ = true;
			else
				seventh_ = true;
			++i;
		}

		if (i < chord.size() && chord[i] == 'i') {
			inversion_ = 1;
			++i;
			if (i < chord.size()) {
				if (chord[i] != 'i')
					throw MusicError("Invalid character '", chord[i],
						"' after inversion specifier in chord '",
						chord, "'");
				inversion_ = 2;
				++i;
				if (i < chord.size()) {
					if (chord[i] != 'i')
						throw MusicError("Invalid character '", chord[i],
							"' after inversion specifier in chord '",
							chord, "'");
					if (!(seventh_ || maj7_ || dim7_))
						throw MusicError("Only seventh chords have a ",
							"third inversion");
					inversion_ = 3;
					++i;
				}
			}
		}

		if (i < chord.size())
			throw MusicError("Invalid and/or out of place ",
				"tokens in chord '", chord, "'");
	}
}

int ChordGen::bass() const
{
	int r = keyShift_;
	if (inversion_ == 1)
		r = keyShift_ + ((minor_) ? 3 : 4);
	else if (inversion_ == 2) {
		if (augmented_)
			r = keyShift_ + 8;
		else if (diminished_)
			r = keyShift_ + 6;
		else
			r = keyShift_ + 7;
	} else if (inversion_ == 3) {
		if (seventh_)
			r = keyShift_ + 10;
		else if (maj7_)
			r = keyShift_ + 11;
		else if (dim7_)
			r = keyShift_ + 9;
	}
	return r;
}

void ChordGen::operator()(std::discrete_distribution<int>& rand) const
{
	rand.param({12 * octaves_, 0.0, 12.0 * octaves_, [this](int x){return this->weight(x);}});
}

}
