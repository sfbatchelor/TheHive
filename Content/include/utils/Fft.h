#pragma once
#include <array>


struct Fft
{
	static constexpr size_t m_numFftBands = 512;
	std::array<float, m_numFftBands> m_fftSmoothed{ {0} };

};
