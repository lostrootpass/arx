#ifndef EERIE_RENDERER_H_
#define EERIE_RENDERER_H_

#include "EERIEAnim.h"
#include "EERIEobject.h"

class EERIERenderer
{
public:
	virtual void DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D  * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ);
	virtual void DrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex);
	virtual void DrawPrim(TextureContainer* tex, LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io);
};


/************************************************************/
/*							GL								*/
/************************************************************/
class EERIERendererGL : public EERIERenderer
{
public:
	void DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D  * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ) override;
	void DrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex) override;
	void DrawPrim(TextureContainer* tex, LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io) override;
};


/************************************************************/
/*							D3D								*/
/************************************************************/
class EERIERendererD3D7 : public EERIERenderer
{
public:
	void DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D  * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ) override;
	void DrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex) override;
	void DrawPrim(TextureContainer* tex, LPVOID lpvVertices, DWORD dwVertexCount, EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io) override;
};

#endif