#include "EERIERenderer.h"

#include "EERIE_GL.h"
#include "EERIE_GLshaders.h"
#include "EERIEmath.h"
#include "EERIEAnim.h"
#include "EERIEDraw.h"

#include "Danae.h"
#include "ARX_Cedric.h"
#include "ARX_C_cinematique.h"

#include <glm/gtc/matrix_transform.hpp>

extern INTERACTIVE_OBJ * DESTROYED_DURING_RENDERING;
extern long USE_CEDRIC_ANIM;

void PrepareAnim(EERIE_3DOBJ * eobj, ANIM_USE * eanim, unsigned long time, INTERACTIVE_OBJ * io);

struct VtxAttrib
{
	glm::vec4 color;
	glm::vec3 normal;
	glm::vec2 uv;
	GLint texId;
};

//TODO: move/improve
GLuint quadVAO = -1;
GLuint ioVAO = -1;

extern TILE_LIGHTS tilelights[MAX_BKGX][MAX_BKGZ];

void addLights(std::vector<LightData>& data, EERIE_LIGHT** sources, size_t count)
{
	for (int i = 0; i < count; ++i)
	{
		EERIE_LIGHT* light = sources[i];
		LightData ld;

		if (!light || !light->exist || !light->treat)
		{
			continue;
		}

		ld.pos = glm::vec4(light->pos.x, -light->pos.y, light->pos.z, 0.0f);
		ld.color = glm::vec4(light->rgb.r, light->rgb.g, light->rgb.b, light->intensity);
		ld.fallstart = light->fallstart;
		ld.fallend = light->fallend;
		ld.precalc = light->precalc;

		data.push_back(ld);
	}
}


void EERIERenderer::DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ)
{

}

void EERIERenderer::DrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex, const float* uvs)
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
	D3DCOLOR color = 0L;
	EERIEDrawAnimQuat(0, eobj, eanim, angle, pos, time, io, color, typ);
}

void EERIERendererGL::DrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex, const float* uvs)
{
	const float w = (float)DANAESIZX, h = (float)DANAESIZY;

	const float startX = (x / w);
	const float startY = (y / h);
	const float dX = (sx / w);
	const float dY = (sy / h);

	if (quadVAO == -1)
	{
		glGenVertexArrays(1, &quadVAO);
	}

	glBindVertexArray(quadVAO);

	GLuint program = EERIEGetGLProgramID("bitmap");
	glUseProgram(program);

	//TODO: store uniforms instead of looking up every frame
	GLuint uniformLocation = glGetUniformLocation(program, "texsampler");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex->textureID);
	glUniform1i(uniformLocation, 0);

	_drawQuad(program, startX, startY, dX, dY, uvs);
}

void EERIERendererGL::DrawCinematic(float x, float y, float sx, float sy, float z, TextureContainer * tex, C_LIGHT* light, float LightRND)
{
	const float w = (float)DANAESIZX, h = (float)DANAESIZY;

	const float startX = (x / w);
	const float startY = (y / h);
	const float dX = (sx / w);
	const float dY = (sy / h);

	GLint oldProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);

	if (quadVAO == -1)
	{
		glGenVertexArrays(1, &quadVAO);
	}

	glBindVertexArray(quadVAO);
	
	GLuint program = EERIEGetGLProgramID("cinematic");
	glUseProgram(program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex->textureID);
	glUniform1i(glGetUniformLocation(program, "texsampler"), 0);

	LightData lightData;
	lightData.pos = glm::vec4(light->pos.x / (float)DANAESIZX, light->pos.y / (float)DANAESIZY, light->pos.z, light->intensite);
	lightData.color = glm::vec4(light->r / 255.0f, light->g / 255.0f, light->b / 255.0f, LightRND);
	lightData.fallstart = light->fallin / (float)DANAESIZX;
	lightData.fallend = light->fallout / (float)DANAESIZX;

	static GLuint lightBuffer = -1;
	if (lightBuffer == -1)
		glGenBuffers(1, &lightBuffer);

	glBindBuffer(GL_UNIFORM_BUFFER, lightBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightData), &lightData, GL_DYNAMIC_DRAW);

	GLuint block = glGetUniformBlockIndex(program, "LightData");
	glUniformBlockBinding(program, block, 1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightBuffer);

	_drawQuad(program, startX, startY, dX, dY);

	glUseProgram(oldProgram);
}

void EERIERendererGL::DrawFade(const EERIE_RGB& color, float visibility)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (quadVAO == -1)
	{
		glGenVertexArrays(1, &quadVAO);
	}

	glBindVertexArray(quadVAO);

	GLuint program = EERIEGetGLProgramID("fade");
	glUseProgram(program);

	glm::vec4 colorUniform = glm::vec4(color.r, color.g, color.b, visibility);
	glUniform4fv(glGetUniformLocation(program, "colorUniform"), 1, &colorUniform[0]);

	_drawQuad(program, 0.0f, 0.0f, 1.0f, 1.0f);

	glDisable(GL_BLEND);
}

void EERIERendererGL::DrawPrim(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io)
{
	GLenum type = GL_TRIANGLES;
	bool useAlphaBlending = false;

	EERIE_VERTEX* vtxArray = static_cast<EERIE_VERTEX*>(lpvVertices);
	std::vector<GLfloat> vtx;

	if (ioVAO == -1)
	{
		glGenVertexArrays(1, &ioVAO);
	}

	glBindVertexArray(ioVAO);

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
		glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
	}

	std::vector<GLuint> indices;
	std::vector<VtxAttrib> vtxAttribs;

	std::unordered_map<short, short>& texMapBindings = _texMapBindings[eobj];

	const bool genAttribs = (eobj->glAttribBuffer == 0);
	const bool genIdx = (eobj->glIdxBuffer == 0);

	
	if (eobj->glAttribBuffer == 0)
	{
		glGenBuffers(1, &eobj->glAttribBuffer);
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

			short b = -1;
			if (genAttribs)
			{
				if (face.texid != -1 && texMapBindings.find(face.texid) == texMapBindings.end())
				{
					texMapBindings[face.texid] = binding;
					binding++;
				}

				b = (face.texid == -1) ? -1 : texMapBindings[face.texid];

			}

			unsigned short fv;
			int faceVtxCount = (face.facetype & POLY_QUAD) ? 4 : 3;
			for (int i = 0; i < faceVtxCount; ++i)
			{
				fv = face.vid[i];

				if(genIdx)
					indices.push_back(fv);

				VtxAttrib attrib;

				if (genAttribs)
				{
					attrib.uv.x = face.u[i];
					attrib.uv.y = face.v[i];
				}

				vtx.push_back(vtxArray[fv].v.x);
				vtx.push_back(-vtxArray[fv].v.y);
				vtx.push_back(vtxArray[fv].v.z);

				attrib.texId = b;
				attrib.normal = glm::vec3(vtxArray[fv].norm.x, vtxArray[fv].norm.y, -vtxArray[fv].norm.z);

				float alpha = (vtxArray[fv].vert.color >> 24 & 0xFF) / 255.0f;
				if (face.transval > 0.0f)
				{
					if (!useAlphaBlending)
					{
						useAlphaBlending = true;
					}

					if (face.transval > 1.0f)
					{
						alpha = face.transval - 1.0f;
					}
					else
					{
						alpha = face.transval;
					}
				}

				attrib.color.a = alpha;
				attrib.color.r = (vtxArray[fv].vert.color >> 16 & 0xFF) / 255.0f;
				attrib.color.g = (vtxArray[fv].vert.color >> 8 & 0xFF) / 255.0f;
				attrib.color.b = (vtxArray[fv].vert.color >> 0 & 0xFF) / 255.0f;

				vtxAttribs.push_back(attrib);
			}

			if (faceVtxCount == 4)
			{
				indices.push_back(face.vid[3]);
				indices.push_back(face.vid[2]);
				indices.push_back(face.vid[1]);
			}
		}

		if (genAttribs)
		{
			glBindBuffer(GL_ARRAY_BUFFER, eobj->glAttribBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vtxAttribs[0]) * vtxAttribs.size(), vtxAttribs.data(), GL_STATIC_DRAW);
		}

		if (genIdx)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eobj->glIdxBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
		}
	}

	{
		std::vector<LightData> lightData;
		addLights(lightData, llights, MAX_LLIGHTS);
		addLights(lightData, IO_PDL, MAX_LLIGHTS);
		
		UpdateLights(lightData);
	}

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

	glBindBuffer(GL_ARRAY_BUFFER, eobj->glAttribBuffer);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VtxAttrib), (void*)offsetof(VtxAttrib, uv));
	
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_INT, sizeof(VtxAttrib), (void*)offsetof(VtxAttrib, texId));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VtxAttrib), (void*)offsetof(VtxAttrib, normal));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VtxAttrib), (void*)offsetof(VtxAttrib, color));

	if (useAlphaBlending)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
	}

	//glDrawElements(type, eobj->nbfaces * 3, GL_UNSIGNED_INT, 0);
	glDrawArrays(type, 0, eobj->nbfaces * 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);

	if(useAlphaBlending)
		glDisable(GL_BLEND);
}

void EERIERendererGL::DrawRoom(EERIE_ROOM_DATA* room)
{
	//TODO: a lot of code here shared with DrawPrim, can be refactored out.

	GLenum type = GL_TRIANGLES;

	int px = room->epdata->px;
	int py = room->epdata->py;

	DWORD dwVertexCount = room->nb_vertices;

	if (ioVAO == -1)
	{
		glGenVertexArrays(1, &ioVAO);
	}

	glBindVertexArray(ioVAO);

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
	std::vector<VtxAttrib> vtxAttribs;
	
	const bool genAttribs = (room->glAttribBuffer == 0);


	//TODO: update the index buffer if it has changed.
	const bool genIdx = (room->glIdxBuffer == 0);


	if (room->glAttribBuffer == 0)
	{
		glGenBuffers(1, &room->glAttribBuffer);
	}

	if (room->glIdxBuffer == 0)
	{
		glGenBuffers(1, &room->glIdxBuffer);
	}

	if (genAttribs || genIdx)
	{

		short binding = 0;

		for (long i = 0; i < dwVertexCount; ++i)
		{
			VtxAttrib attrib;

			attrib.uv.x = room->pVtxBuffer[i].tu;
			attrib.uv.y = room->pVtxBuffer[i].tv;

			if (room->pTexIdBuffer[i] != -1 && texMapBindings.find(room->pTexIdBuffer[i]) == texMapBindings.end())
			{
				texMapBindings[room->pTexIdBuffer[i]] = binding;
				binding++;
			}

			short b = (room->pTexIdBuffer[i] == -1) ? -1 : texMapBindings[room->pTexIdBuffer[i]];
			attrib.texId = b;

			attrib.color.a = (room->pVtxBuffer[i].color >> 24 & 0xFF) / 255.0f;
			attrib.color.r = (room->pVtxBuffer[i].color >> 16 & 0xFF) / 255.0f;
			attrib.color.g = (room->pVtxBuffer[i].color >> 8 & 0xFF) / 255.0f;
			attrib.color.b = (room->pVtxBuffer[i].color >> 0 & 0xFF) / 255.0f;

			attrib.normal = glm::vec3(room->pNormals[i].x, -room->pNormals[i].y, room->pNormals[i].z);

			vtxAttribs.push_back(attrib);
		}

		if (genAttribs)
		{
			glBindBuffer(GL_ARRAY_BUFFER, room->glAttribBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vtxAttribs[0]) * vtxAttribs.size(), vtxAttribs.data(), GL_STATIC_DRAW);
		}

		if (genIdx)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, room->glIdxBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * room->nb_indices, room->pussIndice, GL_STATIC_DRAW);
		}
	}

	{
		std::vector<LightData> lightData;
		addLights(lightData, PDL, MAX_LLIGHTS);
		
		if (px != -1 && py != -1)
		{
			TILE_LIGHTS * tls = &tilelights[px][py];
			if(tls->num)
				addLights(lightData, tls->el, tls->num);
		}

		UpdateLights(lightData);
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

	glBindBuffer(GL_ARRAY_BUFFER, room->glAttribBuffer);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VtxAttrib), (void*)offsetof(VtxAttrib, uv));

	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_INT, sizeof(VtxAttrib), (void*)offsetof(VtxAttrib, texId));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VtxAttrib), (void*)offsetof(VtxAttrib, normal));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VtxAttrib), (void*)offsetof(VtxAttrib, color));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, room->glIdxBuffer);

	//TODO: break up the draw command in to smaller commands? Some rooms use a lot of textures.
	glDrawElements(type, room->nb_indices, GL_UNSIGNED_SHORT, 0);
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);

	glUseProgram(0);
}

void EERIERendererGL::DrawRotatedSprite(LPVOID lpvVertices, DWORD dwVertexCount, TextureContainer* tex)
{
	GLenum type = tex? GL_TRIANGLE_FAN : GL_TRIANGLES;

	D3DTLVERTEX* vtxArray = static_cast<D3DTLVERTEX*>(lpvVertices);
	std::vector<GLfloat> vtx;

	static GLuint glVtxBuffer = -1;
	static GLuint glAttribBuffer = -1;

	if (quadVAO == -1)
	{
		glGenVertexArrays(1, &quadVAO);
	}

	glBindVertexArray(quadVAO);

	//TODO: only update vertex buffers when necessary, not every tick.
	{
		//LEAK
		if (glVtxBuffer == -1)
			glGenBuffers(1, &glVtxBuffer);
	}

	GLint oldProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
	GLuint program = EERIEGetGLProgramID("particle");
	glUseProgram(program);

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(EERIE_PRIM_RESTART_IDX);

	glEnable(GL_BLEND);

	glm::mat4 modelMatrix = glm::mat4();
	modelMatrix[1][1] *= -1; //flip the Y-axis
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &modelMatrix[0][0]);

	glm::mat4 p = glm::mat4();
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, &_view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_FALSE, &_projection[0][0]);

	std::vector<VtxAttrib> vtxAttribs;

	if (glAttribBuffer == -1)
	{
		glGenBuffers(1, &glAttribBuffer);
	}

	for (int i = 0; i < dwVertexCount; ++i)
	{
		VtxAttrib attrib;

		attrib.uv.x = vtxArray[i].tu;
		attrib.uv.y = vtxArray[i].tv;

		vtx.push_back(vtxArray[i].sx);
		vtx.push_back(vtxArray[i].sy);
		vtx.push_back(vtxArray[i].sz);
		vtx.push_back(vtxArray[i].rhw);

		attrib.texId = (tex ? 0 : -1);

		attrib.color.a = (vtxArray[i].color >> 24 & 0xFF) / 255.0f;
		attrib.color.r = (vtxArray[i].color >> 16 & 0xFF) / 255.0f;
		attrib.color.g = (vtxArray[i].color >> 8 & 0xFF) / 255.0f;
		attrib.color.b = (vtxArray[i].color >> 0 & 0xFF) / 255.0f;

		vtxAttribs.push_back(attrib);
	}

	glBindBuffer(GL_ARRAY_BUFFER, glAttribBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtxAttribs[0]) * vtxAttribs.size(), vtxAttribs.data(), GL_DYNAMIC_DRAW);

	if (tex)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex->textureID);
		glUniform1i(glGetUniformLocation(program, "texsampler"), 0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, glVtxBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtx[0]) * vtx.size(), vtx.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, glVtxBuffer);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, glAttribBuffer);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VtxAttrib), (void*)offsetof(VtxAttrib, uv));

	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_INT, sizeof(VtxAttrib), (void*)offsetof(VtxAttrib, texId));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(VtxAttrib), (void*)offsetof(VtxAttrib, color));

	glDrawArrays(type, 0, dwVertexCount);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	glUseProgram(oldProgram);

	glDisable(GL_BLEND);
}

void EERIERendererGL::DrawSprite(float x, float y, float sx, float sy, D3DCOLOR col, TextureContainer * tex)
{
	const float w = (float)DANAESIZX, h = (float)DANAESIZY;

	const float startX = (x / w);
	const float startY = (y / h);
	const float dX = (sx / w);
	const float dY = (sy / h);

	if (quadVAO == -1)
	{
		glGenVertexArrays(1, &quadVAO);
	}

	glBindVertexArray(quadVAO);

	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ONE);

	GLuint program = EERIEGetGLProgramID("screenfx");
	glUseProgram(program);

	glm::vec4 color;
	color.a = (col >> 24 & 0xFF) / 255.0f;
	color.r = (col >> 16 & 0xFF) / 255.0f;
	color.g = (col >> 8 & 0xFF) / 255.0f;
	color.b = (col >> 0 & 0xFF) / 255.0f;
	glUniform4fv(glGetUniformLocation(program, "color"), 1, &color[0]);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex->textureID);
	glUniform1i(glGetUniformLocation(program, "texsampler"), 0);

	_drawQuad(program, startX, startY, dX, dY);

	glDisable(GL_BLEND);
}

void EERIERendererGL::UpdateLights(const std::vector<LightData>& lightData)
{
	GLuint program = EERIEGetGLProgramID("poly");
	
	//TODO: don't call this for every object.
	//Has to be done this way for now because legacy Arx updates lights between objects
	{
		
		static GLuint lightBuffer = -1;
		if (lightBuffer == -1)
			glGenBuffers(1, &lightBuffer);

		if (lightData.size())
		{
			glBindBuffer(GL_UNIFORM_BUFFER, lightBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(LightData) * lightData.size(), lightData.data(), GL_DYNAMIC_DRAW);

			GLuint block = glGetUniformBlockIndex(program, "LightData");
			glUniformBlockBinding(program, block, 1);
			glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightBuffer);
		}

		glUniform1i(glGetUniformLocation(program, "numLights"), lightData.size());
	}
}

void EERIERendererGL::setProj(const glm::mat4& proj)
{
	EERIERenderer::setProj(proj);

	GLuint program = EERIEGetGLProgramID("poly");
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_FALSE, &proj[0][0]);
}

void EERIERendererGL::setView(const glm::mat4& view)
{
	EERIERenderer::setView(view);

	GLuint program = EERIEGetGLProgramID("poly");
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, &view[0][0]);
}

void EERIERendererGL::_drawQuad(GLuint program, float x, float y, float sx, float sy, const float* uvs)
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
	}

	if (uvbuffer == -1)
	{
		glGenBuffers(1, &uvbuffer);
	}


	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);

	if(uvs)
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, uvs, GL_DYNAMIC_DRAW);
	else
		glBufferData(GL_ARRAY_BUFFER, sizeof(uvData), uvData, GL_DYNAMIC_DRAW);

	const GLfloat quadData[] = {
		//Top left
		(x), (y), 0.0f,

		//Top right
		(x + sx), (y), 0.0f,

		//Bot left
		(x), (y + sy), 0.0f,

		//Bot right
		(x + sx), (y + sy), 0.0f
	};


	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glm::mat4 proj = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_FALSE, &proj[0][0]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}




/************************************************************/
/*							D3D								*/
/************************************************************/

void EERIERendererD3D7::DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ)
{

}

void EERIERendererD3D7::DrawFade(const EERIE_RGB& color, float visibility)
{
	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
	SETZWRITE(GDevice, FALSE);
	SETALPHABLEND(GDevice, TRUE);

	EERIEDrawBitmap(GDevice, 0.f, 0.f, (float)DANAESIZX, (float)DANAESIZY, 0.0001f,
		NULL, _EERIERGB(visibility));

	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	float col = visibility;
	EERIEDrawBitmap(GDevice, 0.f, 0.f, (float)DANAESIZX, (float)DANAESIZY, 0.0001f,
		NULL, EERIERGB(col*FADECOLOR.r, col*FADECOLOR.g, col*FADECOLOR.b));
	SETALPHABLEND(GDevice, FALSE);
	SETZWRITE(GDevice, TRUE);
}

void EERIERendererD3D7::DrawPrim(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io)
{

}
