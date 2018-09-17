#include "stdafx.h"
#include "layers\Layer.h"
#include "..\..\include\layers\Layer.h"


Layer::Layer(std::string name):
	m_stringName(name)
{
}

Layer::~Layer()
{

}

void Layer::play()
{
	m_isPlaying = true;
}

void Layer::pause()
{
	m_isPlaying = false;
}

void Layer::setPlay(bool playState)
{
	m_isPlaying = playState;
}

std::string Layer::getName()
{
	return m_stringName;
}
