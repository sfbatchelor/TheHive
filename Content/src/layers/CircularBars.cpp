#include "stdafx.h"
#include "layers\CircularBars.h"

CircularBars::CircularBars(std::shared_ptr<Fft> soundData ):
	AudioGraphicsLayer(soundData, "CircularBars"),
	m_barWidth(5),
	m_barXSpacing(5),
	m_barMaxY(700),
	m_radius(450)
{
	generatePolyLine();
}

CircularBars::~CircularBars()
{
}

void CircularBars::draw()
{
	m_cam.begin();
	ofSetColor(255);
	ofPushMatrix();
	m_line.draw();
	ofPopMatrix();
	m_cam.end();
}

void CircularBars::update()
{
	generatePolyLine();
}

void CircularBars::reset()
{
	generatePolyLine();
}

void CircularBars::generatePolyLine()
{
	float angle = 2880. / m_soundData->m_numFftBands;
	m_line.clear();
	for (int i = 0; i < m_soundData->m_numFftBands; i++)
	{

		int height = m_soundData->m_fftSmoothed[i] * m_barMaxY;
		int y = glm::cos(glm::radians(angle*i))*(m_radius+height);
		int x = glm::sin(glm::radians(angle*i))*(m_radius+height);
		m_line.addVertex(x, y);
	}
	//loop back
	int height = m_soundData->m_fftSmoothed[0] * m_barMaxY;
	int y = glm::cos(glm::radians(0.))*(m_radius+height);
	int x = glm::sin(glm::radians(0.))*(m_radius+height);
	m_line.addVertex(x, y);
}
