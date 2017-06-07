#ifndef EERIE_RENDERER_H_
#define EERIE_RENDERER_H_

#include "EERIEAnim.h"
#include "EERIEobject.h"

#include <unordered_map>

class EERIERenderer
{
public:
	virtual void DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D  * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ);
	virtual void DrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex);
	virtual void DrawIndexedPrim(LPVOID lpvVertices, DWORD dwVertexCount, unsigned short* indices, DWORD idxCount, TextureContainer* tex);
	virtual void DrawPrim(LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io);
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

private:

	//TODO: move/remove
	std::unordered_map<EERIE_3DOBJ*, std::unordered_map<short, short> > _texMapBindings;
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