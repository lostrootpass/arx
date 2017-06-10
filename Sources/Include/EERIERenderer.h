#ifndef EERIE_RENDERER_H_
#define EERIE_RENDERER_H_

#include "EERIEAnim.h"
#include "EERIEobject.h"
#include "EERIE_GL.h"

#include <glm/glm.hpp>
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include <unordered_map>

const unsigned short EERIE_PRIM_RESTART_IDX = 65534;

class EERIERenderer
{
	glm::mat4 _projection;

public:
	virtual void DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D  * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ);
	virtual void DrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex);
	virtual void DrawIndexedPrim(LPVOID lpvVertices, DWORD dwVertexCount, unsigned short* indices, DWORD idxCount, TextureContainer* tex);
	virtual void DrawPrim(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io);
	virtual void DrawRoom(EERIE_ROOM_DATA* room) {};

	inline glm::mat4 proj() const { return _projection; }
	inline void setProj(const glm::mat4& proj) { _projection = proj; }
};


/************************************************************/
/*							GL								*/
/************************************************************/
class EERIERendererGL : public EERIERenderer
{
public:
	void DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D  * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ) override;
	void DrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex) override;
	void DrawIndexedPrim(LPVOID lpvVertices, DWORD dwVertexCount, unsigned short* indices, DWORD idxCount, TextureContainer* tex) override;
	void DrawPrim(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io) override;
	void DrawRoom(EERIE_ROOM_DATA* room) override;

private:

	//TODO: move/remove
	std::unordered_map<void*, std::unordered_map<short, short> > _texMapBindings;
};


/************************************************************/
/*							D3D								*/
/************************************************************/
class EERIERendererD3D7 : public EERIERenderer
{
public:
	void DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D  * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ) override;
	void DrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex) override;
	void DrawIndexedPrim(LPVOID lpvVertices, DWORD dwVertexCount, unsigned short* indices, DWORD idxCount, TextureContainer* tex) override;
	void DrawPrim(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io) override;
};

#endif