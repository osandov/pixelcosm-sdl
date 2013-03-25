#ifndef PARTICLES_INTERDIR_HPP
#define PARTICLES_INTERDIR_HPP

namespace PixelCosm
{

enum class InterPattern {
	Uniform,
	Up,
	Down,
};

class InterDirGenerator {
	static int uniformStart()
	{
		using Dist = std::uniform_int_distribution<int>;
		static Dist uid{0, 3};
		return uid(random_engine);
	}

	static bool uniformTowards()
	{
		using Dist = std::bernoulli_distribution;
		static Dist bd{0.4};
		return bd(random_engine);
	}

	static bool towardsOnWhichAxis()
	{
		using Dist = std::bernoulli_distribution;
		static Dist bd{0.5};
		return bd(random_engine);
	}

	void towardsAndAway(InterPattern pattern,
						const tetra::point<int>& from, const tetra::point<int>& to,
						int& towards, int& away)
	{
		towards = -1;
		away = -1;
		if (towardsOnWhichAxis()) {
			if (from.x < to.x) {
				towards = 3;
				away = 1;
			} else if (from.x > to.x) {
				towards = 1;
				away = 3;
			} else if (pattern == InterPattern::Uniform) {
				if (from.y < to.y) {
					towards = 2;
					away = 0;
				} else if (from.y > to.y) {
					towards = 0;
					away = 2;
				}
			}
		} else {
			if (from.y < to.y) {
				towards = 2;
				away = 0;
			} else if (from.y > to.y) {
				towards = 0;
				away = 2;
			} else if (pattern == InterPattern::Uniform) {
				if (from.x < to.x) {
					towards = 3;
					away = 1;
				} else if (from.x > to.x) {
					towards = 1;
					away = 3;
				}
			}
		}
	}

	int buffer_[4];

public:
	InterDirGenerator() = default;

	int* begin() {return buffer_;}

	int* end() {return buffer_ + 4;}

	int operator[](int i) {
		return *(buffer_ + i);
	}

	void generate(InterPattern pattern)
	{
		int start = uniformStart();
		if (pattern == InterPattern::Uniform) {
			for (int i = 0; i < 4; ++i) {
				buffer_[i] = start;
				if (++start > 3)
					start = 0;
			}
		} else if (pattern == InterPattern::Up) {
			for (int i = 0; i < 4; ++i) {
				buffer_[i] = (start == 2) ? 0 : start;
				++start;
				if (start > 3)
					start = 0;
			}
		} else if (pattern == InterPattern::Down) {
			for (int i = 0; i < 4; ++i) {
				buffer_[i] = (start) ? start : 2;
				++start;
				if (start > 3)
					start = 0;
			}
		}
	}

	void generate(InterPattern pattern, const tetra::point<int>& from, const tetra::point<int>& to)
	{
		int start = uniformStart();
		int towards, away;
		towardsAndAway(pattern, from, to, towards, away);
		if (pattern == InterPattern::Uniform) {
			for (int i = 0; i < 4; ++i) {
				buffer_[i] = (start == away) ? towards : (uniformTowards()) ? towards : start;
				++start;
				if (start > 3)
					start = 0;
			}
		} else {
			for (int i = 0; i < 4; ++i) {
				buffer_[i] = (towards == -1) ? start : towards;
				++start;
				if (start > 3)
					start = 0;
			}
		}
	}
};

}

#endif
