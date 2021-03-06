#include "stdafx.h"
#include "layers\Scanlines.h"

Scanlines::Scanlines(std::shared_ptr<Fft> soundData):
	AudioGraphicsLayer(soundData, "Scanlines"),
	m_width(ofGetWidth()),
	m_height(ofGetHeight()),
	m_xDivisions(2),
	m_yDivisions(2)
{
	m_shader.load("scanlinesVert.glsl", "scanlinesFrag.glsl");
	reset();
}

Scanlines::~Scanlines()
{
	m_shader.exit();
}

void Scanlines::draw()
{
	m_cam.begin();
	m_shader.getShader().begin();
	m_shader.getShader().setUniform1i("uNumFftBands", m_soundData->m_numFftBands);
	m_shader.getShader().setUniform1i("uXDivisions", m_xDivisions);
	m_shader.getShader().setUniform1i("uYDivisions", m_yDivisions);
	m_shader.getShader().setUniform1fv("uFft", &m_soundData->m_fftSmoothed[0], m_soundData->m_numFftBands);
	m_shader.getShader().setUniform1f("uTime", ofGetElapsedTimef());
	ofSetColor(255);
	m_fullscreenQuad.enableTextures();
	m_fullscreenQuad.draw();
	m_shader.getShader().end();
	m_cam.end();
}

void Scanlines::update()
{
	m_shader.update();
}

void Scanlines::reset()
{
	m_width = ofGetWidth();
	m_height = ofGetHeight();

	// unit quad with normalized texels
	m_fullscreenQuad.set(m_width, m_height, 10, 10);
	m_fullscreenQuad.mapTexCoords(0, 0, 1., 1.);

}
