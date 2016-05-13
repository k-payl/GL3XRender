#pragma once
#include "Common.h"
#include "GL/glew.h"


class GL3XCoreRender final : public ICoreRenderer
{
	IEngineCore *_core;
	GLuint _programID;
	GLuint _fragID;
	GLuint _vertID;
	TMatrix4x4 modelViewMat;
	TMatrix4x4 projectionMat;

	void _load_and_compile_shader(const char* filename, GLenum type);

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

