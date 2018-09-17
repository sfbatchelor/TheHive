#include "stdafx.h"
#include "Content.h"
#include "layers\ParticleExplosion.h"

Content::Content():
	m_bloomActive(true),
	m_dofPass("Depth Of Field", ofGetWidth(), ofGetHeight()),
	m_bloomPass("Bloom", ofGetWidth(), ofGetHeight()),
	m_soundData(new Fft())
{
	ofSetFrameRate(30);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(true);
	ofSetWindowTitle("The Hive");
	m_snapshot = false;
	m_showGui = true;

	m_soundPlayer.load("theHive.mp3");
	m_soundPlayer.setLoop(true);
	//m_soundPlayer.play();
	m_soundPlayer.setPosition(.3);

	m_pause = false;
	m_restart = false;
	resetFbo();

	ofEnableDepthTest();
	ofSetBackgroundColor(10, 10, 10);
	m_dofPass.reset();
	m_bloomPass.reset();

	m_layers.push_back(std::shared_ptr<ParticleExplosion>(new ParticleExplosion(m_soundData)));
}


void Content::update()
{
	m_dofPass.update(m_fbo->getTexture(0), m_fbo->getDepthTexture());
	m_bloomPass.update(m_fbo->getTexture(0), m_fbo->getDepthTexture());
	ofSoundUpdate();

	//  grab the fft, and put in into a "smoothed" array,
	//	by taking maximums, as peaks and then smoothing downward
	float * val = ofSoundGetSpectrum(m_soundData->m_numFftBands);		// request 128 values for fft
	for (int i = 0; i < m_soundData->m_numFftBands; i++) {
		// let the smoothed value sink to zero:
		m_soundData->m_fftSmoothed[i] *= 0.96f;
		// take the max, either the smoothed or the incoming:
		if (m_soundData->m_fftSmoothed[i] < val[i]) m_soundData->m_fftSmoothed[i] = val[i];
	}

	if (m_restart)
	{
		resetLayers();
		resetFbo();
		m_restart = false;
	}

	updateLayers();


	if (m_bloomActive && m_fbo)
	{
		m_fbo->begin();
		m_fbo->activateAllDrawBuffers();
		m_fbo->clearDepthBuffer(10000);
		glEnable(GL_DEPTH_TEST);  
		ofClear(0, 0, 0, 255);
		drawScene();
		m_fbo->end();
	}
}

void Content::drawScene()
{
	drawLayers();
}

void Content::draw()
{
	///// WORLD
	if (m_bloomActive && m_fbo)
	{
		//m_dofPass.draw(0, 0);
		m_bloomPass.draw(0, 0);
	}
	else
	{
		drawScene();
	}

	/// SCREEN GRAB
	if (m_snapshot == true) {
		m_screenGrab.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
		string fileName = "screenshots\\snapshot_" + ofGetTimestampString() + ".png";
		m_screenGrab.save(fileName);
		m_snapshot = false;
	}

	///// GUI
	if (m_showGui)
	{

		stringstream ss;
		ss << "FPS: " << ofToString(ofGetFrameRate(), 0) << endl << endl;
		ss << "--CONTROLS--" << endl ;
		ss << "'R' TO RESET POINTS" << endl;
		ss << "'F' TO DRAW TO FBO" << endl;
		ss << "' ' TO PAUSE" << endl;
		const std::string string = ss.str();
		ofDrawBitmapStringHighlight(string, glm::vec2(20, 100));
		drawInteractionArea();

		if (m_pause) // draw a pause symbol
		{

			ofSetColor(255);
			ofDrawRectangle(glm::vec2(ofGetWidth() - 100, ofGetHeight() - 200), 35, 80);
			ofDrawRectangle(glm::vec2(ofGetWidth() - 150, ofGetHeight() - 200), 35, 80);
		}
		else
		{
			ofSetColor(0,255, 0);
			ofDrawTriangle(glm::vec2(ofGetWidth() - 150, ofGetHeight() - 200), glm::vec2(ofGetWidth() - 150, ofGetHeight() - 120),  glm::vec2(ofGetWidth() - 65, ofGetHeight() - 160));
			ofSetColor(255);
		}
	}


}

void Content::resetFbo()
{
	ofDisableArbTex();
	m_fbo.reset(new ofFbo());
	ofFbo::Settings settings;
	settings.width = ofNextPow2(ofGetWidth());
	settings.height = ofNextPow2(ofGetHeight());
	settings.internalformat = GL_RGB32F;
	settings.numSamples = 0;
	settings.useDepth = true;
	settings.useStencil = true;
	settings.depthStencilAsTexture = true;
	settings.textureTarget = GL_TEXTURE_2D;
	settings.depthStencilInternalFormat = GL_DEPTH_COMPONENT24;
	settings.minFilter = GL_NEAREST;
	settings.maxFilter = GL_NEAREST;
	m_fbo->allocate(settings);
	m_fbo->createAndAttachTexture(GL_RGBA32F, 0);
	m_fbo->createAndAttachTexture(GL_RGBA32F, 1);
	m_fbo->begin();
	m_fbo->activateAllDrawBuffers();
	ofClear(0, 0, 0, 255);
	m_fbo->end();
	m_fbo->getTexture(0).getTextureData().bFlipTexture = true;
	m_fbo->getTexture(1).getTextureData().bFlipTexture = true;



	m_dofPass.reset(ofGetWidth(), ofGetHeight());
	m_bloomPass.reset(ofGetWidth(), ofGetHeight());
}

void Content::drawLayers()
{
	for (auto layer : m_layers)
	{
		layer->draw();
	}
}

void Content::updateLayers()
{
	for (auto layer : m_layers)
	{
		layer->update();
	}
}

void Content::resetLayers()
{
	for (auto layer : m_layers)
	{
		layer->reset();
	}
}

void Content::setPlayLayers(bool playState)
{
	for (auto layer : m_layers)
	{
		layer->setPlay(playState);
	}
}

void Content::drawInteractionArea()
{
	ofRectangle vp = ofGetCurrentViewport();
	float r = std::min<float>(vp.width, vp.height) * 0.5f;
	float x = vp.width * 0.5f;
	float y = vp.height * 0.5f;

	ofPushStyle();
	ofSetLineWidth(3);
	ofSetColor(255, 255, 0, 100);
	ofNoFill();
	glDepthMask(false);
	ofSetCircleResolution(64);
	ofDrawCircle(x, y, r);
	glDepthMask(true);
	ofSetColor(255);
	ofPopStyle();
}

void Content::exit()
{
}


void Content::keyPressed(int key)
{
	switch (key) {
	case 'x':
		m_snapshot = true;
		break;
	case 'h':
		m_showGui = !m_showGui;
		break;
	case ' ':
		setPlayLayers(m_pause);
		m_pause = !m_pause;
		break;
	case 'r':
		m_restart = true;
		break;
	case '1':
		m_bloomActive = !m_bloomActive;
		break;
	}
}

void Content::keyReleased(int key)
{
}

void Content::mouseMoved(int x, int y)
{
}

void Content::mouseDragged(int x, int y, int button)
{
}

void Content::mousePressed(int x, int y, int button)
{
}

void Content::mouseReleased(int x, int y, int button)
{
}

void Content::mouseEntered(int x, int y)
{
}

void Content::mouseExited(int x, int y)
{
}

void Content::windowResized(int w, int h)
{
}

void Content::dragEvent(ofDragInfo info)
{
	//if (info.files.size() > 0) {

	//	m_draggedImages.assign(info.files.size(), ofImage());
	//	for (unsigned int k = 0; k < info.files.size(); k++) {
	//		m_draggedImages[k].load(info.files[k]);
	//	}
	//	m_plane.mapTexCoordsFromTexture(m_draggedImages[0].getTexture());
	//}
}

void Content::gotMessage(ofMessage msg)
{
}

bool Content::isValid()
{
	return true;
}
