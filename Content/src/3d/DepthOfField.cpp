#include "stdafx.h"
#include "3d\DepthOfField.h"

DepthOfField::DepthOfField(std::string stringName, int width, int height, int finalBufferFormat):
	PostProcess(stringName, width, height, finalBufferFormat)
{
	m_gaussianShader.load("gaussianVert.glsl", "gaussianFrag.glsl");
	m_dofFinalShader.load("dofFinalVert.glsl", "dofFinalFrag.glsl");
}

DepthOfField::~DepthOfField()
{
	m_dofFinalShader.exit();
	m_gaussianShader.exit();
}

void DepthOfField::update(ofTexture & sourceTex)
{
	checkAndInitialize();
	m_dofFinalShader.update();
	m_gaussianShader.update();

	//GAUSSIAN BLUR PASS
	//first pass using the fbo result, then it switches between vertical and horizontal blur passes ping-pong style
	bool horizontal = true, firstIteration = true;
	int amount = 10;

	m_gaussianShader.getShader().begin();
	for (unsigned int i = 0; i < amount; i++)
	{
		m_gaussianShader.getShader().setUniform1i("horizontal", horizontal);
		//m_gaussianShader.getShader().setUniformTexture("depth", m_fbo->getDepthTexture(), 5);
		m_gaussianShader.getShader().setUniform1i("DOF", true);
		if (horizontal)
			m_gaussianBack->begin();
		else
			m_gaussianFront->begin();

		if (firstIteration)
			sourceTex.bind(0);
		else if (horizontal)
			m_gaussianFront->getTexture().bind(0);
		else
			m_gaussianBack->getTexture().bind(0);

		ofSetColor(255);
		m_plane.enableTextures();
		ofClear(0, 0, 0, 255);
		m_plane.draw();

		if (firstIteration)
			sourceTex.unbind(0);
		else if (horizontal)
			m_gaussianFront->getTexture().unbind(0);
		else
			m_gaussianBack->getTexture().unbind(0);
		if (horizontal)
			m_gaussianBack->end();
		else
			m_gaussianFront->end();

		horizontal = !horizontal;
		if (firstIteration)
			firstIteration = false;
	}
	m_gaussianShader.getShader().end();


	if (m_finalBuffer)
	{
		//FINAL BLEND PASS
		m_finalBuffer->begin();
		m_dofFinalShader.getShader().begin();
		m_dofFinalShader.getShader().setUniformTexture("scene", sourceTex, 5);
		m_dofFinalShader.getShader().setUniformTexture("bloomBlur", m_gaussianFront->getTexture(), 6);
//		m_dofFinalShader.getShader().setUniformTexture("depth", m_fbo->getDepthTexture(), 7);
		ofSetColor(255);
		m_plane.enableTextures();
		ofClear(0, 0, 0, 255);
		m_plane.draw();
		m_gaussianFront->getTexture().unbind(6);
		m_dofFinalShader.getShader().end();
		m_finalBuffer->end();
	}


}

void DepthOfField::reset(int width, int height)
{
	PostProcess::reset(width, height);

	m_gaussianFront.reset(new ofFbo());
	m_gaussianFront->allocate(m_width, m_height, m_finalBufferFormat);
	m_gaussianBack.reset(new ofFbo());
	m_gaussianBack->allocate(m_width, m_height, m_finalBufferFormat);
}
