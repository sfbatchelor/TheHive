#include "stdafx.h"
#include "layers/DebugAudio.h"

DebugAudio::DebugAudio(std::shared_ptr<Fft> soundData):
	AudioGraphicsLayer(soundData, "Debug Audio"),
	m_barWidth(10),
	m_barXSpacing(4),
	m_barMaxY(400)
{
	for (int i = 0; i < m_soundData->m_numFftBands; i++)
	{
		int x = (m_barWidth + m_barXSpacing)  * i;
		int height = m_soundData->m_fftSmoothed[i] * m_barMaxY;
		m_bars.push_back(std::shared_ptr<ofRectangle>(new ofRectangle(x, ofGetHeight(), m_barWidth, -height )));
	}


}

DebugAudio::~DebugAudio()
{

}

void DebugAudio::draw()
{
	ofSetColor(255);
	for (auto bar : m_bars)
		ofDrawRectangle(*bar);
}

void DebugAudio::update()
{
	int i = 0;
	for (auto bar : m_bars)
	{
		int height = m_soundData->m_fftSmoothed[i] * m_barMaxY;
		bar->setHeight(  -height);
		i++;
	}
}

void DebugAudio::reset()
{
}
