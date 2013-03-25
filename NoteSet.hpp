#ifndef PARTICLES_NOTESET_HPP
#define PARTICLES_NOTESET_HPP

namespace PixelCosm
{

class NoteSet {
	std::size_t octavesBelowMiddleC_, octavesAboveMiddleC_, octaves_;
	Mix_Chunk** sine_;
	Mix_Chunk** square_;
	Mix_Chunk** triangle_;
	Mix_Chunk** sawtooth_;
	Mix_Chunk** chunks_[4];
	Mix_Chunk* silence_;

	std::discrete_distribution<int> rand_;
	int bass_;

	template <typename F>
	void setup(Mix_Chunk*& chunk, float freq);

public:
	NoteSet(std::size_t octavesBelowMiddleC, std::size_t octavesAboveMiddleC);
	~NoteSet();

	void setVolume(int volume);
	int getOctaves() const;

	void setChord(const ChordGen& chord, bool change);

	Mix_Chunk* random(Waveform form = Waveform::Sine);
	Mix_Chunk* randomInKey(Waveform form = Waveform::Sine);
	Mix_Chunk* bass(Waveform form = Waveform::Sine, int shift = 0);
	Mix_Chunk* silence();
};

}

#endif // PARTICLES_NOTESET_HPP
