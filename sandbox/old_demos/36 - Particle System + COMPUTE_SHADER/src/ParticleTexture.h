#ifndef _PARTICLETEXTURE_
#define _PARTICLETEXTURE_

#ifndef _GLEW_
#define _GLEW_
	#include <GL\glew.h>
	#include <GLFW\glfw3.h>
#endif

#include <stdexcept>
#include <string>

#include "tga.h"

struct ParticleTexture
{
	GLuint	_textureID;
	GLint	_uniTexture;

	void deleteTexture();

	~ParticleTexture()
		{ deleteTexture(); };

	void loadTexture(const std::string& path);
	void useTexture(const GLuint shaderProgramID);

};

#endif