#include "GL3XCoreRender.h"
#include <assert.h>
using std::vector;
using std::string;

#define SHADERS_DIRECTORY "shaders\\"

// Context create/delete functions
// and another platform specific stuff
extern bool CreateGL(TWindowHandle);
extern void MakeCurrent(); // realy need??
extern void FreeGL();
extern void SwapBuffer();


static GLsizei VertexCost(const TDrawDataDesc& stDrawDesc)
{
	return
		4 * (	// sizeof(float)											
			(stDrawDesc.bVertices2D ? 2 : 3) +
			(stDrawDesc.uiNormalOffset != -1 ? 3 : 0) +
			(stDrawDesc.uiTextureVertexOffset != -1 ? 2 : 0) +
			(stDrawDesc.uiColorOffset != -1 ? 4 : 0) +
			(stDrawDesc.uiTangentOffset != -1 ? 3 : 0) +
			(stDrawDesc.uiBinormalOffset != -1 ? 3 : 0));
}

vector<string> exact_lines(const char *str)
{
	vector<string> ret;
	char *search = const_cast<char*>(str);
	char *freeze = search;

	while (*search)
	{
		if (*search == '\n')
		{
			ret.push_back(string(freeze, search - freeze + 1));
			ret.back().back() = '\n';
			freeze = search + 1;
		}

		if (*search == '\r')
		{
			if (*(search + 1) == '\n')
			{
				ret.push_back(string(freeze, search - freeze + 1));
				ret.back().back() = '\n';
				freeze = search + 2;
				search++; // Note the 2! CRLF is 2 characters!
			}
			else
			{
				ret.push_back(string(freeze, search - freeze));
				ret.back().back() = '\n';
				freeze = search + 1;
			}
		}
		search++;
	}

	ret.push_back(string(freeze, search - freeze + 1));
	ret.back().back() = '\n';

	return ret;
}

vector<const char*> make_ptr_vector(const vector<string>& str_list)
{
	vector<const char*> v{ str_list.size() + 1, nullptr };
	for (size_t i = 0; i < str_list.size(); i++) 
		v[i] = str_list[i].c_str();
	return v;
}

////////////////////////////
//    GLGeometryBuffer    //
////////////////////////////

class GLGeometryBuffer final : public ICoreGeometryBuffer
{
	bool _bEmpty;
	GLsizei _vertexCount;
	GLsizei _indexCount;
	GLuint _vao;
	GLuint _vbo;
	GLuint _ibo;
	E_CORE_RENDERER_BUFFER_TYPE _eBufferType;
	E_CORE_RENDERER_DRAW_MODE _eDrawMode;

public:

	inline GLuint VAO_ID() { return _vao; }
	inline bool IndexDrawing() { return _ibo > 0; }
	inline GLsizei VertexCount() { return _vertexCount; }
	inline GLsizei IndexCount() { return _indexCount; }

	GLGeometryBuffer(E_CORE_RENDERER_BUFFER_TYPE eType, bool indexBuffer) :
		_bEmpty(true), _vertexCount(0), _indexCount(0), _vao(0), _vbo(0), _ibo(0), _eBufferType(eType)
	{		
		glGenVertexArrays(1, &_vao);
		glGenBuffers(1, &_vbo);	
		if (indexBuffer) glGenBuffers(1, &_ibo);
		
		LOG("GLGeometryBuffer()");
	}

	~GLGeometryBuffer()
	{		
		if (_ibo!=0) glDeleteBuffers(1, &_ibo);
		glDeleteBuffers(1, &_vbo);
		glDeleteVertexArrays(1, &_vao);

		LOG("~GLGeometryBuffer()");
	}

	DGLE_RESULT DGLE_API GetGeometryData(TDrawDataDesc& stDesc, uint uiVerticesDataSize, uint uiIndexesDataSize) override {return S_OK;}
	DGLE_RESULT DGLE_API SetGeometryData(const TDrawDataDesc& stDrawDesc, uint uiVerticesDataSize, uint uiIndexesDataSize) override	
	{
		if (_bEmpty)
		{
			if (_eBufferType == CRBT_SOFTWARE) return E_FAIL; // not implemented
			const GLenum glBufferType = _eBufferType == CRBT_HARDWARE_STATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

			glBindVertexArray(_vao);

			glBindBuffer(GL_ARRAY_BUFFER, _vbo);
			glBufferData(GL_ARRAY_BUFFER, uiVerticesDataSize, reinterpret_cast<const void*>(stDrawDesc.pData), glBufferType); // send data to VRAM

			// Shader attrubute mapping:
			// 0 - position
			// 1 - normal
			// 2 - texture coordinates
			// 3 - color

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, stDrawDesc.bVertices2D ? 2 : 3, GL_FLOAT, GL_FALSE, stDrawDesc.uiVertexStride , reinterpret_cast<void*>(0));
			auto ptr = stDrawDesc.pData + stDrawDesc.uiNormalOffset;
			if (stDrawDesc.uiNormalOffset != -1)
			{
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stDrawDesc.uiNormalStride, reinterpret_cast<void*>(stDrawDesc.uiNormalOffset));
			}
			if (stDrawDesc.uiTextureVertexOffset != -1)
			{
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stDrawDesc.uiTextureVertexStride, reinterpret_cast<void*>(stDrawDesc.uiTextureVertexOffset));
			}
			//if (stDrawDesc.uiColorOffset != -1)
			//{
			//	glEnableVertexAttribArray(3);
			//	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stDrawDesc.uiColorStride, reinterpret_cast<void*>(stDrawDesc.uiColorOffset));
			//}
			// ...
			// TODO: implement tangent and binormal

			if (uiIndexesDataSize > 0)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiIndexesDataSize, reinterpret_cast<const void*>(stDrawDesc.pIndexBuffer), glBufferType); // send data to VRAM
			}

			glBindVertexArray(0);
			_bEmpty = false;
		}
		else // update data in buffer
		{			
		}

		return S_OK;
	}
	DGLE_RESULT DGLE_API Reallocate(const TDrawDataDesc& stDrawDesc, uint uiVerticesCount, uint uiIndicesCount, E_CORE_RENDERER_DRAW_MODE eMode) override // reduant?
	{
		_eDrawMode = eMode;
		_vertexCount = uiVerticesCount;
		_indexCount = uiIndicesCount;

		const GLsizei v_cost = VertexCost(stDrawDesc);
		const GLsizei vertex_data_bytes = uiVerticesCount * v_cost;
		const GLsizei indexes_data_bytes = uiIndicesCount * (stDrawDesc.bIndexBuffer32 ? sizeof(uint32) : sizeof(uint16));

		return SetGeometryData(stDrawDesc, vertex_data_bytes, indexes_data_bytes);		
	}
	DGLE_RESULT DGLE_API GetBufferDimensions(uint& uiVerticesDataSize, uint& uiVerticesCount, uint& uiIndexesDataSize, uint& uiIndexesCount) override {return S_OK;}
	DGLE_RESULT DGLE_API GetBufferDrawDataDesc(TDrawDataDesc& stDesc) override {return S_OK;}
	DGLE_RESULT DGLE_API GetBufferDrawMode(E_CORE_RENDERER_DRAW_MODE& eMode) override {return S_OK;}
	DGLE_RESULT DGLE_API GetBufferType(E_CORE_RENDERER_BUFFER_TYPE& eType) override {return S_OK;}
	DGLE_RESULT DGLE_API GetBaseObject(IBaseRenderObjectContainer*& prObj) override	{return S_OK;}
	DGLE_RESULT DGLE_API Free() override 
	{
		delete this;
		return S_OK;
	}

	IDGLE_BASE_IMPLEMENTATION(ICoreGeometryBuffer, INTERFACE_IMPL_END)
};


////////////////////////////
//        GLTexture       //
////////////////////////////

class GLTexture final : public ICoreTexture
{
	bool _bCreated = true;

public:

	inline bool IsCreated() { return _bCreated; }

	GLTexture(const uint8* pData, uint uiWidth, uint uiHeight, bool bMipmapsPresented, E_CORE_RENDERER_DATA_ALIGNMENT eDataAlignment, E_TEXTURE_DATA_FORMAT eDataFormat, E_TEXTURE_LOAD_FLAGS eLoadFlags)
	{
		LOG("GLTexture()");
	}
	~GLTexture()
	{
		LOG("~GLTexture()");
	}

	DGLE_RESULT DGLE_API GetSize(uint& width, uint& height) override {return S_OK;}
	DGLE_RESULT DGLE_API GetDepth(uint& depth) override {return S_OK;}
	DGLE_RESULT DGLE_API GetType(E_TEXTURE_TYPE& eType) override {return S_OK;}
	DGLE_RESULT DGLE_API GetFormat(E_TEXTURE_DATA_FORMAT& eFormat) override {return S_OK;}
	DGLE_RESULT DGLE_API GetLoadFlags(E_TEXTURE_LOAD_FLAGS& eLoadFlags) override {return S_OK;}
	DGLE_RESULT DGLE_API GetPixelData(uint8* pData, uint& uiDataSize, uint uiLodLevel) override {return S_OK;}
	DGLE_RESULT DGLE_API SetPixelData(const uint8* pData, uint uiDataSize, uint uiLodLevel) override {return S_OK;}
	DGLE_RESULT DGLE_API Reallocate(const uint8* pData, uint uiWidth, uint uiHeight, bool bMipMaps, E_TEXTURE_DATA_FORMAT eDataFormat) override {return S_OK;}
	DGLE_RESULT DGLE_API GetBaseObject(IBaseRenderObjectContainer*& prObj) override {return S_OK;}
	DGLE_RESULT DGLE_API Free() override
	{
		delete this;
		return S_OK;
	}


	IDGLE_BASE_IMPLEMENTATION(ICoreTexture, INTERFACE_IMPL_END)
};


//////////////////////////
//    GL3XCoreRender    //
//////////////////////////

GL3XCoreRender::GL3XCoreRender(IEngineCore *pCore) : _core(pCore), _programID(0), _fragID(0), _vertID(0)
{
}

DGLE_RESULT DGLE_API GL3XCoreRender::Prepare(TCrRndrInitResults& stResults)
{ 	

	return S_OK;
}

void _checkShaderError(uint id, GLenum constant)
{
	int iStatus;

	if (constant == GL_COMPILE_STATUS)
		glGetShaderiv(id, GL_COMPILE_STATUS, &iStatus);
	else if (constant == GL_LINK_STATUS)
		glGetProgramiv(id, GL_LINK_STATUS, &iStatus);

	if (iStatus == GL_FALSE)
	{
		char * buf;
		GLint maxLenght = 0;
		if (constant == GL_COMPILE_STATUS)
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLenght);
		else if (constant == GL_LINK_STATUS)
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxLenght);
		buf = new char[maxLenght];

		if (constant == GL_COMPILE_STATUS)
			glGetShaderInfoLog(id, maxLenght, &maxLenght, buf);
		else if (constant == GL_LINK_STATUS)
			glGetProgramInfoLog(id, maxLenght, &maxLenght, buf);

		assert(false);
		delete buf;
	}
}

void GL3XCoreRender::_load_and_compile_shader(const char* filename, GLenum type)
{
	IMainFileSystem *pFileSystem;
	_core->GetSubSystem(ESS_FILE_SYSTEM, reinterpret_cast<IEngineSubSystem *&>(pFileSystem));

	IFile *pFile;
	pFileSystem->LoadFile(filename, pFile);

	uint32 size;
	pFile->GetSize(size);

	string buf(size + 1, '\0');
	uint read;
	pFile->Read(&buf[0], size, read);
	pFileSystem->FreeFile(pFile);

	auto string_vec = exact_lines(buf.c_str());
	auto ptr_vec = make_ptr_vector(string_vec);

	GLuint& shd = type == GL_VERTEX_SHADER ? _vertID : _fragID;
	shd = glCreateShader(type);
	glShaderSource(shd, ptr_vec.size() - 1, &ptr_vec[0], nullptr);
	glCompileShader(shd);
	_checkShaderError(shd, GL_COMPILE_STATUS);
}
DGLE_RESULT DGLE_API GL3XCoreRender::Initialize(TCrRndrInitResults& stResults, TEngineWindow& stWin, E_ENGINE_INIT_FLAGS& eInitFlags)
{ 
	TWindowHandle handle;
	_core->GetWindowHandle(handle);
	stResults = CreateGL(handle);
	if (!stResults) return E_FAIL;


	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	_programID = glCreateProgram();

	_load_and_compile_shader(SHADERS_DIRECTORY"camera_vert.shader", GL_VERTEX_SHADER);
	_load_and_compile_shader(SHADERS_DIRECTORY"camera_frag.shader", GL_FRAGMENT_SHADER);

	glAttachShader(_programID, _vertID);
	glAttachShader(_programID, _fragID);

	glLinkProgram(_programID);
	_checkShaderError(_programID, GL_LINK_STATUS);

	return stResults ? S_OK : E_FAIL;
}

DGLE_RESULT DGLE_API GL3XCoreRender::Finalize()
{ 
	if (_vertID != 0) glDeleteShader(_vertID);
	if (_fragID != 0) glDeleteShader(_fragID);
	if (_programID != 0) glDeleteProgram(_programID);
	FreeGL();

	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::AdjustMode(TEngineWindow& stNewWin)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::MakeCurrent()
{ 
	::MakeCurrent();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::Present()
{ 
	SwapBuffer();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetClearColor(const TColor4& stColor)
{ 
	glClearColor(stColor.r, stColor.g, stColor.b, stColor.a);
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetClearColor(TColor4& stColor)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::Clear(bool bColor, bool bDepth, bool bStencil)
{ 
	GLbitfield mask = 0;
	if (bColor)	mask |= GL_COLOR_BUFFER_BIT;
	if (bDepth)	mask |= GL_DEPTH_BUFFER_BIT;
	if (bStencil) mask |= GL_STENCIL_BUFFER_BIT;
	glClear(mask);
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetViewport(uint x, uint y, uint width, uint height)
{ 
	glViewport(x, y, width, height);
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetViewport(uint& x, uint& y, uint& width, uint& height)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetScissorRectangle(uint x, uint y, uint width, uint height)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetScissorRectangle(uint& x, uint& y, uint& width, uint& height)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetLineWidth(float fWidth)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetLineWidth(float& fWidth)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetPointSize(float fSize)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetPointSize(float& fSize)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::ReadFrameBuffer(uint uiX, uint uiY, uint uiWidth, uint uiHeight, uint8* pData, uint uiDataSize, E_TEXTURE_DATA_FORMAT eDataFormat)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetRenderTarget(ICoreTexture* pTexture)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetRenderTarget(ICoreTexture*& prTexture)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::CreateTexture(ICoreTexture*& prTex, const uint8* pData, uint uiWidth, uint uiHeight, bool bMipmapsPresented, E_CORE_RENDERER_DATA_ALIGNMENT eDataAlignment, E_TEXTURE_DATA_FORMAT eDataFormat, E_TEXTURE_LOAD_FLAGS eLoadFlags)
{ 
	auto texture = new GLTexture(pData, uiWidth, uiHeight, bMipmapsPresented, eDataAlignment, eDataFormat, eLoadFlags);
	if (!texture->IsCreated())
	{
		delete texture;
		return S_FALSE;
	}

	prTex = texture;
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::CreateGeometryBuffer(ICoreGeometryBuffer*& prBuffer, const TDrawDataDesc& stDrawDesc, uint uiVerticesCount, uint uiIndicesCount, E_CORE_RENDERER_DRAW_MODE eMode, E_CORE_RENDERER_BUFFER_TYPE eType)
{ 
	auto buffer = new GLGeometryBuffer(eType, uiIndicesCount > 0);
	auto ret = buffer->Reallocate(stDrawDesc, uiVerticesCount, uiIndicesCount, eMode);

	prBuffer = buffer;

	return ret;
}

DGLE_RESULT DGLE_API GL3XCoreRender::ToggleStateFilter(bool bEnabled)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::InvalidateStateFilter()
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::PushStates()
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::PopStates()
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetMatrix(const TMatrix4x4& stMatrix, E_MATRIX_TYPE eMatType)
{ 
	switch (eMatType)
	{
		case MT_MODELVIEW: modelViewMat = stMatrix; break;
		case MT_PROJECTION: projectionMat = stMatrix; break;
		case MT_TEXTURE: // TODO:  
			break;
	}

	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetMatrix(TMatrix4x4& stMatrix, E_MATRIX_TYPE eMatType)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::Draw(const TDrawDataDesc& stDrawDesc, E_CORE_RENDERER_DRAW_MODE eMode, uint uiCount)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::DrawBuffer(ICoreGeometryBuffer* pBuffer)
{ 
	GLGeometryBuffer *b = static_cast<GLGeometryBuffer*>(pBuffer);
	if (b == nullptr) return S_OK;

	glUseProgram(_programID);

	TMatrix4x4 MVP = modelViewMat * projectionMat;
	const GLuint ID = glGetUniformLocation(_programID, "MVP");
	glUniformMatrix4fv(ID, 1, GL_FALSE, &MVP._1D[0]);


	glBindVertexArray(b->VAO_ID());

	if (b->IndexDrawing())
		glDrawElements(GL_TRIANGLES, b->IndexCount(), ((b->IndexCount() > 65535) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT), nullptr);
	else if (b->VertexCount() > 0)
		glDrawArrays(GL_TRIANGLES, 0, b->VertexCount());

	glBindVertexArray(0);

	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetColor(const TColor4& stColor)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetColor(TColor4& stColor)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::ToggleBlendState(bool bEnabled)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::ToggleAlphaTestState(bool bEnabled)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetBlendState(const TBlendStateDesc& stState)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetBlendState(TBlendStateDesc& stState)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetDepthStencilState(const TDepthStencilDesc& stState)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetDepthStencilState(TDepthStencilDesc& stState)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetRasterizerState(const TRasterizerStateDesc& stState)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetRasterizerState(TRasterizerStateDesc& stState)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::BindTexture(ICoreTexture* pTex, uint uiTextureLayer)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetBindedTexture(ICoreTexture*& prTex, uint uiTextureLayer)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetFixedFunctionPipelineAPI(IFixedFunctionPipeline*& prFFP)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetDeviceMetric(E_CORE_RENDERER_METRIC_TYPE eMetric, int& iValue)
{
	iValue = 0;
	switch (eMetric)
	{
		case CRMT_MAX_TEXTURE_RESOLUTION: glGetIntegerv(GL_MAX_TEXTURE_SIZE, &iValue); break;
		case CRMT_MAX_TEXTURE_LAYERS: glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &iValue); break;
		case CRMT_MAX_ANISOTROPY_LEVEL: if (GLEW_EXT_texture_filter_anisotropic) glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &iValue); break;
		default: break;
	}
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::IsFeatureSupported(E_CORE_RENDERER_FEATURE_TYPE eFeature, bool& bIsSupported)
{ 
	bIsSupported = false;
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetRendererType(E_CORE_RENDERER_TYPE& eType)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetType(E_ENGINE_SUB_SYSTEM& eSubSystemType)
{
	eSubSystemType = ESS_CORE_RENDERER;
	return S_OK;
}
