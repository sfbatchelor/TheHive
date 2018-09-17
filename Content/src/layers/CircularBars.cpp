#include "stdafx.h"
#include "layers\CircularBars.h"

CircularBars::CircularBars(std::shared_ptr<Fft> soundData ):
	AudioGraphicsLayer(soundData, "CircularBars"),
	m_barWidth(5),
	m_barXSpacing(5),
	m_barMaxY(900),
	m_radius(600)
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
	ofRotateZDeg(60 * sin(ofGetElapsedTimef()*.1));
	m_line.draw();
	for (auto bar : m_bars)
		bar.draw();
	ofScale(1, -1, 1);
	ofRotateZDeg(120*cos(ofGetElapsedTimef()*.1));
	m_line.draw();

	ofScale(-1, -2*sin(cos(ofGetElapsedTimef())), 1);
	ofRotateZDeg(20*tan(ofGetElapsedTimef()*.1));
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
	float angle = glm::fract(ofGetElapsedTimef()*.3)*36000. / m_soundData->m_numFftBands/4;
	m_line.clear();
	for (int i = 0; i < m_soundData->m_numFftBands/4; i++)
	{
		float radius = m_radius;
		radius += m_radius*m_soundData->m_fftSmoothed[0];

		int height = m_soundData->m_fftSmoothed[i] * m_barMaxY;
		int y = glm::cos(glm::radians(angle*i))*(radius+height);
		int x = glm::sin(glm::radians(angle*i))*(radius+height);
		x += ofRandom(-10, 10);
		x += ofRandom(-10, 10);
		m_line.addVertex(x, y);

	}
	//loop back
	int height = m_soundData->m_fftSmoothed[0] * m_barMaxY;
	int y = glm::cos(glm::radians(0.))*(m_radius+height);
	int x = glm::sin(glm::radians(0.))*(m_radius+height);
	m_line.addVertex(x, y);
}
