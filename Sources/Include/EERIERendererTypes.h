#ifndef EERIE_RENDERER_TYPES_H_
#define EERIE_RENDERER_TYPES_H_

enum class EERIEPrimType
{
	TriangleList,
	TriangleStrip,
	TriangleFan,

	LineList,
	LineStrip
};

enum class EERIEBlendType
{
	Zero,
	One,
	SrcColor, 
	OneMinusSrcColor, 
	DstColor, 
	OneMinusDstColor, 
	SrcAlpha, 
	OneMinusSrcAlpha, 
	DstAlpha, 
	OneMinusDstAlpha,
	ConstantColor,
	OneMinusConstantColor,
	ConstantAlpha, 
	OneMinusConstantAlpha
};

#endif // !EERIE_RENDERER_TYPES_H_
