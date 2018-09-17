#pragma once
#include <ofRectangle.h>
#include "AudioGraphicsLayer.h"

class DebugAudio : public AudioGraphicsLayer
{
public:

	DebugAudio(std::shared_ptr<Fft> soundData);
	~DebugAudio();

	virtual void draw();
	virtual void update();
	virtual void reset();


protected:

	std::vector<std::shared_ptr<ofRectangle>> m_bars;
	int m_barWidth;
	int m_barXSpacing;
	int m_barMaxY;


};