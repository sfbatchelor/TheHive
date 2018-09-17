#pragma once
#include "AudioGraphicsLayer.h"
#include "3d\ParticleSimulation.h"

class ParticleExplosion : public AudioGraphicsLayer
{

public:

	ParticleExplosion(std::shared_ptr<Fft> soundData);
	~ParticleExplosion();
	virtual void draw();
	virtual void update();
	virtual void reset();

protected:

	std::vector<GpuParticle> m_points;
	ParticleSimulation m_particleSim;
	ofTexture m_texture; //used for image based explosion - todo get rid of
	ShaderWatcher m_constantShader;
	ofEasyCam m_cam;
	float m_minDepth;
	float m_maxDepth;
	int m_numPoints;
	glm::vec3 m_particleBounds;

};
