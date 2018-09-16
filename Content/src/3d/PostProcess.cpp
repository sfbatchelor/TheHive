#include "stdafx.h"
#include "3d\PostProcess.h"

PostProcess::PostProcess(std::string stringName, int width, int height, int finalBufferFormat):
	m_stringName(stringName),
	m_width(width),
	m_height(height),
	m_finalBufferFormat(finalBufferFormat),
	m_isInitialized(false)
{
}

PostProcess::~PostProcess()
{
}

void PostProcess::draw(float x, float y)
{
	checkAndInitialize();
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
	m_finalBuffer->getTexture().getTextureData().bFlipTexture = true;

	// unit quad with normalized texels
	m_plane.set(2. * (m_width/m_height), 2, 10, 10);
	m_plane.mapTexCoords(0, 0, 1., 1.);
}

std::string PostProcess::getName()
{
	return m_stringName;
}

ofTexture & PostProcess::getTexture()
{
	checkAndInitialize();
	return m_finalBuffer->getTexture();
}

void PostProcess::checkAndInitialize()
{
	if (!m_isInitialized)
	{
		reset();
		m_isInitialized = true;
	}
}
