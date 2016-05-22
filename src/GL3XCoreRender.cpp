/**
\author		Konstantin Pajl aka Consta
\date		21.05.2016 (c)Andrey Korotkov

This file is a part of DGLE project and is distributed
under the terms of the GNU Lesser General Public License.
See "DGLE.h" for more details.
*/

#include "GL3XCoreRender.h"
#include <vector>
#include <assert.h>
using namespace std;


#define SHADERS_DIRECTORY "..\\resources\\shaders\\"
#define LOG_INFO(txt) LogToDGLE(string(txt).c_str(), LT_INFO, __FILE__, __LINE__)

static IEngineCore *_core;
// TODO: put shader text here 

extern bool CreateGL(TWindowHandle hwnd, IEngineCore* pCore, const TEngineWindow& stWin);
extern void MakeCurrent(); // realy need??
extern void FreeGL();
extern void SwapBuffer();

static void LogToDGLE(const char *pcTxt, E_LOG_TYPE eType, const char *pcSrcFileName, int iSrcLineNumber)
{
	_core->WriteToLogEx(pcTxt, eType, pcSrcFileName, iSrcLineNumber);
}

static GLsizei vertexSize(const TDrawDataDesc& stDrawDesc)
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

static vector<string> exact_lines(const char *str)
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

static vector<const char*> make_ptr_vector(const vector<string>& str_list)
{
	vector<const char*> v{ str_list.size() + 1, nullptr };
	for (size_t i = 0; i < str_list.size(); i++) 
		v[i] = str_list[i].c_str();
	return v;
}

static void checkShaderError(uint id, GLenum constant)
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


////////////////////////////
//        Geometry        //
////////////////////////////

class GLGeometryBuffer final : public ICoreGeometryBuffer
{
	bool _bNotInitalizedCorrectlyYet;
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
		_bNotInitalizedCorrectlyYet(true), _vertexCount(0), _indexCount(0), _vao(0), _vbo(0), _ibo(0), _eBufferType(eType)
	{		
		glGenVertexArrays(1, &_vao);
		glGenBuffers(1, &_vbo);	
		if (indexBuffer) glGenBuffers(1, &_ibo);
		
		LOG_INFO("GLGeometryBuffer()");
	}

	~GLGeometryBuffer()
	{		
		if (_ibo!=0) glDeleteBuffers(1, &_ibo);
		glDeleteBuffers(1, &_vbo);
		glDeleteVertexArrays(1, &_vao);

		LOG_INFO("~GLGeometryBuffer()");
	}

	DGLE_RESULT DGLE_API GetGeometryData(TDrawDataDesc& stDesc, uint uiVerticesDataSize, uint uiIndexesDataSize) override {return S_OK;}
	DGLE_RESULT DGLE_API SetGeometryData(const TDrawDataDesc& stDrawDesc, uint uiVerticesDataSize, uint uiIndexesDataSize) override	
	{
		if (_bNotInitalizedCorrectlyYet)
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
				glEnableVertexAttribArray(3);
				void *np = reinterpret_cast<void*>(stDrawDesc.pData + stDrawDesc.uiNormalOffset);
				glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stDrawDesc.uiNormalStride, reinterpret_cast<void*>(stDrawDesc.uiNormalOffset));
			}
			if (stDrawDesc.uiTextureVertexOffset != -1)
			{
				glEnableVertexAttribArray(2);		
				void *tp = reinterpret_cast<void*>(stDrawDesc.pData + stDrawDesc.uiTextureVertexOffset);
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
			_bNotInitalizedCorrectlyYet = false;
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

		const GLsizei v_cost = vertexSize(stDrawDesc);
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
//         Texture        //
////////////////////////////

class GLTexture final : public ICoreTexture
{
	bool _bCreated = true;

public:

	inline bool IsCreated() { return _bCreated; }

	GLTexture(const uint8* pData, uint uiWidth, uint uiHeight, bool bMipmapsPresented, E_CORE_RENDERER_DATA_ALIGNMENT eDataAlignment, E_TEXTURE_DATA_FORMAT eDataFormat, E_TEXTURE_LOAD_FLAGS eLoadFlags)
	{
		LOG_INFO("GLTexture()");
	}
	~GLTexture()
	{
		LOG_INFO("~GLTexture()");
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
//         Render       //
//////////////////////////

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
	checkShaderError(shd, GL_COMPILE_STATUS);
}

GL3XCoreRender::GL3XCoreRender(IEngineCore *pCore) : _programID(0), _fragID(0), _vertID(0)
{
	_core = pCore;
}

DGLE_RESULT DGLE_API GL3XCoreRender::Prepare(TCrRndrInitResults& stResults)
{ 	
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::Initialize(TCrRndrInitResults& stResults, TEngineWindow& stWin, E_ENGINE_INIT_FLAGS& eInitFlags)
{ 
	TWindowHandle handle;
	_core->GetWindowHandle(handle);
	if (!CreateGL(handle, _core, stWin)) return E_FAIL;

	#define OGLI "Initialized at OpenGL " 
	GLint major, minor;
	char buffer[sizeof(OGLI) + 4];	
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	sprintf(buffer, OGLI"%i.%i", major, minor);
	LOG_INFO(string(buffer));

	_programID = glCreateProgram();
	_load_and_compile_shader(SHADERS_DIRECTORY"camera_vert.shader", GL_VERTEX_SHADER);
	_load_and_compile_shader(SHADERS_DIRECTORY"camera_frag.shader", GL_FRAGMENT_SHADER);
	glAttachShader(_programID, _vertID);
	glAttachShader(_programID, _fragID);
	glLinkProgram(_programID);
	checkShaderError(_programID, GL_LINK_STATUS);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glClearDepth(1.0);

	return S_OK;
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
	if (bColor) mask |= GL_COLOR_BUFFER_BIT;
	if (bDepth) mask |= GL_DEPTH_BUFFER_BIT;
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
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	x = vp[0]; y = vp[1];
	width = vp[2]; height = vp[3];

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
		case MT_MODELVIEW: MV = stMatrix; break;
		case MT_PROJECTION: P = stMatrix; break;
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

	TMatrix4x4 MVP = MV * P;
	const GLuint MVP_ID = glGetUniformLocation(_programID, "MVP");
	glUniformMatrix4fv(MVP_ID, 1, GL_FALSE, &MVP._1D[0]);

	const GLuint MV_ID = glGetUniformLocation(_programID, "MV");
	glUniformMatrix4fv(MV_ID, 1, GL_FALSE, &MV._1D[0]);

	// Normal matrix = (MV^-1)^T
	TMatrix4x4 NM = MatrixTranspose(MatrixInverse(MV));
	const GLuint Norm_ID = glGetUniformLocation(_programID, "NM");
	glUniformMatrix4fv(Norm_ID, 1, GL_FALSE, &NM._1D[0]);

	const TVector3 L = { 0.2f, 0.7f, 3.5f };
	const TVector3 nL = L / L.Length();
	const TVector3 nL_eyeSpace = MV.ApplyToVector(nL);
	const GLuint nL_ID = glGetUniformLocation(_programID, "nL");
	glUniform3f(nL_ID, nL_eyeSpace.x, nL_eyeSpace.y, nL_eyeSpace.z);

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
