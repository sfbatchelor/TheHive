#include "stdafx.h"
#include "Content.h"

Content::Content():
	m_particleBounds(glm::vec3(10000.)),
	m_bloomActive(true)
{
	ofSetFrameRate(30);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(true);
	ofSetWindowTitle("The Hive");
	m_snapshot = false;
	m_showGui = true;
	m_numPoints = 1024 * 38;
	m_constantShader.load("constantVert.glsl", "constantFrag.glsl", "sphereGeom.glsl");
	m_imageShader.load("imageVert.glsl", "imageFrag.glsl");
	m_gaussianShader.load("gaussianVert.glsl", "gaussianFrag.glsl");
	m_bloomFinalShader.load("bloomFinalVert.glsl", "bloomFinalFrag.glsl");
	m_dofFinalShader.load("dofFinalVert.glsl", "dofFinalFrag.glsl");


	m_soundPlayer.load("theHive.mp3");
	m_soundPlayer.setLoop(true);
	//m_soundPlayer.play();
	m_soundPlayer.setPosition(.3);

	m_pause = false;
	m_restart = false;

	m_minDepth = -300.;
	m_maxDepth = 300.;


	// GENERATE POINTS FROM IMAGE
	// load an image from disk
	m_particleSim.loadCompute("compute.glsl");
	m_image.load("paint1.png");
	m_texture.allocate(m_image.getPixels());
	m_points = GpuParticleFactory::fromImage(m_image, m_numPoints, m_minDepth, m_maxDepth);
	m_particleSim.loadParticles(m_points);

	resetFbo();

	// SETUP RAY BUFFER ON GPU
	m_cam.setVFlip(true); //flip for upside down image
	m_cam.setFarClip(100000000.);

	ofEnableDepthTest();
	ofSetBackgroundColor(10, 10, 10);


}


void Content::update()
{
	m_imageShader.update();
	m_constantShader.update();
	m_gaussianShader.update();
	m_bloomFinalShader.update();
	m_dofFinalShader.update();

	ofSoundUpdate();

	//  grab the fft, and put in into a "smoothed" array,
	//	by taking maximums, as peaks and then smoothing downward
	float * val = ofSoundGetSpectrum(m_numFftBands);		// request 128 values for fft
	for (int i = 0; i < m_numFftBands; i++) {
		// let the smoothed value sink to zero:
		m_fftSmoothed[i] *= 0.96f;
		// take the max, either the smoothed or the incoming:
		if (m_fftSmoothed[i] < val[i]) m_fftSmoothed[i] = val[i];
	}

	if (m_restart)
	{
		m_particleSim.reset();
		resetFbo();
		m_restart = false;
	}

	if (!m_pause)
	{
		m_particleSim.begin();
		m_texture.bindAsImage(0, GL_READ_ONLY);
		m_particleSim.getShader().setUniform1i("uNumPointsSF", m_numPoints / 1024);
		m_particleSim.getShader().setUniform1f("uWidth", m_particleBounds.x);
		m_particleSim.getShader().setUniform1f("uHeight", m_particleBounds.y);
		m_particleSim.getShader().setUniform1f("uDepth", m_particleBounds.z);
		m_particleSim.getShader().setUniform1f("uTime", ofGetElapsedTimef());
		m_particleSim.getShader().setUniform1f("uMinDepth", m_minDepth);
		m_particleSim.getShader().setUniform1f("uMaxDepth", m_maxDepth);
		m_particleSim.getShader().setUniform1i("uNumFftBands", m_numFftBands);
		m_particleSim.getShader().setUniform1fv("uFft", &m_fftSmoothed[0], m_numFftBands);
		m_particleSim.getShader().dispatchCompute((m_points.size() + 1024 - 1) / 1024, 1, 1);
		m_particleSim.updateAndEnd();
	}



	if (m_bloomActive && m_fbo)
	{
		m_fbo->begin();
		m_fbo->activateAllDrawBuffers();
		m_fbo->clearDepthBuffer(10000);
		glEnable(GL_DEPTH_TEST);  
		ofClear(0, 0, 0, 255);
		drawScene();
		m_fbo->end();

		//drawBloom();
	}
}

void Content::drawScene()
{
	m_cam.begin();
	ofPushMatrix();
	ofTranslate(-m_image.getWidth() / 2, -m_image.getHeight() / 2);
	ofEnableAlphaBlending();
	ofSetColor(255);
	m_constantShader.getShader().begin();
	m_constantShader.getShader().setUniform1i("uNumFftBands", m_numFftBands);
	m_constantShader.getShader().setUniform1fv("uFft", &m_fftSmoothed[0], m_numFftBands);
	m_constantShader.getShader().setUniform1f("uTime", ofGetElapsedTimef());
	m_constantShader.getShader().setUniformMatrix4f("modelView", m_cam.getModelViewMatrix());
	glPointSize(3);
	m_constantShader.getShader().setUniform1f("uAlpha", 1.f);
	m_particleSim.draw(GL_POINTS);
	m_constantShader.getShader().end();
	ofPopMatrix();
	m_cam.end();
}

void Content::draw()
{
	///// WORLD
	if (m_bloomActive && m_fbo)
	{
		drawBloom(m_fbo->getTexture(0), m_fbo->getTexture(1));
//		drawDOF(m_fbo->getTexture(0));
//		m_dofFinal->getTexture().draw(0, 0);
		m_bloomFinal->getTexture().draw(0, 0);
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

	if (m_showGui)
	{
		m_cam.begin();

		ofSetColor(255, 100);
		ofDrawGrid(5000, 5, true, true, true, true);
		m_cam.end();
	}


	///// GUI
	if (m_showGui)
	{

		m_cam.begin();
		ofNoFill();
		ofSetColor(255);
		ofDrawBox(0, 0, 0, m_particleBounds.x, m_particleBounds.y, m_particleBounds.z);
		ofFill();
		m_cam.end();

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
	//settings.wrapModeHorizontal = GL_CLAMP_TO_BORDER;
	//settings.wrapModeVertical = GL_CLAMP_TO_BORDER;
	m_fbo->allocate(settings);
	m_fbo->createAndAttachTexture(GL_RGBA32F, 0);
	m_fbo->createAndAttachTexture(GL_RGBA32F, 1);
	m_fbo->begin();
	m_fbo->activateAllDrawBuffers();
	ofClear(0, 0, 0, 255);
	m_fbo->end();
	m_fbo->getTexture(0).getTextureData().bFlipTexture = true;
	m_fbo->getTexture(1).getTextureData().bFlipTexture = true;


	m_gaussianFront.reset(new ofFbo());
	m_gaussianFront->allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	m_gaussianBack.reset(new ofFbo());
	m_gaussianBack->allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	m_bloomFinal.reset(new ofFbo());
	m_bloomFinal->allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	m_bloomFinal->getTexture().getTextureData().bFlipTexture = true;
	m_dofFinal.reset(new ofFbo());
	m_dofFinal->allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	m_dofFinal->getTexture().getTextureData().bFlipTexture = true;

	// unit quad with normalized texels
	m_plane.set(2. * (ofGetWidth() / ofGetHeight()), 2, 10, 10);
	m_plane.mapTexCoords(0, 0, 1., 1.);
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

void Content::drawBloom( ofTexture& sceneTexture, ofTexture& highlightsTexture)
{

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
			highlightsTexture.bind(0);
		else if (horizontal)
			m_gaussianFront->getTexture().bind(0);
		else
			m_gaussianBack->getTexture().bind(0);

		ofSetColor(255);
		m_plane.enableTextures();
		ofClear(0, 0, 0, 255);
		m_plane.draw();

		if (firstIteration)
			highlightsTexture.unbind(0);
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


	if (m_bloomFinal)
	{
		//FINAL BLEND PASS
		m_bloomFinal->begin();
		m_bloomFinalShader.getShader().begin();
		m_bloomFinalShader.getShader().setUniformTexture("scene", sceneTexture, 5);
		m_bloomFinalShader.getShader().setUniformTexture("bloomBlur", m_gaussianFront->getTexture(), 6);
		m_bloomFinalShader.getShader().setUniformTexture("depth", m_fbo->getDepthTexture(), 7);
		ofSetColor(255);
		m_plane.enableTextures();
		ofClear(0, 0, 0, 255);
		m_plane.draw();
		m_fbo->getTexture(0).unbind(5);
		m_gaussianFront->getTexture().unbind(6);
		m_bloomFinalShader.getShader().end();
		m_bloomFinal->end();
	}

}

void Content::drawDOF( ofTexture& sceneTexture)
{

	//GAUSSIAN BLUR PASS
	//first pass using the brightness fbo result, then it switches between vertical and horizontal blur passes ping-pong style
	bool horizontal = true, firstIteration = true;
	int amount = 50;

	m_gaussianShader.getShader().begin();
	for (unsigned int i = 0; i < amount; i++)
	{
		m_gaussianShader.getShader().setUniform1i("horizontal", horizontal);
		m_gaussianShader.getShader().setUniformTexture("depth", m_fbo->getDepthTexture(), 5);
		m_gaussianShader.getShader().setUniform1i("DOF", true);
		if (horizontal)
			m_gaussianBack->begin();
		else
			m_gaussianFront->begin();

		if (firstIteration)
			sceneTexture.bind(0);
		else if (horizontal)
			m_gaussianFront->getTexture().bind(0);
		else
			m_gaussianBack->getTexture().bind(0);

		ofSetColor(255);
		m_plane.enableTextures();
		ofClear(0, 0, 0, 255);
		m_plane.draw();

		if (firstIteration)
			sceneTexture.unbind(0);
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


	if (m_dofFinal)
	{
		//FINAL BLEND PASS
		m_dofFinal->begin();
		m_dofFinalShader.getShader().begin();
		m_dofFinalShader.getShader().setUniformTexture("scene", sceneTexture, 5);
		m_dofFinalShader.getShader().setUniformTexture("bloomBlur", m_gaussianFront->getTexture(), 6);
		m_dofFinalShader.getShader().setUniformTexture("depth", m_fbo->getDepthTexture(), 7);
		ofSetColor(255);
		m_plane.enableTextures();
		ofClear(0, 0, 0, 255);
		m_plane.draw();
		m_fbo->getTexture(0).unbind(5);
		m_gaussianFront->getTexture().unbind(6);
		m_dofFinalShader.getShader().end();
		m_dofFinal->end();
	}

}

void Content::exit()
{
	m_imageShader.exit();
	m_constantShader.exit();
	m_gaussianShader.exit();
	m_bloomFinalShader.exit();
	m_dofFinalShader.exit();
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
		m_particleSim.setPlay(m_pause);
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
