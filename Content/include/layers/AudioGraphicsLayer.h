#pragma once
#include "Layer.h"
#include <memory>
#include "utils/Fft.h"

class AudioGraphicsLayer : public Layer
{

public:

	AudioGraphicsLayer(std::shared_ptr<Fft> soundData, std::string name);
	~AudioGraphicsLayer();

	virtual void draw() = 0;
	virtual void update() = 0;

	Fft getSoundData() const;

protected:

	std::shared_ptr<Fft> m_soundData;


};
