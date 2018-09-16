#pragma once
#include "PostProcess.h"
#include "utils\ShaderWatcher.h"


//! Depth of Field post process.
class Bloom : public PostProcess
{
public:

	Bloom(std::string stringName, int width, int height, int finalBufferFormat = GL_RGBA);
	~Bloom();

	virtual void update(ofTexture & sourceTex, ofTexture& depthTex);
	virtual void reset(int width = 0, int height = 0);

protected:

	// ping pong gaussian shaders
	std::shared_ptr<ofFbo> m_gaussianFront;
	std::shared_ptr<ofFbo> m_gaussianBack;

	std::shared_ptr<ofFbo> m_highlightBuffer;

	ShaderWatcher m_gaussianShader;
	ShaderWatcher m_highlightShader;
	ShaderWatcher m_bloomFinalShader;
};

