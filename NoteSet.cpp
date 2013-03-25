#include "MusicGenerator.hpp"

namespace PixelCosm
{

template <typename F>
void NoteSet::setup(Mix_Chunk*& chunk, float freq)
{
	F f(freq);
	std::size_t period = 1.0 / freq ;

	chunk = (Mix_Chunk*)SDL_malloc(sizeof(Mix_Chunk));
	chunk->allocated = 1;
	chunk->abuf = (Uint8*)SDL_malloc(sizeof(Sint16) * period);
	chunk->alen = Uint32(sizeof(Sint16) * period);
	chunk->volume = MIX_MAX_VOLUME;

	Sint16* buf = (Sint16*)chunk->abuf;
	for (std::size_t t = 0; t < period; ++t)
		buf[t] = f(t) * 12000.0;
}

NoteSet::NoteSet(std::size_t octavesBelowMiddleC, std::size_t octavesAboveMiddleC) :
	octavesBelowMiddleC_(octavesBelowMiddleC),
	octavesAboveMiddleC_(octavesAboveMiddleC),
	octaves_(octavesBelowMiddleC + octavesAboveMiddleC + 1),

	sine_(new Mix_Chunk*[12 * octaves_]),
	square_(new Mix_Chunk*[12 * octaves_]),
	triangle_(new Mix_Chunk*[12 * octaves_]),
	sawtooth_(new Mix_Chunk*[12 * octaves_]),
	chunks_{sine_, square_, triangle_, sawtooth_},
	silence_((Mix_Chunk*)SDL_malloc(sizeof(Mix_Chunk))),

	rand_(12 * octaves_, 0.0, 12.0 * octaves_, [](float){return 0;})
{
	for (std::size_t i = 0; i < 12 * octaves_; ++i) {
		float freq = 440.0 * pow(2.0, (i - 9.0 - 12.0 * octavesBelowMiddleC_) / 12.0) / MIX_DEFAULT_FREQUENCY;
		setup<Sine<float>>(sine_[i], freq);
		setup<Square<float>>(square_[i], freq);
		setup<Triangle<float>>(triangle_[i], freq);
		setup<Sawtooth<float>>(sawtooth_[i], freq);
	}
	silence_->allocated = 1;
	silence_->abuf = (Uint8*)SDL_malloc(sizeof(Sint16));
	silence_->alen = Uint32(sizeof(Sint16));
	silence_->volume = 0;
}

NoteSet::~NoteSet()
{
	for (std::size_t i = 0; i < 12 * octaves_; ++i) {
		Mix_FreeChunk(sine_[i]);
		Mix_FreeChunk(square_[i]);
		Mix_FreeChunk(triangle_[i]);
		Mix_FreeChunk(sawtooth_[i]);
	}

	delete[] sine_;
	delete[] square_;
	delete[] triangle_;
	delete[] sawtooth_;

	Mix_FreeChunk(silence_);
}

void NoteSet::setVolume(int volume)
{
	for (std::size_t i = 0; i < 12 * octaves_; ++i) {
		Mix_VolumeChunk(sine_[i], volume);
		Mix_VolumeChunk(square_[i], volume);
		Mix_VolumeChunk(triangle_[i], volume);
		Mix_VolumeChunk(sawtooth_[i], volume);
	}
}

int NoteSet::getOctaves() const
{
	return octaves_;
}

void NoteSet::setChord(const ChordGen& chord, bool change)
{
	if (change)
		chord(rand_);
	bass_ = chord.bass() + 12;
}

Mix_Chunk* NoteSet::random(Waveform form)
{
	using Dist = std::uniform_int_distribution<int>;
	static Dist uid{0, int(12 * octaves_ - 1)};
	return chunks_[int(form)][uid(random_engine)];
}

Mix_Chunk* NoteSet::randomInKey(Waveform form)
{
	return chunks_[int(form)][rand_(random_engine)];
}

Mix_Chunk* NoteSet::bass(Waveform form, int shift)
{
	return chunks_[int(form)][bass_ + 12 * shift];
}

Mix_Chunk* NoteSet::silence()
{
	return silence_;
}

}
