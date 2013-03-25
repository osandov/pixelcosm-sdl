#ifndef PARTICLES_MUSICGENERATOR_HPP
#define PARTICLES_MUSICGENERATOR_HPP

#include "Dependencies.hpp"

namespace PixelCosm
{

class MusicError : public std::exception {
	std::string what_;

	void construct(std::ostringstream&) {}

	template <typename T, typename... Args>
	void construct(std::ostringstream& stream, T value, Args... args)
	{
		stream << value;
		construct(stream, args...);
	}

public:
	MusicError(const MusicError& rhs) :
		what_(rhs.what()) {}

	template <typename... Args>
	MusicError(Args... args)
	{
		std::ostringstream stream;
		construct(stream, args...);
		what_ = stream.str();
	}

	const char* what() const throw() {return what_.c_str();}
};

}

#include "ChordGen.hpp"
#include "Waveforms.hpp"
#include "NoteSet.hpp"
#include "SoundEffects.hpp"
#include "TrackScanner.hpp"

namespace PixelCosm
{

class MusicGenerator {
	static constexpr int reservedChannels_ = 1, toneChannels_ = 10;
	static constexpr float fadeLength_ = 0.5;
	static constexpr int toneVolume_ = MIX_MAX_VOLUME * 0.15;
	static constexpr float toneLength_ = 1.0;

	NoteSet noteSet_;
	std::vector<TrackScanner> tracks_;
	std::vector<std::vector<TrackScanner>::iterator> shuffledTracks_;
	std::vector<std::vector<TrackScanner>::iterator>::iterator currentTrack_;

	Waveform bass_, melody_;

	float between_ = 0.0, remaining_ = 0.0;

	void scanFile(const std::string& filename);

	NoiseEffect noise_;
	bool changeTone_ = false, renewNoise_ = false;

	bool enabled_ = true;

public:
	MusicGenerator();
	~MusicGenerator();

	void setup(const std::string& filename);
	void halt();

	void setVolume(int volume);
	int getVolume() const;

	bool enabled() const;
	void toggleEnabled();

	void playRandomInKey(void* id);

	void activateNoise();
	void deactivateNoise();
	void renewNoise();

	void skipSong();

	void step(float delta = 1.0 / FRAMERATECAP);
};

}

#endif // PARTICLES_MUSICGENERATOR_HPP
