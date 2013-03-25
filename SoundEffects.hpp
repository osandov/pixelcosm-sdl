#ifndef PARTICLES_SOUNDEFFECTS_HPP
#define PARTICLES_SOUNDEFFECTS_HPP

namespace PixelCosm
{

class SoundEffect {
	/** Effect adapter */
	static void effect_a(int chan, void* stream, int len, void* instance)
	{
		static_cast<SoundEffect*>(instance)->effect(chan, stream, len);
	}

	/** Done adapter */
	static void done_a(int chan, void* instance)
	{
		static_cast<SoundEffect*>(instance)->done(chan);
	}

	virtual void effect(int chan, void* stream, int len) = 0;
	virtual void done(int chan) {};

public:
	int reg(int channel)
	{
		return Mix_RegisterEffect(channel, effect_a, done_a, this);
	}

	int unreg(int channel)
	{
		return Mix_UnregisterEffect(channel, effect_a);
	}
};

class NoiseEffect : public SoundEffect {
	typedef std::uniform_real_distribution<float> Dist;
	Dist urd;
	float mag_;

	void effect(int chan, void* stream, int len)
	{
		Sint16* data = static_cast<Sint16*>(stream);
		int N = len / sizeof(Sint16);

		for (int i = 0; i < N; ++i, ++data)
			*data += urd(random_engine, Dist::param_type{-mag_, mag_})
					  * 32767 * MIX_MAX_VOLUME / Mix_VolumeMusic(-1);
	}

public:
	NoiseEffect(float magnitude = 0.0) : mag_(magnitude) {}

	float magnitude() const {return mag_;}
	void magnitude(float mag) {mag_ = mag;}
};

class BlurEffect : public SoundEffect {
	typedef std::uniform_real_distribution<float> Dist;
	Dist urd;
	float mag_;

	void effect(int chan, void* stream, int len)
	{
		Sint16* data = static_cast<Sint16*>(stream);
		int N = len / sizeof(Sint16);

		for (int i = 0; i < N; ++i, ++data)
			*data *= urd(random_engine, Dist::param_type{1.f - mag_, 1.f})
					 * MIX_MAX_VOLUME / Mix_VolumeMusic(-1);
	}

public:
	BlurEffect(float magnitude = 0.0) : mag_(magnitude) {}

	float magnitude() const {return mag_;}
	void magnitude(float mag) {mag_ = mag;}
};

}

#endif // PARTICLES_SOUNDEFFECTS_HPP
