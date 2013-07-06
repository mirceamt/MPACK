#include "Animation.hpp"

Animation::Animation(GLuint startFrame, GLuint endFrame, GLfloat framesPerSecond, bool loop, bool backward)
	: startFrame(startFrame),endFrame(endFrame),framesPerSecond(framesPerSecond),loop(loop),backward(backward)
{
}
