#pragma once
#include <string>

class Layer
{

public:

	Layer(std::string name);
	~Layer();

	virtual void draw()=0;
	virtual void update()=0;
	virtual void reset()=0;
	void play();
	void pause();
	void setPlay(bool playState);
	std::string getName();

protected:

	bool m_isPlaying;
	std::string m_stringName;


};