#include "MusicGenerator.hpp"

namespace PixelCosm
{

void MusicGenerator::scanFile(const std::string& filename)
{
	static std::vector<TrackScanner>::iterator it = tracks_.begin();
	it->scan(filename, noteSet_.getOctaves());
	++it;
}

MusicGenerator::MusicGenerator() : noteSet_(3, 1) {}

MusicGenerator::~MusicGenerator()
{
	noise_.unreg(MIX_CHANNEL_POST);
}

void MusicGenerator::setup(const std::string& filename)
{
	Mix_AllocateChannels(reservedChannels_ + toneChannels_);
	Mix_ReserveChannels(reservedChannels_);
	std::ifstream file(filename);
	std::string line, path;

	std::size_t pos = filename.find('/');
	if (pos == std::string::npos)
		path = "tracks/";
	else
		path = filename.substr(0, pos + 1) + "tracks/";

	if (!file.is_open())
		throw MusicError("Could not open file '", filename, "'");

	std::size_t tracks = 0;
	while (!file.eof()) {
		getline(file, line);
		if (!line.empty() && line[0] != '#')
			++tracks;
	}

	file.clear();
	file.seekg(0, std::ios::beg);
	tracks_.resize(tracks);

	while (!file.eof()) {
		getline(file, line);
		if (!line.empty() && line[0] != '#')
			scanFile(path + line);
	}

	if (tracks_.empty()) {
		currentTrack_ = shuffledTracks_.end();
		return;
	}

	for (auto it = tracks_.begin(); it != tracks_.end(); ++it)
		shuffledTracks_.push_back(it);

	std::shuffle(shuffledTracks_.begin(), shuffledTracks_.end(), random_engine);
	currentTrack_ = shuffledTracks_.begin();
	bass_ = (*currentTrack_)->bassForm();
	melody_ = (*currentTrack_)->melodyForm();
}

void MusicGenerator::halt()
{
	noise_.unreg(MIX_CHANNEL_POST);
	Mix_HaltChannel(-1);
}

void MusicGenerator::setVolume(int volume)
{
	Mix_VolumeMusic(volume);
	noteSet_.setVolume(volume);
}

int MusicGenerator::getVolume() const
{
	return Mix_VolumeMusic(-1);
}

bool MusicGenerator::enabled() const
{
	return enabled_;
}

void MusicGenerator::toggleEnabled()
{
	if ((enabled_ ^= true)) { // Intentional compound assignment
		Mix_Resume(-1);
		noise_.reg(MIX_CHANNEL_POST);
	} else {
		Mix_Pause(-1);
		noise_.unreg(MIX_CHANNEL_POST);
	}
}

void MusicGenerator::playRandomInKey(void* id)
{
	Mix_Chunk* chunk = noteSet_.randomInKey(melody_);

	int tag = (intptr_t)id;
	int channel = Mix_GroupOldest(tag);
	if (channel == -1) {
		channel = Mix_PlayChannel(-1, chunk, -1);
		if (channel == -1) {
			channel = Mix_GroupOldest(-1);
			Mix_PlayChannel(channel, chunk, -1);
		}
	} else if (changeTone_)
		Mix_PlayChannel(channel, chunk, -1);

	Mix_GroupChannel(channel, tag);
	Mix_Volume(channel, toneVolume_);
	Mix_FadeOutChannel(channel, toneLength_ * 1000.0);
}

void MusicGenerator::activateNoise()
{
	noise_.reg(MIX_CHANNEL_POST);
}

void MusicGenerator::deactivateNoise()
{
	noise_.unreg(MIX_CHANNEL_POST);
}

void MusicGenerator::renewNoise()
{
	if (enabled_)
		renewNoise_ = true;
}

void MusicGenerator::skipSong()
{
	Mix_HaltChannel(-1);
	between_ = 1.0;
	if (++currentTrack_ != shuffledTracks_.end()) {
		bass_ = (*currentTrack_)->bassForm();
		melody_ = (*currentTrack_)->melodyForm();
	}
}

void MusicGenerator::step(float delta)
{
	if (!enabled_ || tracks_.empty())
		return;

	changeTone_ = false;

	if (currentTrack_ == shuffledTracks_.end()) {
		for (auto& track : tracks_)
			track.reset();

		std::shuffle(shuffledTracks_.begin(), shuffledTracks_.end(), random_engine);
		currentTrack_ = shuffledTracks_.begin();
		between_ = 3.0;
		return;
	}
	between_ -= delta;

	static constexpr float noiseFadeIn = 0.025, noiseFadeOut = 0.02;
	static constexpr float noiseMax = 0.025;
	float noiseMag = noise_.magnitude();
	if (renewNoise_) {
		noiseMag = (noiseMag + noiseFadeIn * delta > noiseMax) ? noiseMax : noiseMag + noiseFadeIn * delta;
		renewNoise_ = false;
	} else
		noiseMag = (noiseMag - noiseFadeOut * delta < 0.0) ? 0.0 : noiseMag - noiseFadeOut * delta;
	noise_.magnitude(noiseMag);

	if ((remaining_ -= delta) < fadeLength_ && !(*currentTrack_)->isLegato()
		&& Mix_Fading(0) != MIX_FADING_OUT)
			Mix_FadeOutChannel(0, remaining_ * 1000.0);

	if (between_ <= 0.0 && !(*currentTrack_)->done() && !Mix_Playing(0)) {
		(*currentTrack_)->next();
		if ((*currentTrack_)->done()) {
			between_ = 3.0;
			if (++currentTrack_ != shuffledTracks_.end()) {
				bass_ = (*currentTrack_)->bassForm();
				melody_ = (*currentTrack_)->melodyForm();
			}
			return;
		}

		auto& current = (*currentTrack_)->current();

		remaining_ = current.duration;
		noteSet_.setChord(current.chord, current.changeChord);

		if (!current.rest) {
			Mix_Chunk* chunk = noteSet_.bass(bass_, current.octaveShift);
			Mix_PlayChannel(0, chunk,
				current.duration * (sizeof(Uint16) * MIX_DEFAULT_FREQUENCY) / float(chunk->alen));
			Mix_Volume(0, MIX_MAX_VOLUME);
			changeTone_ = true;
		} else {
			Mix_PlayChannel(0, noteSet_.silence(),
				current.duration * MIX_DEFAULT_FREQUENCY);
		}
	}
}

}
