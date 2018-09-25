#pragma once
#include "utils\ShaderWatcher.h"
#include "AudioGraphicsLayer.h"

class Scanlines : public AudioGraphicsLayer
{



public:

	Scanlines(std::shared_ptr<Fft> soundData);
	~Scanlines();

	virtual void draw();
	virtual void update();
	virtual void reset();



protected:

	ofEasyCam m_cam;
	ofPlanePrimitive m_fullscreenQuad;
	ShaderWatcher m_shader;
	int m_width;
	int m_height;



};