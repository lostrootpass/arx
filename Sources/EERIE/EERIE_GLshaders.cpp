#include "EERIE_GLshaders.h"
#include "EERIEapp.h"

#include <fstream>
#include <unordered_map>

#undef max //win32 defines this but it messes with std::numeric_limits

std::unordered_map<const char*, GLuint> programCache;

GLuint getIdForShader(const std::string& shaderName, GLenum shaderType)
{
	std::ifstream shaderText(shaderName, std::ios::binary | std::ios::in);

	if(!shaderText)
		return -1;

	std::string srcString{std::istreambuf_iterator<char>{shaderText},{}};

	GLuint shaderID = glCreateShader(shaderType);

	const char* src = srcString.c_str();
	glShaderSource(shaderID, 1, &src, nullptr);
	glCompileShader(shaderID);

	GLint status;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);

	GLint logLength;
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);

	char error[256] = {'\0'};
	glGetShaderInfoLog(shaderID, logLength, NULL, error);

	return shaderID;
}

GLuint EERIEGetGLProgramID(const char* shaderName)
{
	if(programCache.find(shaderName) != programCache.end())
	{
		return programCache[shaderName];
	}

	std::string baseName = Project.workingdir;
	baseName += "Graph/shaders/" + std::string(shaderName);

	GLuint vertexShaderID = getIdForShader(baseName + ".vert", GL_VERTEX_SHADER);
	GLuint fragShaderID = getIdForShader(baseName + ".frag", GL_FRAGMENT_SHADER);

	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragShaderID);
	glLinkProgram(programID);

	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragShaderID);
	
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragShaderID);

	programCache[shaderName] = programID;
	return programID;
}