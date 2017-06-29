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


#define STB_RECT_PACK_IMPLEMENTATION
#include <stb/stb_rect_pack.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>


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

void EERIERenderer::DrawObj(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io)
{

}


/************************************************************/
/*							GL								*/
/************************************************************/


EERIERendererGL::~EERIERendererGL()
{
	for (EERIEFont* font : _fonts)
	{
		delete font;
	}

	_fonts.clear();

	glDeleteVertexArrays(1, &ioVAO);
	glDeleteVertexArrays(1, &quadVAO);
}


void EERIERendererGL::DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ)
{
	D3DCOLOR color = 0L;
	EERIEDrawAnimQuat(0, eobj, eanim, angle, pos, time, io, color, typ);
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

void EERIERendererGL::DrawObj(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io)
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

void EERIERendererGL::DrawQuad(float x, float y, float sx, float sy, float z, TextureContainer * tex, const float* uvs, unsigned long color)
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
}

void EERIERendererGL::DrawText(char* text, float x, float y, long col, int vHeightPx)
{
	GLuint program = EERIEGetGLProgramID("text");
	glUseProgram(program);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Toggle this if you want a unique fontsheet for every size
	//EERIEFont* font = _getFont(vHeightPx);
	//For now just load in the font at a large size and scale it down.
	EERIEFont* font = _getFont(48);

	GLuint uniformLocation = glGetUniformLocation(program, "texsampler");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font->glTextureID);
	glUniform1i(uniformLocation, 0);

	//col is BGR
	glm::vec3 color = glm::vec3((col >> 0 & 0xFF) / 255.0f, (col >> 8 & 0xFF) / 255.0f, (col >> 16 & 0xFF) / 255.0f);
	glUniform3fv(glGetUniformLocation(program, "fontColor"), 1, &color[0]);

	stbtt_aligned_quad quad;
	float offsetX = 0.0f, offsetY = 0.0f;

	char* c = text;

	float offset = 0.0f;
	int idx;

	//TODO: the 1.33f factor is a magic number based on Windows point scale divisions i.e. 96/72.
	//This should be fixed in a different, more reliable way.
	const float glyphScreenSize = (vHeightPx * Yratio) / 1.33f;
	const float glyphScale = glyphScreenSize / (float)font->fontSize;

	//TODO: batch!
	while (*c)
	{
		idx = (int)*c - font->info.fontstart;

		stbtt_GetPackedQuad(font->fontData, font->bitmapWidth, font->bitmapHeight, idx, &offsetX, &offsetY, &quad, 0);

		const float uvs[] = {
			quad.s0, quad.t0,	//Top Left
			quad.s1, quad.t0,	//Top Right
			quad.s0, quad.t1,	//Bot Left
			quad.s1, quad.t1	//Bot Right
		};

		float adv = font->fontData[idx].xadvance * glyphScale;
		float yStart = (font->fontData[idx].yoff*glyphScale);
		float yEnd = (font->fontData[idx].yoff2 - font->fontData[idx].yoff) * glyphScale;
		_drawQuad(program, (x + offset) / (float)DANAESIZX, ((y + glyphScreenSize) + yStart) / (float)DANAESIZY, adv / (float)DANAESIZX, yEnd / (float)DANAESIZY, uvs);

		offset += adv;
		++c;
	}

	glDisable(GL_BLEND);
}

void EERIERendererGL::MeasureText(char* text, int size, int* width, int* height)
{
	EERIEFont* f = _getFont(size);

	f->measure(text, width, height);
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

void EERIERendererGL::SetBlendFunc(EERIEBlendType srcFactor, EERIEBlendType dstFactor)
{
	glBlendFunc(_nativeBlendType(srcFactor), _nativeBlendType(dstFactor));
}

void EERIERendererGL::SetViewport(int x, int y, int w, int h)
{
	glViewport(x, y, w, h);
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




EERIEFont* EERIERendererGL::_getFont(int size)
{
	for (EERIEFont* font : _fonts)
	{
		if (font->fontSize == size)
			return font;
	}

	return _loadFontData(size);
}

EERIEFont* EERIERendererGL::_loadFontData(int size)
{
	EERIEFont* font = new EERIEFont;

	//TODO: this is too big for some fonts. Can make it smaller.
	font->bitmapWidth = 1024;
	font->bitmapHeight = 1024;
	font->fontSize = size;

	char tx[256];

	sprintf(tx, "%smisc\\%s", Project.workingdir, "arx.ttf"); // Full path

	font->ttf = new unsigned char[1 << 24];
	FILE* file = fopen(tx, "rb");
	fread(font->ttf, 1, 1 << 24, file);
	fclose(file);

	stbtt_InitFont(&font->info, font->ttf, 0);

	font->bitmap = new uint8_t[font->bitmapWidth * font->bitmapHeight];
	font->fontData = new stbtt_packedchar[font->info.numGlyphs];

	stbtt_pack_context context;
	stbtt_PackBegin(&context, font->bitmap, font->bitmapWidth, font->bitmapHeight, 0, 1, nullptr);
	stbtt_PackSetOversampling(&context, 2, 2);
	stbtt_PackFontRange(&context, font->ttf, 0, font->fontSize, font->info.fontstart, font->info.numGlyphs, font->fontData);
	stbtt_PackEnd(&context);

	font->bind();

	_fonts.push_back(font);

	return font;
}

GLenum EERIERendererGL::_nativeBlendType(EERIEBlendType type)
{
	switch (type)
	{
	case EERIEBlendType::Zero:
		return GL_ZERO;
		break;
	case EERIEBlendType::One:
		return GL_ONE;
		break;
	case EERIEBlendType::SrcColor:
		return GL_SRC_COLOR;
		break;
	case EERIEBlendType::OneMinusSrcColor:
		return GL_ONE_MINUS_SRC_COLOR;
		break;
	case EERIEBlendType::DstColor:
		return GL_DST_COLOR;
		break;
	case EERIEBlendType::OneMinusDstColor:
		return GL_ONE_MINUS_DST_COLOR;
		break;
	case EERIEBlendType::SrcAlpha:
		return GL_SRC_ALPHA;
		break;
	case EERIEBlendType::OneMinusSrcAlpha:
		return GL_ONE_MINUS_SRC_ALPHA;
		break;
	case EERIEBlendType::DstAlpha:
		return GL_DST_ALPHA;
		break;
	case EERIEBlendType::OneMinusDstAlpha:
		return GL_ONE_MINUS_DST_ALPHA;
		break;
	case EERIEBlendType::ConstantColor:
		return GL_CONSTANT_COLOR;
		break;
	case EERIEBlendType::OneMinusConstantColor:
		return GL_ONE_MINUS_CONSTANT_COLOR;
		break;
	case EERIEBlendType::ConstantAlpha:
		return GL_CONSTANT_ALPHA;
		break;
	case EERIEBlendType::OneMinusConstantAlpha:
		return GL_ONE_MINUS_CONSTANT_ALPHA;
		break;
	default:
		break;
	}

	return GL_ONE;
}

/************************************************************/
/*							D3D								*/
/************************************************************/

void EERIERendererD3D7::DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ)
{

}

void EERIERendererD3D7::DrawQuad(float x, float y, float sx, float sy, float z, TextureContainer * tex, const float* uvs /*= 0*/, unsigned long color /*= 0*/)
{
	float smu, smv;
	float fEndu, fEndv;

	float u0, u1, u2, u3;
	float v0, v1, v2, v3;

	if (uvs)
	{
		u0 = uvs[0], u1 = uvs[2], u2 = uvs[4], u3 = uvs[6];
		v0 = uvs[1], v1 = uvs[3], v2 = uvs[5], v3 = uvs[7];
	}
	else if (tex)
	{
		u0 = tex->m_hdx, u1 = tex->m_dx, u2 = tex->m_hdx, u3 = tex->m_dx;
		v0 = tex->m_hdy, v1 = tex->m_hdy, v2 = tex->m_dy, v3 = tex->m_dy;
	}
	else
	{
		u0 = 0.0f, u1 = 1.0f, u2 = 0.0f, u3 = 1.0f;
		v0 = 0.0f, v1 = 0.0f, v2 = 1.0f, v3 = 1.0f;
	}

	if (tex)
	{
		smu = tex->m_hdx;
		smv = tex->m_hdy;
		fEndu = tex->m_dx;
		fEndv = tex->m_dy;
	}
	else
	{
		smu = smv = 0.f;
		fEndu = fEndv = 0.f;
	}

	D3DTLVERTEX v[4];
	v[0] = D3DTLVERTEX(D3DVECTOR(x, y, z), 1.f, color, 0xFF000000, smu + u0, smv + v0);
	v[1] = D3DTLVERTEX(D3DVECTOR(x + sx, y, z), 1.f, color, 0xFF000000, smu + u1, smv + v1);
	v[2] = D3DTLVERTEX(D3DVECTOR(x, y + sy, z), 1.f, color, 0xFF000000, smu + u2, smv + v2);
	v[3] = D3DTLVERTEX(D3DVECTOR(x + sx, y + sy, z), 1.f, color, 0xFF000000, smu + u3, smv + v3);

	SETTC(GDevice, tex);
	GDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, v, 4, 0);
}

void EERIERendererD3D7::SetBlendFunc(EERIEBlendType srcFactor, EERIEBlendType dstFactor)
{
	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, _nativeBlendType(srcFactor));
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, _nativeBlendType(dstFactor));
}

void EERIERendererD3D7::DrawFade(const EERIE_RGB& color, float visibility)
{
	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
	SETZWRITE(GDevice, FALSE);
	SETALPHABLEND(GDevice, TRUE);

	DrawQuad(0.f, 0.f, (float)DANAESIZX, (float)DANAESIZY, 0.0001f,
		0, 0, _EERIERGB(visibility));

	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	float col = visibility;
	DrawQuad(0.f, 0.f, (float)DANAESIZX, (float)DANAESIZY, 0.0001f,
		0, 0, EERIERGB(col*FADECOLOR.r, col*FADECOLOR.g, col*FADECOLOR.b));
	SETALPHABLEND(GDevice, FALSE);
	SETZWRITE(GDevice, TRUE);
}

void EERIERendererD3D7::DrawObj(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io)
{

}

bool bForce_NoVB = false;
void SET_FORCE_NO_VB(const bool& _NoVB)
{
	bForce_NoVB = _NoVB;
}
bool GET_FORCE_NO_VB(void)
{
	return bForce_NoVB;
}

//Draw primitive using vertex buffer
HRESULT ARX_DrawPrimitiveVB(LPDIRECT3DDEVICE7	_d3dDevice,
	D3DPRIMITIVETYPE	_dptPrimitiveType,
	DWORD				_dwVertexTypeDesc,	// Vertex Format
	LPVOID				_pD3DTLVertex,		// don't specify _pD3DTLVertex type.
	int *				_piNbVertex,
	DWORD				_dwFlags);

void EERIERendererD3D7::DrawPrim(EERIEPrimType primType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags, long eerieFlags)
{
	D3DPRIMITIVETYPE dptPrimitiveType = _toD3DPT(primType);

	if (!(EERIE_NOCOUNT & eerieFlags))
	{
		EERIEDrawnPolys++;
	}

	if (!bForce_NoVB &&	eerieFlags&EERIE_USEVB)
	{
		ARX_DrawPrimitiveVB(GDevice,
			dptPrimitiveType,
			dwVertexTypeDesc, 		//FVF
			lpvVertices,			//No type specified
			(int*)&dwVertexCount,
			dwFlags);				//Same thing throught DrawPrimitiveVB
	}
	else
	{
		GDevice->DrawPrimitive(dptPrimitiveType, dwVertexTypeDesc, lpvVertices, dwVertexCount, dwFlags);
	}
}

void EERIERendererD3D7::SetViewport(int x, int y, int w, int h)
{
	D3DVIEWPORT7 vp = { (unsigned long)x, (unsigned long)y, (unsigned long)w, (unsigned long)h, 0.f, 1.f };

	GDevice->SetViewport(&vp);
}

D3DPRIMITIVETYPE EERIERendererD3D7::_toD3DPT(EERIEPrimType primType)
{
	D3DPRIMITIVETYPE prim = D3DPT_TRIANGLELIST;

	switch (primType)
	{
	case EERIEPrimType::TriangleList:
		prim = D3DPT_TRIANGLELIST;
		break;
	case EERIEPrimType::TriangleStrip:
		prim = D3DPT_TRIANGLESTRIP;
		break;
	case EERIEPrimType::TriangleFan:
		prim = D3DPT_TRIANGLEFAN;
		break;
	}

	return prim;
}

D3DBLEND EERIERendererD3D7::_nativeBlendType(EERIEBlendType type)
{
	switch (type)
	{
	case EERIEBlendType::Zero:
		return D3DBLEND_ZERO;
		break;
	case EERIEBlendType::One:
		return D3DBLEND_ONE;
		break;
	case EERIEBlendType::SrcColor:
		return D3DBLEND_SRCCOLOR;
		break;
	case EERIEBlendType::OneMinusSrcColor:
		return D3DBLEND_INVSRCCOLOR;
		break;
	case EERIEBlendType::DstColor:
		return D3DBLEND_DESTCOLOR;
		break;
	case EERIEBlendType::OneMinusDstColor:
		return D3DBLEND_INVDESTCOLOR;
		break;
	case EERIEBlendType::SrcAlpha:
		return D3DBLEND_SRCALPHA;
		break;
	case EERIEBlendType::OneMinusSrcAlpha:
		return D3DBLEND_INVSRCALPHA;
		break;
	case EERIEBlendType::DstAlpha:
		return D3DBLEND_DESTALPHA;
		break;
	case EERIEBlendType::OneMinusDstAlpha:
		return D3DBLEND_INVDESTALPHA;
		break;

		//Not supported by D3D7
	case EERIEBlendType::ConstantColor:
	case EERIEBlendType::OneMinusConstantColor:
	case EERIEBlendType::ConstantAlpha:
	case EERIEBlendType::OneMinusConstantAlpha:
		return D3DBLEND_ONE;
		break;
	default:
		break;
	}

	return D3DBLEND_ONE;
}
