#ifndef PARTICLES_TRACKSCANNER_HPP
#define PARTICLES_TRACKSCANNER_HPP

namespace PixelCosm
{

class TrackScanner {
	Waveform bass_, melody_;
	bool legato_ = false;

	class ChordEvent {
	public:
		std::size_t line;
		ChordGen chord;
		float duration;
		bool changeChord;
		bool rest;
		int octaveShift;

		ChordEvent(std::size_t line, const std::string& chord,
				   std::size_t octaves, float duration,
				   int octaveShift = 0) :
			line(line), chord(chord, octaves),
			duration(duration), changeChord(chord.front() != '*'),
			rest(false), octaveShift(octaveShift) {}

		/*** Rest constructor */
		ChordEvent(std::size_t line, float duration) :
			line(line), duration(duration),
			changeChord(false), rest(true), octaveShift(0) {}

		bool operator<(std::size_t rhs) {return line < rhs;}
	};

	std::deque<ChordEvent> events_;
	std::deque<ChordEvent>::iterator currentEvent_;

	class Repeat {
	public:
		std::size_t line;
		int n;

		Repeat(std::size_t line, int n) : line(line), n(n) {}

		bool operator==(std::size_t rhs) {return line == rhs;}
	};

	std::list<Repeat> repeats_;

	int scanHeader(std::ifstream& file, std::size_t& linen);
	void scanRepeats(std::ifstream& file);
	void scanChords(std::ifstream& file, int octaves);

public:
	TrackScanner() = default;

	void scan(const std::string& filename, int octaves);

	bool done() const;
	ChordEvent& current();
	void next();
	void reset();

	Waveform bassForm() const;
	Waveform melodyForm() const;
	bool isLegato() const;
};

}

#endif // PARTICLES_TRACKSCANNER_HPP
