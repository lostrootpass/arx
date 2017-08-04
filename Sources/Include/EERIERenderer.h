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

#include <stb/stb_truetype.h>

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

struct EERIEFont
{
	stbtt_fontinfo info;
	stbtt_packedchar* fontData;
	uint8_t* bitmap;
	unsigned char* ttf;
	unsigned int glTextureID;
	int fontSize;
	int bitmapWidth;
	int bitmapHeight;

	~EERIEFont()
	{
		glDeleteTextures(1, &glTextureID);

		delete[] fontData;
		delete[] bitmap;
		delete[] ttf;
	}

	void bind()
	{
		glGenTextures(1, &glTextureID);
		glBindTexture(GL_TEXTURE_2D, glTextureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bitmapWidth, bitmapHeight, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	void measure(char* text, int* width, int* height)
	{
		*width = 0;
		*height = fontSize;

		char* c = text;
		while (*c)
		{
			*width += (int)fontData[(int)*c++ - info.fontstart].xadvance;
		}
	}
};

struct EERIEParticle
{
	D3DTLVERTEX vtx[4];
	TextureContainer* tex;
};

class EERIERenderer
{
protected:
	glm::mat4 _projection;
	glm::mat4 _view;

public:
	virtual ~EERIERenderer() = 0;

	virtual void AddParticle(D3DTLVERTEX *in, float siz, TextureContainer * tex, D3DCOLOR col, float Zpos, float rot) {};
	virtual void DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D  * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ) {};
	virtual void DrawCinematic(float x, float y, float sx, float sy, float z, TextureContainer * tex, C_LIGHT* light, float LightRND) {};
	virtual void DrawFade(const EERIE_RGB& color, float visibility) {};
	virtual void DrawObj(EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io = 0, EERIE_3D* pos = 0, EERIE_3D* angle = 0, EERIE_MOD_INFO* modinfo = 0, EERIEMATRIX* matrix = 0) {};
	virtual void DrawPrim(EERIEPrimType primType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags, long eerieFlags = 0) {};
	virtual void DrawQuad(float x, float y, float sx, float sy, float z, TextureContainer * tex, const float* uvs = 0, unsigned long color = 0) {};
	virtual void DrawRoom(EERIE_ROOM_DATA* room) {};
	virtual void DrawRotatedSprite(LPVOID lpvVertices, DWORD dwVertexCount, TextureContainer* tex) {};
	virtual void DrawSprite(float x, float y, float sx, float sy, D3DCOLOR col, TextureContainer * tex) {};
	virtual void DrawText(char* text, float x, float y, long col = 0xFFFFFF, int vHeightPx = 24) {};
	virtual void FlushParticles() { };

	virtual void MeasureText(char* text, int size, int* width, int* height) {};

	virtual void UpdateLights(const std::vector<LightData>& lightData) {};

	inline glm::mat4 proj() const { return _projection; }
	inline virtual void setProj(const glm::mat4& proj) { _projection = proj; }

	inline glm::mat4 view() const { return _view; }
	inline virtual void setView(const glm::mat4& view) { _view = view; }

	virtual void SetAlphaBlend(bool enableAlphaBlending) {};
	virtual void SetBlendFunc(EERIEBlendType srcFactor, EERIEBlendType dstFactor) {};
	virtual void SetCull(EERIECull mode) {};
	virtual void SetViewport(int x, int y, int w, int h) {};
	virtual void SetZFunc(EERIEZFunc func) {};
	virtual void SetZWrite(bool enableZWrite) {};
};

inline EERIERenderer::~EERIERenderer() {}

/************************************************************/
/*							GL								*/
/************************************************************/
class EERIERendererGL : public EERIERenderer
{
public:
	~EERIERendererGL();


	void AddParticle(D3DTLVERTEX *in, float siz, TextureContainer * tex, D3DCOLOR col, float Zpos, float rot) override;
	void DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D  * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ) override;
	void DrawCinematic(float x, float y, float sx, float sy, float z, TextureContainer * tex, C_LIGHT* light, float LightRND) override;
	void DrawFade(const EERIE_RGB& color, float visibility) override;
	void DrawObj(EERIE_3DOBJ* eobj, INTERACTIVE_OBJ* io = 0, EERIE_3D* pos = 0, EERIE_3D* angle = 0, EERIE_MOD_INFO* modinfo = 0, EERIEMATRIX* matrix = 0) override;
	void DrawPrim(EERIEPrimType primType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags, long eerieFlags = 0) override;
	void DrawQuad(float x, float y, float sx, float sy, float z, TextureContainer * tex, const float* uvs = 0, unsigned long color = 0) override;
	void DrawRoom(EERIE_ROOM_DATA* room) override;
	void DrawRotatedSprite(LPVOID lpvVertices, DWORD dwVertexCount, TextureContainer* tex) override;
	void DrawSprite(float x, float y, float sx, float sy, D3DCOLOR col, TextureContainer* tex) override;
	void DrawText(char* text, float x, float y, long col = 0xFFFFFF, int vHeightPx = 24) override;
	void FlushParticles() override;

	void MeasureText(char* text, int size, int* width, int* height) override;

	void UpdateLights(const std::vector<LightData>& lightData) override;

	void setProj(const glm::mat4& proj) override;
	void setView(const glm::mat4& view) override;

	void SetAlphaBlend(bool enableAlphaBlending) override;
	void SetBlendFunc(EERIEBlendType srcFactor, EERIEBlendType dstFactor) override;
	void SetCull(EERIECull mode) override;
	void SetViewport(int x, int y, int w, int h) override;
	void SetZFunc(EERIEZFunc func) override;
	void SetZWrite(bool enableZWrite) override;

private:

	//TODO: move/remove
	std::unordered_map<void*, std::unordered_map<short, short> > _texMapBindings;

	std::vector<EERIEFont*> _fonts;

	std::vector<EERIEParticle> _particles;

	void _drawQuad(GLuint program, float x, float y, float sx, float sy, const float* uvs = 0);

	EERIEFont* _getFont(int size);

	EERIEFont* _loadFontData(int size);

	GLenum _nativeBlendType(EERIEBlendType type);

	GLenum _toGLType(EERIEPrimType type);
};


/************************************************************/
/*							D3D								*/
/************************************************************/
class EERIERendererD3D7 : public EERIERenderer
{
public:
	~EERIERendererD3D7() {};

	void AddParticle(D3DTLVERTEX *in, float siz, TextureContainer * tex, D3DCOLOR col, float Zpos, float rot) override;
	void DrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, EERIE_3D * angle, EERIE_3D  * pos, unsigned long time, INTERACTIVE_OBJ * io, long typ) override;
	void DrawFade(const EERIE_RGB& color, float visibility) override;
	void DrawPrim(EERIEPrimType primType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags, long eerieFlags = 0) override;
	void DrawQuad(float x, float y, float sx, float sy, float z, TextureContainer * tex, const float* uvs = 0, unsigned long color = 0) override;

	void SetAlphaBlend(bool enableAlphaBlending) override;
	void SetBlendFunc(EERIEBlendType srcFactor, EERIEBlendType dstFactor) override;
	void SetCull(EERIECull mode) override;
	void SetViewport(int x, int y, int w, int h) override;
	void SetZFunc(EERIEZFunc func) override;
	void SetZWrite(bool enableZWrite) override;

private:

	D3DPRIMITIVETYPE _toD3DPT(EERIEPrimType primType);
	D3DBLEND _nativeBlendType(EERIEBlendType type);
};

#endif