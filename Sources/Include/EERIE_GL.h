#ifndef EERIE_GL_H_
#define EERIE_GL_H_

#ifndef ARX_OPENGL
#define ARX_OPENGL
#endif

#define GLEW_STATIC
#include <GL/glew.h>

#include <cassert>

inline void glCheckError()
{
	GLuint error = glGetError();
	assert(error == GL_NO_ERROR);
}

#endif