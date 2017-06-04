#include "EERIERenderer.h"

#include "EERIE_GL.h"
#include "EERIE_GLshaders.h"
#include "EERIEmath.h"

#include "Danae.h"
#include "ARX_Cedric.h"

#include <glm/gtc/matrix_transform.hpp>

extern INTERACTIVE_OBJ * DESTROYED_DURING_RENDERING;
extern long USE_CEDRIC_ANIM;

void PrepareAnim(EERIE_3DOBJ * eobj, ANIM_USE * eanim, unsigned long time, INTERACTIVE_OBJ * io);

void EERIERenderer::DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ)
{

}

void EERIERenderer::DrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex)
{

}

void EERIERenderer::DrawPrim(TextureContainer* tex, LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io)
{

}


/************************************************************/
/*							GL								*/
/************************************************************/

void EERIERendererGL::DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ)
{
	if ((io)
		&& (io != inter.iobj[0]))
	{
		float speedfactor = io->basespeed + io->speed_modif;

		if (speedfactor < 0) speedfactor = 0;

		float tim = (float)time*(speedfactor);

		if (tim <= 0.f) time = 0;
		else time = (unsigned long)tim;

		io->frameloss += tim - time;

		if (io->frameloss > 1.f) // recover lost time...
		{
			long tt;
			F2L(io->frameloss, &tt);
			io->frameloss -= tt;
			time += tt;
		}
	}

	if (time <= 0) goto suite;

	if (time > 200) time = 200; // TO REMOVE !!!!!!!!!

	PrepareAnim(eobj, eanim, time, io);

	if (io)
		for (long count = 1; count < MAX_ANIM_LAYERS; count++)
		{
			ANIM_USE * animuse = &io->animlayer[count];

			if (animuse->cur_anim)
				PrepareAnim(eobj, animuse, time, io);
		}

suite:
	;

	DESTROYED_DURING_RENDERING = NULL;

	if (USE_CEDRIC_ANIM)
		Cedric_AnimateDrawEntityGL(eobj, eanim, angle, pos, io, typ);
}

void EERIERendererGL::DrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex)
{
	//Top-left is (0,0); bottom right is (1,1) - legacy Arx assumptions.

	static const GLfloat uvData[] = {
		0.0f, 0.0f,	//Top Left
		1.0f, 0.0f,	//Top Right
		0.0f, 1.0f,	//Bot Left
		1.0f, 1.0f	//Bot Right
	};

	static GLuint vertexbuffer = -1;
	static GLuint uvbuffer = -1;

	if (vertexbuffer == -1)
	{
		//LEAK
		glGenBuffers(1, &vertexbuffer);

		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uvData), uvData, GL_STATIC_DRAW);
	}

	const float w = (float)DANAESIZX, h = (float)DANAESIZY;

	const float startX = (x / w);
	const float startY = (y / h);
	const float dX = (sx / w);
	const float dY = (sy / h);

	GLfloat quadData[] = {
		//Top left
		(startX), (startY), 0.0f,

		//Top right
		(startX + dX), (startY), 0.0f,

		//Bot left
		(startX), (startY + dY), 0.0f,

		//Bot right
		(startX + dX), (startY + dY), 0.0f
	};


	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_DYNAMIC_DRAW);

	GLuint program = EERIEGetGLProgramID("bitmap");
	glUseProgram(program);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	//TODO: store uniforms instead of looking up every frame
	GLuint uniformLocation = glGetUniformLocation(program, "texsampler");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex->textureID);
	glUniform1i(uniformLocation, 0);

	GLuint projUniform = glGetUniformLocation(program, "proj");
	glm::mat4 proj = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(projUniform, 1, GL_FALSE, &proj[0][0]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void EERIERendererGL::DrawPrim(TextureContainer* tex, LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io)
{
	static GLuint vertexbuffer = -1;
	static GLuint uvbuffer = -1;
	static GLuint idxbuffer = -1;

	GLenum type = GL_TRIANGLES;

	if (vertexbuffer == -1)
	{
		//LEAK
		glGenBuffers(1, &vertexbuffer);
		glGenBuffers(1, &uvbuffer);
		glGenBuffers(1, &idxbuffer);
	}

	//TODO: hack
	std::vector<GLfloat> vtx;
	std::vector<GLfloat> uv;
	uv.resize(dwVertexCount * 2);

	EERIE_VERTEX* vtxArray = static_cast<EERIE_VERTEX*>(lpvVertices);

	for (DWORD i = 0; i < dwVertexCount; ++i)
	{
		vtx.push_back(vtxArray[i].v.x);
		vtx.push_back(vtxArray[i].v.y);
		vtx.push_back(vtxArray[i].v.z);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtx[0]) * vtx.size(), vtx.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint program = EERIEGetGLProgramID("poly");

	if (tex)
	{
		GLuint uniformLocation = glGetUniformLocation(program, "texsampler");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex->textureID);
		glUniform1i(uniformLocation, 0);
	}

	if (io)
	{
		glm::mat4 modelMatrix = glm::translate(glm::mat4(), glm::vec3(io->pos.x, -io->pos.y, io->pos.z));
		modelMatrix = glm::rotate(modelMatrix, io->angle.x, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, io->angle.y, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, io->angle.z, glm::vec3(0.0f, 0.0f, 1.0f));

		glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
	}

	if (eobj)
	{
		std::vector<GLuint> indices;

		for (long i = 0; i < eobj->nbfaces; ++i)
		{
			unsigned short v1, v2, v3;
			EERIE_FACE face = eobj->facelist[i];
			v1 = face.vid[0];
			v2 = face.vid[1];
			v3 = face.vid[2];

			indices.push_back(v1);
			indices.push_back(v2);
			indices.push_back(v3);

			uv[v1 * 2] = face.u[0];
			uv[v2 * 2] = face.u[1];
			uv[v3 * 2] = face.u[2];

			uv[v1 * 2 + 1] = face.v[0];
			uv[v2 * 2 + 1] = face.v[1];
			uv[v3 * 2 + 1] = face.v[2];
		}

		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uv[0]) * uv.size(), uv.data(), GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);

		glDrawElements(type, indices.size(), GL_UNSIGNED_INT, 0);
	}
	else
	{
		glDrawArrays(type, 0, dwVertexCount);
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}



/************************************************************/
/*							D3D								*/
/************************************************************/

void EERIERendererD3D7::DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ)
{

}

void EERIERendererD3D7::DrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex)
{

}

void EERIERendererD3D7::DrawPrim(TextureContainer* tex, LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io)
{

}