#pragma once
#include <ofFbo.h>
#include <string>


class PostProcess
{
public:

	PostProcess(std::string stringName, int width, int height, int finalBufferFormat = GL_RGBA);
	~PostProcess();

	virtual void draw(float x, float y);
	virtual void update(ofTexture& sourceTex) = 0;
	virtual void reset(int width = 0, int height = 0);

	std::string getName();
	ofTexture& getTexture();


protected:

	void checkAndInitialize();

	bool m_isInitialized;
	std::string m_stringName;
	std::shared_ptr<ofFbo> m_finalBuffer;
	ofPlanePrimitive m_plane;
	int m_finalBufferFormat;
	int m_width;
	int m_height;



};
