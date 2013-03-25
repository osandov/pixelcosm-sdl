#include "GameField.hpp"

namespace ParticleGame
{

template <int W, int H>
void GameField<W, H>::updateField() noexcept
{
	static constexpr int soBuffer = W * H * sizeof(PointForce);
	memset(forceBuffer_, 0, soBuffer);

	PointForce* pField = forces_ + H;
	PointForce* pRight = forceBuffer_ + (H + H);
	PointForce* pLeft = forceBuffer_;
	PointForce* pAbove = forceBuffer_ + (H - 1);
	PointForce* pBelow = forceBuffer_ + (H + 1);

	static constexpr int N = (W - 1) * H;

	for (int i = H; i < N; ++i) {
		if (i % H && (i + 1) % H) {
			PointForce f = *pField * 0.24;
			*pRight += f;
			*pLeft += f;
			*pAbove += f;
			*pBelow += f;
		}
		++pField;

		++pRight;
		++pLeft;
		++pAbove;
		++pBelow;
	}

	std::swap(forces_, forceBuffer_);
}

}
