#include "stdafx.h"
#include "layers\Scanlines.h"

Scanlines::Scanlines(std::shared_ptr<Fft> soundData):
	AudioGraphicsLayer(soundData, "Scanlines"),
	m_width(ofGetWidth()),
	m_height(ofGetHeight())
{
	m_shader.load("scanlinesVert.glsl", "scanlinesFrag.glsl");
}

Scanlines::~Scanlines()
{
	m_shader.exit();
}

void Scanlines::draw()
{
	m_shader.getShader().begin();
	m_shader.getShader().setUniform1i("uNumFftBands", m_soundData->m_numFftBands);
	m_shader.getShader().setUniform1fv("uFft", &m_soundData->m_fftSmoothed[0], m_soundData->m_numFftBands);
	m_shader.getShader().setUniform1f("uTime", ofGetElapsedTimef());
	m_fullscreenQuad.draw();
	m_shader.getShader().end();
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
	m_fullscreenQuad.set(2. * (m_width/m_height), 2, 10, 10);
	m_fullscreenQuad.mapTexCoords(0, 0, 1., 1.);

}
