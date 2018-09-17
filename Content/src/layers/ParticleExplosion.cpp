#include "stdafx.h"
#include "layers/ParticleExplosion.h"


ParticleExplosion::ParticleExplosion(std::shared_ptr<Fft> soundData):
	AudioGraphicsLayer(soundData, "Particle Explosion"),
	m_particleBounds(glm::vec3(10000.)),
	m_numPoints(1024 * 38),
	m_minDepth ( -300.),
	m_maxDepth ( 300.)
{

	m_constantShader.load("constantVert.glsl", "constantFrag.glsl", "sphereGeom.glsl");// todo - not actually constant need to fix
	m_particleSim.loadCompute("compute.glsl");
	ofImage image;
	image.load("paint1.png");
	m_texture.allocate(image.getPixels());
	m_points = GpuParticleFactory::fromImage(image, m_numPoints, m_minDepth, m_maxDepth);
	m_particleSim.loadParticles(m_points);

	m_cam.setVFlip(true); //flip for upside down image
	m_cam.setFarClip(100000000.);
}

ParticleExplosion::~ParticleExplosion()
{
	m_constantShader.exit();
}

void ParticleExplosion::draw()
{
	m_cam.begin();
	ofPushMatrix();
	ofTranslate(-m_texture.getWidth() / 2, -m_texture.getHeight() / 2);
	ofEnableAlphaBlending();
	ofSetColor(255);
	m_constantShader.getShader().begin();
	m_constantShader.getShader().setUniform1i("uNumFftBands", m_soundData->m_numFftBands);
	m_constantShader.getShader().setUniform1fv("uFft", &m_soundData->m_fftSmoothed[0], m_soundData->m_numFftBands);
	m_constantShader.getShader().setUniform1f("uTime", ofGetElapsedTimef());
	m_constantShader.getShader().setUniformMatrix4f("modelView", m_cam.getModelViewMatrix());
	glPointSize(3);
	m_constantShader.getShader().setUniform1f("uAlpha", 1.f);
	m_particleSim.draw(GL_POINTS);
	m_constantShader.getShader().end();
	ofPopMatrix();
	m_cam.end();
}

void ParticleExplosion::update()
{
	m_particleSim.setPlay(m_isPlaying);

	m_constantShader.update();
	m_particleSim.begin();
	m_texture.bindAsImage(0, GL_READ_ONLY);
	m_particleSim.getShader().setUniform1i("uNumPointsSF", m_numPoints / 1024);
	m_particleSim.getShader().setUniform1f("uWidth", m_particleBounds.x);
	m_particleSim.getShader().setUniform1f("uHeight", m_particleBounds.y);
	m_particleSim.getShader().setUniform1f("uDepth", m_particleBounds.z);
	m_particleSim.getShader().setUniform1f("uTime", ofGetElapsedTimef());
	m_particleSim.getShader().setUniform1f("uMinDepth", m_minDepth);
	m_particleSim.getShader().setUniform1f("uMaxDepth", m_maxDepth);
	m_particleSim.getShader().setUniform1i("uNumFftBands", m_soundData->m_numFftBands);
	m_particleSim.getShader().setUniform1fv("uFft", &m_soundData->m_fftSmoothed[0], m_soundData->m_numFftBands);
	m_particleSim.updateAndEnd();
}

void ParticleExplosion::reset()
{
	m_particleSim.reset();
}
