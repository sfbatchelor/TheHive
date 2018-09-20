#pragma once
#include <ofRectangle.h>
#include "AudioGraphicsLayer.h"

class CircularNormals : public AudioGraphicsLayer
{



public:

	CircularNormals(std::shared_ptr<Fft> soundData);
	~CircularNormals();

	virtual void draw();
	virtual void update();
	virtual void reset();



protected:

	void generateLines();
	void updateLines();

	int m_lineWidth;
	int m_lineCount;
	int m_lineXSpacing;
	int m_lineMaxY;
	ofEasyCam m_cam;
	int m_radius;

	std::vector<std::shared_ptr<ofPolyline>> m_lines;



};