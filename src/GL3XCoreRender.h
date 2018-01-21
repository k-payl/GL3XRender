/**
\author		Konstantin Pajl aka Consta
\date		11.01.2018 (c)Andrey Korotkov

This file is a part of DGLE project and is distributed
under the terms of the GNU Lesser General Public License.
See "DGLE.h" for more details.
*/

#pragma once
#include "DGLE.h"
#include "DGLE_CoreRenderer.h"
#include "GL/glew.h"
#include <vector>


using namespace DGLE;

class GL3XCoreRender;
struct ShaderSrc;

enum INPUT_ATTRIBUTE
{
	NONE = 0,
	POS = 1,
	NORM = 2,
	TEX_COORD = 4
};
inline INPUT_ATTRIBUTE operator|(INPUT_ATTRIBUTE a, INPUT_ATTRIBUTE b)
{
	return static_cast<INPUT_ATTRIBUTE>(static_cast<int>(a) | static_cast<int>(b));
}
inline INPUT_ATTRIBUTE operator&(INPUT_ATTRIBUTE a, INPUT_ATTRIBUTE b)
{
	return static_cast<INPUT_ATTRIBUTE>(static_cast<int>(a) & static_cast<int>(b));
}

class GLShader
{
	const ShaderSrc *p;
	GLuint programID;
	GLuint fragID;
	GLuint vertID;

public:

	GLuint ID_Program() const { return programID; }
	
	void Init(const ShaderSrc& parent);
	void Free();

	bool bPositionIsVec2() const;
	bool bInputNormals() const;
	bool bInputTextureCoords() const;
	bool hasUniform(std::string) const;
	bool bAlphaTest() const;
};

class GLGeometryBuffer final : public ICoreGeometryBuffer
{
	bool _bAlreadyInitalized;
	uint _vertexBytes;
	uint _indexBytes;
	GLsizei _vertexCount;
	GLsizei _indexCount;
	GLuint _vao;
	GLuint _vbo;
	GLuint _ibo;
	E_CORE_RENDERER_BUFFER_TYPE _eBufferType;
	E_CORE_RENDERER_DRAW_MODE _eDrawMode;
	GL3XCoreRender * const _pRnd;
	INPUT_ATTRIBUTE _attribs_presented;
	GLuint activated_attributes[5];
	bool _b2dPosition;

public:

	GLGeometryBuffer(E_CORE_RENDERER_BUFFER_TYPE eType, bool indexBuffer, GL3XCoreRender *pRnd);
	~GLGeometryBuffer();

	GLuint input_attrib_to_uint(INPUT_ATTRIBUTE attrib);
	inline GLuint VAO_ID() { return _vao; }
	inline bool IndexDrawing() { return _ibo > 0; }
	inline GLsizei VertexCount() { return _vertexCount; }
	inline GLsizei IndexCount() { return _indexCount; }
	inline INPUT_ATTRIBUTE GetAttributes() { return _attribs_presented; }
	inline bool Is2dPosition() { return _b2dPosition; }
	inline GLenum GLDrawMode();
	inline void ToggleAttribInVAO(INPUT_ATTRIBUTE attrib, bool value);
	GLsizei vertexSize(const TDrawDataDesc& stDrawDesc);
	
	DGLE_RESULT DGLE_API GetGeometryData(TDrawDataDesc& stDesc, uint uiVerticesDataSize, uint uiIndexesDataSize) override;
	DGLE_RESULT DGLE_API SetGeometryData(const TDrawDataDesc& stDrawDesc, uint uiVerticesDataSize, uint uiIndexesDataSize) override;
	DGLE_RESULT DGLE_API Reallocate(const TDrawDataDesc& stDrawDesc, uint uiVerticesCount, uint uiIndicesCount, E_CORE_RENDERER_DRAW_MODE eMode) override;
	DGLE_RESULT DGLE_API GetBufferDimensions(uint& uiVerticesDataSize, uint& uiVerticesCount, uint& uiIndexesDataSize, uint& uiIndexesCount) override;
	DGLE_RESULT DGLE_API GetBufferDrawDataDesc(TDrawDataDesc& stDesc) override;
	DGLE_RESULT DGLE_API GetBufferDrawMode(E_CORE_RENDERER_DRAW_MODE& eMode) override;
	DGLE_RESULT DGLE_API GetBufferType(E_CORE_RENDERER_BUFFER_TYPE& eType) override;
	DGLE_RESULT DGLE_API GetBaseObject(IBaseRenderObjectContainer*& prObj) override;
	DGLE_RESULT DGLE_API Free() override;

	IDGLE_BASE_IMPLEMENTATION(ICoreGeometryBuffer, INTERFACE_IMPL_END)
};

class GLTexture final : public ICoreTexture
{
	GLuint _textureID;
	bool _bMipmapsAllocated;

public:

	GLTexture();
	~GLTexture();	

	inline GLuint Texture_ID() { return _textureID; }
	void SetMipmapAllocated() { _bMipmapsAllocated = true; }

	DGLE_RESULT DGLE_API GetSize(uint& width, uint& height) override;
	DGLE_RESULT DGLE_API GetDepth(uint& depth) override;
	DGLE_RESULT DGLE_API GetType(E_TEXTURE_TYPE& eType) override;
	DGLE_RESULT DGLE_API GetFormat(E_TEXTURE_DATA_FORMAT& eFormat) override;
	DGLE_RESULT DGLE_API GetLoadFlags(E_TEXTURE_LOAD_FLAGS& eLoadFlags) override;
	DGLE_RESULT DGLE_API GetPixelData(uint8* pData, uint& uiDataSize, uint uiLodLevel) override;
	DGLE_RESULT DGLE_API SetPixelData(const uint8* pData, uint uiDataSize, uint uiLodLevel) override;
	DGLE_RESULT DGLE_API Reallocate(const uint8* pData, uint uiWidth, uint uiHeight, bool bMipMaps, E_TEXTURE_DATA_FORMAT eDataFormat) override;
	DGLE_RESULT DGLE_API GetBaseObject(IBaseRenderObjectContainer*& prObj) override;
	DGLE_RESULT DGLE_API Free() override;

	IDGLE_BASE_IMPLEMENTATION(ICoreTexture, INTERFACE_IMPL_END)
};

struct State
{
	State() : alphaTest(false), tex_ID_last_binded(0), color(1, 1, 1, 1), clearColor(0, 0, 0, 0),
		poligonMode(GL_FILL), pRenderTarget(nullptr){}

	TBlendStateDesc blend;
	bool alphaTest;
	GLuint tex_ID_last_binded;
	TDepthStencilDesc depth;
	TColor4 color;
	TColor4 clearColor;
	GLint poligonMode; // GL_FILL GL_LINE
	GLboolean cullingOn; // GL_FALSE GL_TRUE
	GLint cullingMode; // GL_FRONT GL_BACK
	ICoreTexture *pRenderTarget;
};

struct FBO
{
	FBO() : ID(0), depth_renderbuffer_ID(0), width(0), height(0) {}

	GLuint ID;
	GLuint depth_renderbuffer_ID;
	int width, height;

	void Init();
	void GenerateDepthRenderbuffer(uint w, uint h);
	void Free();
};

class GL3XCoreRender final : public ICoreRenderer
{
	std::vector<GLShader> _shaders;
	std::stack<State> _states;
	TMatrix4x4 MV;
	TMatrix4x4 P;	
	TMatrix4x4 T;	
	GLuint tex_ID_last_binded;
	bool alphaTest;
	TColor4 _color;	
	TColor4 _clearColor;	

	ICoreTexture *pCurrentRenderTarget;
	std::vector<FBO> _fboPool;
	GLsizei viewportWidth, viewportHeight;
	GLint viewportX, viewportY;

	GLShader* chooseShader(INPUT_ATTRIBUTE attributes, bool texture_binded, bool light_on, bool is2d, bool alphaTest);

public:
	
	GL3XCoreRender(IEngineCore *pCore);
	
	DGLE_RESULT DGLE_API Prepare(TCrRndrInitResults &stResults) override;
	DGLE_RESULT DGLE_API Initialize(TCrRndrInitResults &stResults, TEngineWindow &stWin, E_ENGINE_INIT_FLAGS &eInitFlags) override;
	DGLE_RESULT DGLE_API Finalize() override;
	DGLE_RESULT DGLE_API AdjustMode(TEngineWindow &stNewWin) override;
	DGLE_RESULT DGLE_API MakeCurrent() override;
	DGLE_RESULT DGLE_API Present() override;

	DGLE_RESULT DGLE_API SetClearColor(const TColor4 &stColor) override;
	DGLE_RESULT DGLE_API GetClearColor(TColor4 &stColor) override;
	DGLE_RESULT DGLE_API Clear(bool bColor, bool bDepth, bool bStencil) override;
	DGLE_RESULT DGLE_API SetViewport(uint x, uint y, uint width, uint height) override;
	DGLE_RESULT DGLE_API GetViewport(uint &x, uint &y, uint &width, uint &height) override;
	DGLE_RESULT DGLE_API SetScissorRectangle(uint x, uint y, uint width, uint height) override;
	DGLE_RESULT DGLE_API GetScissorRectangle(uint &x, uint &y, uint &width, uint &height) override;
	DGLE_RESULT DGLE_API SetLineWidth(float fWidth) override;
	DGLE_RESULT DGLE_API GetLineWidth(float &fWidth) override;
	DGLE_RESULT DGLE_API SetPointSize(float fSize) override;
	DGLE_RESULT DGLE_API GetPointSize(float &fSize) override;
	DGLE_RESULT DGLE_API ReadFrameBuffer(uint uiX, uint uiY, uint uiWidth, uint uiHeight, uint8 *pData, uint uiDataSize, E_TEXTURE_DATA_FORMAT eDataFormat) override;
	DGLE_RESULT DGLE_API SetRenderTarget(ICoreTexture *pTexture) override;
	DGLE_RESULT DGLE_API GetRenderTarget(ICoreTexture *&prTexture) override;
	DGLE_RESULT DGLE_API CreateTexture(ICoreTexture *&prTex, const uint8* const pData, uint uiWidth, uint uiHeight, bool bMipmapsPresented, E_CORE_RENDERER_DATA_ALIGNMENT eDataAlignment, E_TEXTURE_DATA_FORMAT eDataFormat, E_TEXTURE_LOAD_FLAGS eLoadFlags) override;
	DGLE_RESULT DGLE_API CreateGeometryBuffer(ICoreGeometryBuffer *&prBuffer, const TDrawDataDesc &stDrawDesc, uint uiVerticesCount, uint uiIndicesCount, E_CORE_RENDERER_DRAW_MODE eMode, E_CORE_RENDERER_BUFFER_TYPE eType) override;
	DGLE_RESULT DGLE_API ToggleStateFilter(bool bEnabled) override;
	DGLE_RESULT DGLE_API InvalidateStateFilter() override;
	DGLE_RESULT DGLE_API PushStates() override;
	DGLE_RESULT DGLE_API PopStates() override;
	DGLE_RESULT DGLE_API SetMatrix(const TMatrix4x4 &stMatrix, E_MATRIX_TYPE eMatType) override;
	DGLE_RESULT DGLE_API GetMatrix(TMatrix4x4 &stMatrix, E_MATRIX_TYPE eMatType) override;
	DGLE_RESULT DGLE_API Draw(const TDrawDataDesc &stDrawDesc, E_CORE_RENDERER_DRAW_MODE eMode, uint uiCount) override;
	DGLE_RESULT DGLE_API DrawBuffer(ICoreGeometryBuffer *pBuffer) override;
	DGLE_RESULT DGLE_API SetColor(const TColor4 &stColor) override;
	DGLE_RESULT DGLE_API GetColor(TColor4 &stColor) override;
	DGLE_RESULT DGLE_API ToggleBlendState(bool bEnabled) override;
	DGLE_RESULT DGLE_API ToggleAlphaTestState(bool bEnabled) override;
	DGLE_RESULT DGLE_API SetBlendState(const TBlendStateDesc &stState) override;
	DGLE_RESULT DGLE_API GetBlendState(TBlendStateDesc &stState) override;
	DGLE_RESULT DGLE_API SetDepthStencilState(const TDepthStencilDesc &stState) override;
	DGLE_RESULT DGLE_API GetDepthStencilState(TDepthStencilDesc &stState) override;
	DGLE_RESULT DGLE_API SetRasterizerState(const TRasterizerStateDesc &stState) override;
	DGLE_RESULT DGLE_API GetRasterizerState(TRasterizerStateDesc &stState) override;
	DGLE_RESULT DGLE_API BindTexture(ICoreTexture *pTex, uint uiTextureLayer) override;
	DGLE_RESULT DGLE_API GetBindedTexture(ICoreTexture *&prTex, uint uiTextureLayer) override;
	DGLE_RESULT DGLE_API GetFixedFunctionPipelineAPI(IFixedFunctionPipeline *&prFFP) override;
	DGLE_RESULT DGLE_API GetDeviceMetric(E_CORE_RENDERER_METRIC_TYPE eMetric, int &iValue) override;
	DGLE_RESULT DGLE_API IsFeatureSupported(E_CORE_RENDERER_FEATURE_TYPE eFeature, bool &bIsSupported) override;
	DGLE_RESULT DGLE_API GetRendererType(E_CORE_RENDERER_TYPE &eType) override;

	// IEngineSubSystem
	DGLE_RESULT DGLE_API GetType(E_ENGINE_SUB_SYSTEM &eSubSystemType) override;

	IDGLE_BASE_IMPLEMENTATION(ICoreRenderer, INTERFACE_IMPL(IEngineSubSystem, INTERFACE_IMPL_END))
};

