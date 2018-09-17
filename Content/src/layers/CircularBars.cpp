#include "stdafx.h"
#include "layers\CircularBars.h"

CircularBars::CircularBars(std::shared_ptr<Fft> soundData ):
	AudioGraphicsLayer(soundData, "CircularBars"),
	m_barWidth(5),
	m_barXSpacing(5),
	m_barMaxY(400)
{
	for (int i = 0; i < m_soundData->m_numFftBands; i++)
	{
		int x = (m_barWidth + m_barXSpacing)  * i;
		int height = m_soundData->m_fftSmoothed[i] * m_barMaxY;
		m_bars.push_back(std::shared_ptr<ofRectangle>(new ofRectangle(x, 0, m_barWidth, height)));
	}
}

CircularBars::~CircularBars()
{
}

void CircularBars::draw()
{
	m_cam.begin();
	ofSetColor(255);
	for (auto bar : m_bars)
		ofDrawRectangle(*bar);
	m_cam.end();
}

void CircularBars::update()
{
	int i = 0;
	for (auto bar : m_bars)
	{
		int height = m_soundData->m_fftSmoothed[i] * m_barMaxY;
		bar->setHeight(height);
		i++;
	}
}

void CircularBars::reset()
{
}
