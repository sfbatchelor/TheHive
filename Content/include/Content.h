#pragma once
#include "ofMain.h"
#include "utils\ShaderWatcher.h"
#include "3d\ParticleSimulation.h"
#include "3d\DepthOfField.h"
#include "3d\Bloom.h"
#include "layers\Layer.h"
#include "utils\Fft.h"

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

	void drawLayers();
	void updateLayers();
	void resetLayers();
	void setPlayLayers(bool playState);

private:

	ofImage m_screenGrab;
	bool m_snapshot;
	std::shared_ptr<ofFbo> m_fbo;
	DepthOfField m_dofPass;
	Bloom m_bloomPass;
	bool m_bloomActive;

	bool m_showGui;

	std::vector<std::shared_ptr<Layer>> m_layers;

	ofSoundPlayer m_soundPlayer;
	std::shared_ptr<Fft> m_soundData;


	bool m_pause;
	bool m_restart;

};