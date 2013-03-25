#include "MusicGenerator.hpp"

namespace PixelCosm
{

void TrackScanner::scanRepeats(std::ifstream& file)
{
	std::string line;
	std::size_t linen = 0;

	while (!file.eof()) {
		getline(file, line);
		++linen;

		if (line == "|:")
			repeats_.emplace_back(linen, 0);
		else if (line.find(":|") == 0) {
			int repeatn = 1;
			std::size_t pos = line.find('x');

			if (pos != std::string::npos)
				repeatn = std::stoi(line.substr(pos + 1)) - 1;

			repeats_.emplace_back(linen, repeatn);
		}
	}
}

int TrackScanner::scanHeader(std::ifstream& file, std::size_t& linen)
{
	std::string line, sub;
	bass_ = Waveform::Triangle;
	melody_ = Waveform::Sine;
	while (!file.eof()) {
		getline(file, line);
		++linen;

		if (line.empty() || line.front() == '#')
			continue;
		else if (line.find("bass = ") != std::string::npos) {
			sub = line.substr(line.find('=') + 2);
			if (sub == "sine")
				bass_ = Waveform::Sine;
			else if (sub == "square")
				bass_ = Waveform::Square;
			else if (sub == "triangle")
				bass_ = Waveform::Triangle;
			else if (sub == "sawtooth")
				bass_ = Waveform::Sawtooth;
			else
				throw MusicError("Invalid waveform '", sub, "'");
		}
		else if (line.find("melody = ") != std::string::npos)
		{
			sub = line.substr(line.find('=') + 2);
			if (sub == "sine")
				melody_ = Waveform::Sine;
			else if (sub == "square")
				melody_ = Waveform::Square;
			else if (sub == "triangle")
				melody_ = Waveform::Triangle;
			else if (sub == "sawtooth")
				melody_ = Waveform::Sawtooth;
			else
				throw MusicError("Invalid waveform '", sub, "'");
		}
		else if (line.find("style = ") != std::string::npos)
		{
			sub = line.substr(line.find('=') + 2);
			if (sub == "stacatto")
				legato_ = false;
			else if (sub == "legato")
				legato_ = true;
			else
				throw MusicError("Invalid style '", sub, "'");
		}
		else if (line.find("BPM") != std::string::npos)
		{
			try {
				return std::stod(line);
			} catch (const std::invalid_argument& e) {
				throw MusicError("Invalid tempo line");
			} catch (const std::out_of_range& e) {
				throw MusicError("Tempo is out of range");
			}
		} else
			throw MusicError("Invalid line found in track header ",
							 "(the track header is complete once a tempo is found)");
	}
	throw MusicError("Did not find tempo in track header");
}

void TrackScanner::scanChords(std::ifstream& file, int octaves)
{
	std::string line;
	std::size_t linen = 0;
	float tempo = scanHeader(file, linen);

	while (!file.eof()) {
		getline(file, line);
		++linen;
		if (line.empty() || line[0] == '#' || line[0] == '|') {
			continue;
		}

		if (line.find(":|") == 0) {
			auto open = std::find(repeats_.begin(), repeats_.end(), linen);
			auto close = open;
			--open;

			if (repeats_.size() == 1)
				throw MusicError("Unpaired closing repeat found in line ", linen);

			auto first = std::lower_bound(events_.begin(), events_.end(), open->line);
			auto last = std::lower_bound(events_.begin(), events_.end(), close->line);

			for (int i = 0; i < close->n; ++i)
				events_.insert(events_.end(), first, last);

			repeats_.erase(open);
			repeats_.erase(close);

			continue;
		}

		if (line.find("BPM") != std::string::npos) {
			try {
				tempo = std::stod(line);
			} catch (const std::invalid_argument& e) {
				throw MusicError("Could not find valid tempo in tempo line");
			} catch (const std::out_of_range& e) {
				throw MusicError("Tempo is out of range");
			}
			continue;
		}

		while (true) {
			std::size_t pos = line.find("\t\t");
			if (pos == std::string::npos)
				break;
			line.erase(pos, 1);
		}

		std::size_t lpos = line.find('\t'), rpos = line.rfind('\t');
		if (lpos == std::string::npos)
			throw MusicError("Did not find tab in chord line ", line, "");

		float duration = 60.0 / tempo;
		int octaveShift = 0;
		if (lpos != rpos) {
			std::string shift_s = line.substr(rpos + 1);
			if (shift_s == "8va")
				octaveShift = 1;
			else if (shift_s == "8vb")
				octaveShift = -1;
			else
				throw MusicError("Invalid specifer found after second indent");
			duration *= std::stod(line.substr(lpos + 1, rpos - lpos - 1));
		} else
			duration *= std::stod(line.substr(lpos + 1));

		std::string chord = line.substr(0, lpos);
		if (chord.front() == '-')
			events_.emplace_back(linen, duration);
		else
			events_.emplace_back(linen, chord, octaves, duration, octaveShift);
	}

	if (!repeats_.empty())
		throw MusicError("Unpaired opening repeat found in line ", repeats_.front().line);
}

void TrackScanner::scan(const std::string& filename, int octaves)
{
	std::ifstream file(filename);

	if (!file.is_open())
		throw MusicError("Could not open file '", filename, "'");

	scanRepeats(file);

	file.clear();
	file.seekg(0, std::ios::beg);

	scanChords(file, octaves);

	currentEvent_ = --events_.begin();
}

bool TrackScanner::done() const
{
	return currentEvent_ == events_.end();
}

TrackScanner::ChordEvent& TrackScanner::current()
{
	return *currentEvent_;
}

void TrackScanner::next()
{
	++currentEvent_;

}
void TrackScanner::reset()
{
	currentEvent_ = --events_.begin();
}

Waveform TrackScanner::bassForm() const
{
	return bass_;
}

Waveform TrackScanner::melodyForm() const
{
	return melody_;
}

bool TrackScanner::isLegato() const
{
	return legato_;
}

}
