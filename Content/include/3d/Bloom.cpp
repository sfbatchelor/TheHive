#include "stdafx.h"
#include "Bloom.h"

Bloom::Bloom(std::string stringName, int width, int height, int finalBufferFormat):
	PostProcess(stringName, width, height, finalBufferFormat)
{
	m_gaussianShader.load("gaussianVert.glsl", "gaussianFrag.glsl");
	m_bloomFinalShader.load("bloomFinalVert.glsl", "bloomFinalFrag.glsl");
	m_highlightShader.load("highlightVert.glsl", "highlightFrag.glsl");
}

Bloom::~Bloom()
{
	m_bloomFinalShader.exit();
	m_highlightShader.exit();
	m_gaussianShader.exit();
}

void Bloom::update(ofTexture & sourceTex, ofTexture & depthTex)
{
	checkAndInitialize();
	m_bloomFinalShader.update();
	m_gaussianShader.update();
	m_highlightShader.update();

	//render highlights out from source image
	m_highlightBuffer->begin();
	m_highlightShader.getShader().begin();
	m_highlightShader.getShader().setUniformTexture("src", sourceTex, 5);
	ofSetColor(255);
	m_plane.enableTextures();
	ofClear(0, 0, 0, 255);
	m_plane.draw();
	m_highlightShader.getShader().end();
	m_highlightBuffer->end();

	//GAUSSIAN BLUR PASS
	//first pass using the brightness fbo result, then it switches between vertical and horizontal blur passes ping-pong style
	bool horizontal = true, firstIteration = true;
	int amount = 20;

	m_gaussianShader.getShader().begin();
	for (unsigned int i = 0; i < amount; i++)
	{
		m_gaussianShader.getShader().setUniform1i("horizontal", horizontal);
		m_gaussianShader.getShader().setUniform1i("DOF", false);
		if (horizontal)
			m_gaussianBack->begin();
		else
			m_gaussianFront->begin();

		if (firstIteration)
			m_highlightBuffer->getTexture().bind(0);
		else if (horizontal)
			m_gaussianFront->getTexture().bind(0);
		else
			m_gaussianBack->getTexture().bind(0);

		ofSetColor(255);
		m_plane.enableTextures();
		ofClear(0, 0, 0, 255);
		m_plane.draw();

		if (firstIteration)
			m_highlightBuffer->getTexture().unbind(0);
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
		m_bloomFinalShader.getShader().begin();
		m_bloomFinalShader.getShader().setUniformTexture("scene", sourceTex, 5);

		m_bloomFinalShader.getShader().setUniformTexture("bloomBlur", m_gaussianFront->getTexture(), 6);
		m_bloomFinalShader.getShader().setUniformTexture("depth", depthTex, 7);
		ofSetColor(255);
		m_plane.enableTextures();
		ofClear(0, 0, 0, 255);
		m_plane.draw();
		sourceTex.unbind(5);
		m_gaussianFront->getTexture().unbind(6);
		m_bloomFinalShader.getShader().end();
		m_finalBuffer->end();
	}


}

void Bloom::reset(int width, int height)
{
	PostProcess::reset(width, height);

	m_gaussianFront.reset(new ofFbo());
	m_gaussianFront->allocate(m_width, m_height, m_finalBufferFormat);
	m_gaussianBack.reset(new ofFbo());
	m_gaussianBack->allocate(m_width, m_height, m_finalBufferFormat);

	m_highlightBuffer.reset(new ofFbo());
	m_highlightBuffer->allocate(m_width, m_height, m_finalBufferFormat);

}
