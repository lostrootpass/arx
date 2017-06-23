#ifndef EERIE_RENDERER_H_
#define EERIE_RENDERER_H_

#include "EERIEAnim.h"
#include "EERIEobject.h"
#include "EERIE_GL.h"
#include "EERIERendererTypes.h"

#include <glm/glm.hpp>
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include <unordered_map>
#include <vector>

const unsigned short EERIE_PRIM_RESTART_IDX = 65534;

class C_LIGHT;

struct LightData
{
	glm::vec4 pos;
	glm::vec4 color;
	float fallstart;
	float fallend;
	float precalc;
	float ___pad[1];
};

class EERIERenderer
{
protected:
	glm::mat4 _projection;
	glm::mat4 _view;

public:
	virtual void DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D  * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ);
	virtual void DrawCinematic(float x, float y, float sx, float sy, float z, TextureContainer * tex, C_LIGHT* light, float LightRND) {};
	virtual void DrawFade(const EERIE_RGB& color, float visibility) {};
	virtual void DrawObj(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io);
	virtual void DrawPrim(EERIEPrimType primType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags, long eerieFlags = 0) {};
	virtual void DrawQuad(float x, float y, float sx, float sy, float z, TextureContainer * tex, const float* uvs = 0, unsigned long color = 0) {};
	virtual void DrawRoom(EERIE_ROOM_DATA* room) {};
	virtual void DrawRotatedSprite(LPVOID lpvVertices, DWORD dwVertexCount, TextureContainer* tex) {};
	virtual void DrawSprite(float x, float y, float sx, float sy, D3DCOLOR col, TextureContainer * tex) {};

	virtual void UpdateLights(const std::vector<LightData>& lightData) {};

	inline glm::mat4 proj() const { return _projection; }
	inline virtual void setProj(const glm::mat4& proj) { _projection = proj; }

	inline glm::mat4 view() const { return _view; }
	inline virtual void setView(const glm::mat4& view) { _view = view; }

	virtual void SetViewport(int x, int y, int w, int h) {};
};


/************************************************************/
/*							GL								*/
/************************************************************/
class EERIERendererGL : public EERIERenderer
{
public:
	void DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D  * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ) override;
	void DrawCinematic(float x, float y, float sx, float sy, float z, TextureContainer * tex, C_LIGHT* light, float LightRND) override;
	void DrawFade(const EERIE_RGB& color, float visibility) override;
	void DrawObj(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io) override;
	void DrawQuad(float x, float y, float sx, float sy, float z, TextureContainer * tex, const float* uvs = 0, unsigned long color = 0) override;
	void DrawRoom(EERIE_ROOM_DATA* room) override;
	void DrawRotatedSprite(LPVOID lpvVertices, DWORD dwVertexCount, TextureContainer* tex) override;
	void DrawSprite(float x, float y, float sx, float sy, D3DCOLOR col, TextureContainer* tex) override;

	void UpdateLights(const std::vector<LightData>& lightData) override;

	void setProj(const glm::mat4& proj) override;
	void setView(const glm::mat4& view) override;

	void SetViewport(int x, int y, int w, int h) override;
private:

	//TODO: move/remove
	std::unordered_map<void*, std::unordered_map<short, short> > _texMapBindings;

	void _drawQuad(GLuint program, float x, float y, float sx, float sy, const float* uvs = 0);
};


/************************************************************/
/*							D3D								*/
/************************************************************/
class EERIERendererD3D7 : public EERIERenderer
{
public:
	void DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D  * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ) override;
	void DrawFade(const EERIE_RGB& color, float visibility) override;
	void DrawObj(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io) override;
	void DrawPrim(EERIEPrimType primType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags, long eerieFlags = 0) override;
	void DrawQuad(float x, float y, float sx, float sy, float z, TextureContainer * tex, const float* uvs = 0, unsigned long color = 0) override;

	void SetViewport(int x, int y, int w, int h) override;

private:

	D3DPRIMITIVETYPE _toD3DPT(EERIEPrimType primType);
};

#endif