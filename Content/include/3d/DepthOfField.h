#pragma once
#include "PostProcess.h"
#include "utils\ShaderWatcher.h"


//! Depth of Field post process.
class DepthOfField : public PostProcess
{
public:

	DepthOfField(std::string stringName, int width, int height, int finalBufferFormat = GL_RGBA);
	~DepthOfField();

	virtual void update(ofTexture & sourceTex, ofTexture& depthTex);
	virtual void reset(int width = 0, int height = 0);

protected:

	// ping pong gaussian shaders
	std::shared_ptr<ofFbo> m_gaussianFront;
	std::shared_ptr<ofFbo> m_gaussianBack;

	ShaderWatcher m_gaussianShader;
	ShaderWatcher m_dofFinalShader;
};
