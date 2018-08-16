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
	m_bloomFinalShader.load("bloomFinalShader.glsl", "bloomFinalShader.glsl");
	m_compute.load( "compute.glsl");


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
	m_image.load("paint1.png");
	m_texture.allocate(m_image.getPixels());
	m_mesh.setMode(OF_PRIMITIVE_POINTS);
	// loop through the image in the x and y axes
	for(int i = 0; i<m_numPoints; i++)
	{
		int x = ofRandom(float(m_image.getWidth()));
		int y = ofRandom(float(m_image.getHeight()));



		ofColor cur = m_image.getColor(x, y);
		if (cur.a > 0) {
			// the alpha value encodes depth, let's remap it to a good depth range
			float z = ofMap(cur.getBrightness(), 0, 255, m_minDepth, m_maxDepth);
			cur.a = 255;

			m_mesh.addColor(cur);
			// add offset to centre at origin
			x -= m_image.getWidth() / 2; 
			y -= m_image.getHeight() / 2;
			ofVec3f pos(x, y, z);
			m_mesh.addVertex(pos);

			Point point{};
			point.m_col = ofFloatColor(cur.r, cur.g, cur.b, cur.a);
			point.m_pos = ofVec4f(pos.x, pos.y, pos.z, 1.);
			point.m_vel = ofVec4f(0);
			m_points.push_back(point);
		}
	}
	m_pointsBuffer.allocate(m_points, GL_DYNAMIC_DRAW);
	m_pointsBufferOld.allocate(m_points, GL_DYNAMIC_DRAW);
	m_pointsVbo.setVertexBuffer(m_pointsBuffer,4,sizeof(Point));
	m_pointsVbo.setColorBuffer(m_pointsBuffer,sizeof(Point),sizeof(ofVec4f));
	m_pointsVbo.enableColors();
	m_pointsVbo.disableNormals();
	m_pointsVbo.disableIndices();
	m_pointsVbo.disableTexCoords();
	m_pointsBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
	m_pointsBufferOld.bindBase(GL_SHADER_STORAGE_BUFFER, 1);
	m_texture.loadData(m_image.getPixels());


	resetFbo();

	// SETUP RAY BUFFER ON GPU
	m_cam.setVFlip(true); //flip for upside down image
	m_cam.setFarClip(100000000.);

	ofEnableDepthTest();
	ofSetBackgroundColor(10, 10, 10);


}

void Content::initSimPoints()
{
	m_pointsBuffer.unbindBase(GL_SHADER_STORAGE_BUFFER, 0);
	m_pointsBufferOld.allocate(m_points, GL_DYNAMIC_DRAW);
	m_pointsBuffer.allocate(m_points, GL_DYNAMIC_DRAW);
	m_pointsVbo.setVertexBuffer(m_pointsBuffer,4,sizeof(Point));
	m_pointsVbo.setColorBuffer(m_pointsBuffer,sizeof(Point),sizeof(ofVec4f));
	m_pointsBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
	m_pointsBufferOld.bindBase(GL_SHADER_STORAGE_BUFFER, 1);
}

void Content::update()
{
	m_imageShader.update();
	m_constantShader.update();
	m_gaussianShader.update();
	m_bloomFinalShader.update();

	m_compute.update();

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
		initSimPoints();
		resetFbo();
		m_restart = false;
	}

	if (!m_pause)
	{
		m_compute.getShader().begin();		m_texture.bindAsImage(0, GL_READ_ONLY);		m_compute.getShader().setUniform1i("uNumPointsSF", m_numPoints/1024);		m_compute.getShader().setUniform1f("uWidth", m_particleBounds.x);		m_compute.getShader().setUniform1f("uHeight", m_particleBounds.y);		m_compute.getShader().setUniform1f("uDepth", m_particleBounds.z);		m_compute.getShader().setUniform1f("uTime", ofGetElapsedTimef());		m_compute.getShader().setUniform1f("uMinDepth", m_minDepth);		m_compute.getShader().setUniform1f("uMaxDepth", m_maxDepth);		m_compute.getShader().setUniform1i("uNumFftBands", m_numFftBands);		m_compute.getShader().setUniform1fv("uFft", &m_fftSmoothed[0], m_numFftBands);		m_compute.getShader().dispatchCompute((m_points.size() + 1024 - 1) / 1024, 1, 1);		m_compute.getShader().end();
		m_pointsBuffer.copyTo(m_pointsBufferOld);
	}



	if (m_bloomActive && m_fbo)
	{
		m_fbo->begin();		m_fbo->activateAllDrawBuffers();		m_fbo->clearDepthBuffer(10000);
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
	ofScale(1, -1, 1); // flip the y axis and zoom in a bit
	ofEnableAlphaBlending();

	ofPointSmooth();
	ofSetColor(255);
	m_constantShader.getShader().begin();
	m_constantShader.getShader().setUniform1i("uNumFftBands", m_numFftBands);
	m_constantShader.getShader().setUniform1fv("uFft", &m_fftSmoothed[0], m_numFftBands);
	m_constantShader.getShader().setUniform1f("uTime", ofGetElapsedTimef());
	glPointSize(3);
	m_constantShader.getShader().setUniform1f("uAlpha", 1.f);
	m_pointsVbo.draw(GL_POINTS, 0, m_points.size());
	m_constantShader.getShader().end();
	ofPopMatrix();
	m_cam.end();
}

void Content::draw()
{
	///// WORLD
	{
		if (m_bloomActive && m_fbo)		{			drawBloom();		}		else		{			drawScene();		}		/// SCREEN GRAB
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
	settings.wrapModeHorizontal = GL_CLAMP_TO_BORDER;
	settings.wrapModeVertical = GL_CLAMP_TO_BORDER;
	m_fbo->allocate(settings);
	m_fbo->createAndAttachTexture(GL_RGBA32F, 0);
	m_fbo->createAndAttachTexture(GL_RGBA32F, 1);
	m_fbo->begin();
	m_fbo->activateAllDrawBuffers();
	ofClear(0, 0, 0, 255);
	m_fbo->end();


	m_bloomFront.reset(new ofFbo());	m_bloomFront->allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	m_bloomBack.reset(new ofFbo());
	m_bloomBack->allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);


	m_plane.set(ofGetWidth(), ofGetHeight(), 10, 10);
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

void Content::drawBloom()
{
	//first pass using the brightness fbo result, then it switches between vertical and horizontal blur passes.

	bool horizontal = true, firstIteration = true;
	int amount = 1;

	m_gaussianShader.getShader().begin();
	for (unsigned int i = 0; i < amount; i++)
	{
		m_gaussianShader.getShader().setUniform1i("horizontal", horizontal);
		//if (horizontal)
		//	m_bloomBack->begin();
		//else
		//	m_bloomFront->begin();
//		if (firstIteration)
		m_fbo->getTexture(1).bind(0);
		//m_image.bind(0);
		//m_gaussianShader.getShader().setUniformTexture("src", m_fbo->getTexture(1), 1);
		//m_gaussianShader.getShader().setUniformTexture("src", m_image.getTexture(), 1);
		//else if (horizontal)
		//	m_bloomFront->getTexture().bind(0);
		//else
		//	m_bloomBack->getTexture().bind(0);

		m_cam.begin();
		ofSetColor(255);
		m_plane.enableTextures();
		m_plane.draw();
		m_cam.end();

		//m_image.unbind(0);
		m_fbo->getTexture(1).unbind(0);
//		if (firstIteration)
		//else if (horizontal)
		//	m_bloomFront->getTexture().unbind(0);
		//else
		//	m_bloomBack->getTexture().unbind(0);
		//if (horizontal)
		//	m_bloomBack->end();
		//else
		//	m_bloomFront->end();

		horizontal = !horizontal;
		if (firstIteration)
			firstIteration = false;
	}
	m_gaussianShader.getShader().end();
}


void Content::exit()
{
	m_imageShader.exit();
	m_constantShader.exit();
	m_gaussianShader.exit();
	m_bloomFinalShader.exit();
	m_compute.exit();
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
