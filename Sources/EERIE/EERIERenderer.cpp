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

void EERIERenderer::DrawIndexedPrim(LPVOID lpvVertices, DWORD dwVertexCount, unsigned short* indices, DWORD idxCount, TextureContainer* tex)
{

}

void EERIERenderer::DrawPrim(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io)
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

void EERIERendererGL::DrawIndexedPrim(LPVOID lpvVertices, DWORD dwVertexCount, unsigned short* indices, DWORD idxCount, TextureContainer* tex)
{
	D3DTLVERTEX* vertices = static_cast<D3DTLVERTEX*>(lpvVertices);

	static GLuint vertexbuffer = -1;
	static GLuint uvbuffer = -1;
	static GLuint glIdxBuffer = -1;

	if (vertexbuffer == -1)
	{
		//LEAK
		glGenBuffers(1, &vertexbuffer);
		glGenBuffers(1, &uvbuffer);
		glGenBuffers(1, &glIdxBuffer);
	}

	std::vector<GLfloat> vertexData;
	vertexData.resize(dwVertexCount * 3);
	
	std::vector<GLfloat> uvData;
	uvData.resize(dwVertexCount * 2);

	for (DWORD i = 0; i < dwVertexCount; ++i)
	{
		vertexData[(i * 3) + 0] = vertices[i].sx;
		vertexData[(i * 3) + 1] = vertices[i].sy;
		vertexData[(i * 3) + 2] = vertices[i].sz;

		uvData[(i * 2) + 0] = vertices[i].tu;
		uvData[(i * 2) + 1] = vertices[i].tv;
	}

	GLuint program = EERIEGetGLProgramID("bitmap");
	glUseProgram(program);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData[0]) * vertexData.size(), vertexData.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvData[0]) * uvData.size(), uvData.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	//TODO: store uniforms instead of looking up every frame
	GLuint uniformLocation = glGetUniformLocation(program, "texsampler");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex->textureID);
	glUniform1i(uniformLocation, 0);

	GLuint projUniform = glGetUniformLocation(program, "proj");
	glm::mat4 proj = glm::ortho(0.0f, (float)DANAESIZX, (float)DANAESIZY, 0.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(projUniform, 1, GL_FALSE, &proj[0][0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIdxBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idxCount, indices, GL_DYNAMIC_DRAW);

	glDrawElements(GL_TRIANGLE_STRIP, idxCount, GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void EERIERendererGL::DrawPrim(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io)
{
	GLenum type = GL_TRIANGLES;

	EERIE_VERTEX* vtxArray = static_cast<EERIE_VERTEX*>(lpvVertices);
	std::vector<GLfloat> vtx;

	//TODO: only update vertex buffers when necessary, not every tick.
	{
		//LEAK
		if(eobj->glVtxBuffer == 0)
			glGenBuffers(1, &eobj->glVtxBuffer);
	}

	GLuint program = EERIEGetGLProgramID("poly");
	glUseProgram(program);

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(EERIE_PRIM_RESTART_IDX);

	if (io)
	{
		//vertexlist3 has already been put in to world space for us, so just pass in an identity matrix.
		glm::mat4 modelMatrix = glm::mat4();
		modelMatrix[1][1] *= -1; //flip the Y-axis for characters
		glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
	}

	std::vector<GLuint> indices;
	std::vector<GLfloat> uv;
	std::vector<GLint> vtxTexIds;

	std::unordered_map<short, short>& texMapBindings = _texMapBindings[eobj];

	const bool genUVs = (eobj->glUvBuffer == 0);
	const bool genIdx = (eobj->glIdxBuffer == 0);
	const bool genTexIds = (eobj->glTexIdBuffer == 0);

	
	if (eobj->glUvBuffer == 0)
	{
		glGenBuffers(1, &eobj->glUvBuffer);
	}

	if (eobj->glIdxBuffer == 0)
	{
		glGenBuffers(1, &eobj->glIdxBuffer);
	}

	//if (genUVs || genIdx)
	{
		
		short binding = 0;


		//TODO: huge amount of data duplication here caused by how facelist handles UVs
		for (long i = 0; i < eobj->nbfaces; ++i)
		{
			EERIE_FACE face = eobj->facelist[i];

			unsigned short fv;
			for (int i = 0; i < 3; ++i)
			{
				fv = face.vid[i];

				if(genIdx)
					indices.push_back(fv);

				if (genUVs)
				{
					uv.push_back(face.u[i]);
					uv.push_back(face.v[i]);
				}

				vtx.push_back(vtxArray[fv].v.x);
				vtx.push_back(vtxArray[fv].v.y);
				vtx.push_back(vtxArray[fv].v.z);
			}

			if (genTexIds)
			{
				if (face.texid != -1 && texMapBindings.find(face.texid) == texMapBindings.end())
				{
					texMapBindings[face.texid] = binding;
					binding++;
				}

				short b = (face.texid == -1) ? -1 : texMapBindings[face.texid];
				vtxTexIds.push_back(b);
				vtxTexIds.push_back(b);
				vtxTexIds.push_back(b);
			}
		}

		if (genUVs)
		{
			glBindBuffer(GL_ARRAY_BUFFER, eobj->glUvBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(uv[0]) * uv.size(), uv.data(), GL_STATIC_DRAW);
		}

		if (genIdx)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eobj->glIdxBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
		}

		if (eobj->glTexIdBuffer == 0)
		{
			glGenBuffers(1, &eobj->glTexIdBuffer);

			glBindBuffer(GL_ARRAY_BUFFER, eobj->glTexIdBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vtxTexIds[0]) * vtxTexIds.size(), vtxTexIds.data(), GL_STATIC_DRAW);
		}
	}

	GLuint uniformLocation = glGetUniformLocation(program, "texsampler");
	for (auto& pair : texMapBindings)
	{
		if (pair.first == -1) continue;

		char buf[32] = { '\0' };
		snprintf(buf, 32, "texsampler[%d]", pair.second);

		GLuint uniformLocation = glGetUniformLocation(program, buf);
		glActiveTexture(GL_TEXTURE0 + pair.second);
		glBindTexture(GL_TEXTURE_2D, eobj->texturecontainer[pair.first]->textureID);
		glUniform1i(uniformLocation, 0 + pair.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, eobj->glVtxBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtx[0]) * vtx.size(), vtx.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, eobj->glVtxBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, eobj->glUvBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, eobj->glTexIdBuffer);
	glVertexAttribIPointer(2, 1, GL_INT, 0, 0);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eobj->glIdxBuffer);
	//glDrawElements(type, eobj->nbfaces * 3, GL_UNSIGNED_INT, 0);

	glDrawArrays(type, 0, eobj->nbfaces * 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}


void EERIERendererGL::DrawRoom(EERIE_ROOM_DATA* room)
{
	//TODO: a lot of code here shared with DrawPrim, can be refactored out.

	GLenum type = GL_TRIANGLES;

	DWORD dwVertexCount = room->nb_vertices;

	GLuint program = EERIEGetGLProgramID("poly");
	glUseProgram(program);

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(EERIE_PRIM_RESTART_IDX);

	std::unordered_map<short, short>& texMapBindings = _texMapBindings[room];

	if (room->glVtxBuffer == 0)
	{
		glGenBuffers(1, &room->glVtxBuffer);

		std::vector<GLfloat> vtx;

		//HACK. Can be sped up.

		for (DWORD i = 0; i < dwVertexCount; ++i)
		{
			vtx.push_back(room->pVtxBuffer[i].x);
			vtx.push_back(room->pVtxBuffer[i].y);
			vtx.push_back(room->pVtxBuffer[i].z);
		}

		glBindBuffer(GL_ARRAY_BUFFER, room->glVtxBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vtx[0]) * vtx.size(), vtx.data(), GL_STATIC_DRAW);
	}

	glm::mat4 modelMatrix = glm::mat4();
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &modelMatrix[0][0]);

	std::vector<GLuint> indices;
	std::vector<GLfloat> uv;
	std::vector<GLint> vtxTexIds;
	vtxTexIds.resize(dwVertexCount);
	
	const bool genUVs = (room->glUvBuffer == 0);


	//TODO: update the index buffer if it has changed.
	const bool genIdx = (room->glIdxBuffer == 0);


	if (room->glUvBuffer == 0)
	{
		glGenBuffers(1, &room->glUvBuffer);

		uv.resize(dwVertexCount * 2);
	}

	if (room->glIdxBuffer == 0)
	{
		glGenBuffers(1, &room->glIdxBuffer);
	}

	if (genUVs || genIdx)
	{

		short binding = 0;

		for (long i = 0; i < dwVertexCount; ++i)
		{
			uv[i * 2] = room->pVtxBuffer[i].tu;
			uv[i * 2 + 1] = room->pVtxBuffer[i].tv;

			if (room->pVtxBuffer[i].color != -1 && texMapBindings.find(room->pVtxBuffer[i].color) == texMapBindings.end())
			{
				texMapBindings[room->pVtxBuffer[i].color] = binding;
				binding++;
			}

			short b = (room->pVtxBuffer[i].color == -1) ? -1 : texMapBindings[room->pVtxBuffer[i].color];
			vtxTexIds[i] = b;
		}

		if (genUVs)
		{
			glBindBuffer(GL_ARRAY_BUFFER, room->glUvBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(uv[0]) * uv.size(), uv.data(), GL_STATIC_DRAW);
		}

		if (genIdx)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, room->glIdxBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * room->nb_indices, room->pussIndice, GL_STATIC_DRAW);
		}

		if (room->glTexIdBuffer == 0)
		{
			glGenBuffers(1, &room->glTexIdBuffer);

			glBindBuffer(GL_ARRAY_BUFFER, room->glTexIdBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vtxTexIds[0]) * vtxTexIds.size(), vtxTexIds.data(), GL_STATIC_DRAW);
		}
	}

	GLuint uniformLocation = glGetUniformLocation(program, "texsampler");
	for (DWORD i = 0; i < room->usNbTextures; ++i)
	{

		char buf[32] = { '\0' };
		snprintf(buf, 32, "texsampler[%d]", i);

		GLuint uniformLocation = glGetUniformLocation(program, buf);
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, room->ppTextureContainer[i]->textureID);
		glUniform1i(uniformLocation, i);
	}

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, room->glVtxBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, room->glUvBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, room->glTexIdBuffer);
	glVertexAttribIPointer(2, 1, GL_INT, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, room->glIdxBuffer);

	glDrawElements(type, room->nb_indices, GL_UNSIGNED_SHORT, 0);
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glUseProgram(0);
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

void EERIERendererD3D7::DrawPrim(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io)
{

}
