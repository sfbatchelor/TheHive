#include "stdafx.h"
#include "layers\CircularNormals.h"

CircularNormals::CircularNormals(std::shared_ptr<Fft> soundData ):
	AudioGraphicsLayer(soundData, "CircularNormals"),
	m_lineWidth(5),
	m_lineXSpacing(5),
	m_lineMaxY(200),
	m_radius(500),
	m_lineCount(300)
{
	generateLines();
}

CircularNormals::~CircularNormals()
{
}

void CircularNormals::draw()
{
	m_cam.begin();
	ofSetLineWidth(10);
	ofSetColor(255);
	ofPushMatrix();


	for (auto line : m_lines)
		line->draw();
	ofScale(-1, -1, 1);
	for (auto line : m_lines)
		line->draw();


	ofPopMatrix();
	m_cam.end();
}

void CircularNormals::update()
{
	updateLines();
}

void CircularNormals::reset()
{
	generateLines();
}

void CircularNormals::generateLines()
{
	float angle = 360. / m_lineCount;
	m_lines.clear();
	for (int i = 0; i < m_lineCount; i++)
	{
		ofPolyline line;
		// Work out the line's corresponding sound value 
		float radius = m_radius;
		radius += m_radius*m_soundData->m_fftSmoothed[i];
		int fftNum = float(float(i) / float(m_lineCount))*float(m_soundData->m_numFftBands);
		int height = m_soundData->m_fftSmoothed[fftNum] * m_lineMaxY;
		height *= 2;
		radius += m_radius * m_soundData->m_fftSmoothed[fftNum]; // offset from centre depoending on sound val


		// get top point
		int y = glm::cos(glm::radians(angle*i))*(radius+height);
		int x = glm::sin(glm::radians(angle*i))*(radius+height);
		line.addVertex(x, y);

		// get bottom point 
		y = glm::cos(glm::radians(angle*i))*(radius-(height*2));
		x = glm::sin(glm::radians(angle*i))*(radius-(height*2));
		line.addVertex(x, y);

		// add to container
		m_lines.push_back(std::shared_ptr<ofPolyline>(new ofPolyline(line)));
	}

}

void CircularNormals::updateLines()
{
	int i = 0;
	float angle = 360. / m_lineCount;
	for (auto line : m_lines)
	{
		// Work out the line's corresponding sound value 
		float radius = m_radius;
		radius += m_radius*m_soundData->m_fftSmoothed[0]; //rise and fall with audio to make it seem more responsive
		float fftNum = float(float(i) / float(m_lineCount))*float(m_soundData->m_numFftBands/3);
		//some averaging wizardry to get smoothed sound value
		float soundVal = m_soundData->m_fftSmoothed[glm::floor(fftNum)];
		float nextVal = m_soundData->m_fftSmoothed[glm::ceil(fftNum)];
		soundVal = glm::mix(soundVal, nextVal, glm::fract(fftNum));

		int height = soundVal * m_lineMaxY;
		height *= 2;
		radius += m_radius * soundVal*7; // offset from centre depoending on sound val



		// get top point
		int y = glm::cos(glm::radians(angle*i))*(radius+height);
		int x = glm::sin(glm::radians(angle*i))*(radius+height);
		line->getVertices()[0].x = x; 
		line->getVertices()[0].y = y;

		// get bottom point 
		y = glm::cos(glm::radians(angle*i))*(radius-(height*4));
		x = glm::sin(glm::radians(angle*i))*(radius-(height*4));
		line->getVertices()[1].x = x; 
		line->getVertices()[1].y = y; 
		i++;
	}

}
