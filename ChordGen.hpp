#ifndef PARTICLES_CHORDGEN_HPP
#define PARTICLES_CHORDGEN_HPP

namespace PixelCosm
{

class ChordGen {
	int keyShift_ = 0;
	bool minor_ = false, augmented_ = false, diminished_ = false;
	bool seventh_ = false, maj7_ = false, dim7_ = false;
	int inversion_ = 0;
	std::size_t octaves_;

	float weight(int x) const;

public:
	ChordGen() = default;
	ChordGen(const std::string& chord, std::size_t octaves);

	int bass() const;

	void operator()(std::discrete_distribution<int>& rand) const;
};

}


#endif // PARTICLES_CHORDGEN_HPP
