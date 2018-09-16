#include "stdafx.h"
#include "3d\PostProcess.h"

PostProcess::PostProcess(std::string stringName, int width, int height, int finalBufferFormat):
	m_stringName(stringName),
	m_width(width),
	m_height(height),
	m_finalBufferFormat(finalBufferFormat)
{
}

PostProcess::~PostProcess()
{
}

void PostProcess::draw(float x, float y)
{
	m_finalBuffer->getTexture().draw(x, y);
}

void PostProcess::reset(int width, int height)
{
	if (width != 0)
		m_width = width;
	if (height != 0)
		m_height = height;

	m_finalBuffer.reset(new ofFbo());
	m_finalBuffer->allocate(m_width, m_height, m_finalBufferFormat);
}

std::string PostProcess::getName()
{
	return m_stringName;
}

ofTexture & PostProcess::getTexture()
{
	return m_finalBuffer->getTexture();
}
