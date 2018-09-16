#pragma once
#include "ofMain.h"
#include "utils\ShaderWatcher.h"
#include "3d\ParticleSimulation.h"
#include "3d\DepthOfField.h"
#include "3d\Bloom.h"

struct Point
{
	ofVec4f m_pos;
	ofFloatColor m_col;
	ofVec4f m_vel;
};

class Content {

public:

	Content();

	void update();
	void draw();

	void drawScene();
	void drawInteractionArea();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	bool isValid();

	void resetFbo();

private:

	ofImage m_screenGrab;
	bool m_snapshot;

	std::shared_ptr<ofFbo> m_fbo;

	DepthOfField m_dofPass;
	Bloom m_bloomPass;
	bool m_bloomActive;

	ofEasyCam m_cam;
	bool m_showGui;

	ShaderWatcher m_imageShader;
	ShaderWatcher m_constantShader;

	ofMesh m_mesh;
	ofImage m_image;
	ofTexture m_texture;


	ofSoundPlayer m_soundPlayer;
	static constexpr size_t m_numFftBands = 512;
	std::array<float, m_numFftBands> m_fftSmoothed{ {0} };


	std::vector<GpuParticle> m_points;
	ParticleSimulation m_particleSim;


	int m_numPoints;
	float m_minDepth;
	float m_maxDepth;

	bool m_pause;
	bool m_restart;

	glm::vec3 m_particleBounds;


};