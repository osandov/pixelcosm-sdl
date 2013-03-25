#ifndef PARTICLES_WAVEFORMS_HPP
#define PARTICLES_WAVEFORMS_HPP

namespace PixelCosm
{

template <typename T>
class Sine : public tetra::MathFunction<T> {
	T B_;

public:
	Sine(T freq) : B_(2.0 * M_PI * freq) {}

	T f(const T& x)
	{
		return 1.5 * sinf(B_ * x) + 0.5 * sinf(B_ * 2.0 * x);
	}
};

template <typename T>
class Square : public tetra::MathFunction<T> {
	T period_;

public:
	Square(T freq) : period_(1.0 / freq) {}

	T f(const T& x)
	{
		return (2.0 * x < period_) ? 0.4 : -0.4;
	}
};

template <typename T>
class Triangle : public tetra::MathFunction<T> {
	T period_;
	T m_;

public:
	Triangle(T freq) : period_(1.0 / freq), m_(4.0 / period_) {}

	T f(const T& x)
	{
		return 2.0 * ((4.0 * x < 3.0 * period_) ? ((4.0 * x < period_) ? m_ * x : -m_ * x + 2.0) : m_ * x - 4.0);
	}
};

template <typename T>
class Sawtooth : public tetra::MathFunction<T> {
	T period_, m_;

public:
	Sawtooth(T freq) : period_(1.0 / freq), m_(2.0 / period_) {}

	T f(const T& x)
	{
		return 0.5 * ((2.0 * x < period_) ? m_ * x : m_ * x - 2.0);
	}
};

template <typename T>
class Noise : public tetra::MathFunction<T> {
	std::uniform_real_distribution<float> urd;
	T period_;

public:
	Noise(T freq) : urd(-1.0, 1.0), period_(1.0 / freq) {}

	T f(const T& x)
	{
		return urd(random_engine);
	}
};

enum class Waveform {
	Sine,
	Square,
	Triangle,
	Sawtooth
};

}


#endif // PARTICLES_WAVEFORMS_HPP

