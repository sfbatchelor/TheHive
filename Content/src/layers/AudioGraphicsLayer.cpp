#include "stdafx.h"
#include "layers\AudioGraphicsLayer.h"

AudioGraphicsLayer::AudioGraphicsLayer(std::shared_ptr<Fft> soundData, std::string name):
	Layer(name),
	m_soundData(soundData)
{
}

AudioGraphicsLayer::~AudioGraphicsLayer()
{
}

Fft AudioGraphicsLayer::getSoundData() const
{
	return *m_soundData;
}
