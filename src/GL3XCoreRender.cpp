/**
\author		Konstantin Pajl aka Consta
\date		12.06.2016 (c)Andrey Korotkov

This file is a part of DGLE project and is distributed
under the terms of the GNU Lesser General Public License.
See "DGLE.h" for more details.
*/

#include "GL3XCoreRender.h"
#include <assert.h>
#include <algorithm>
using namespace std;

#define LOG_INFO(txt) LogToDGLE((string("GL3XCoreRender: ") + txt).c_str(), LT_INFO, __FILE__, __LINE__)
#define LOG_WARNING(txt) LogToDGLE(std::string(txt).c_str(), LT_WARNING, __FILE__, __LINE__)
void E_GUARDS()
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		string error;

			switch (err) {
			case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
			case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
			case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
			case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
		}
		assert(err == GL_NO_ERROR);
	}
}

extern bool CreateGL(TWindowHandle hwnd, IEngineCore* pCore, const TEngineWindow& stWin);
extern void MakeCurrent();
extern void FreeGL();
extern void SwapBuffer();

static IEngineCore *_core;

#include "shaderSources.h"

#pragma warning(push)
#pragma warning(disable:4715)
inline GLenum BlendFactor_DGLE_2_GL(E_BLEND_FACTOR dgleFactor)
{
	switch (dgleFactor)
	{
		case BF_ZERO:					return GL_ZERO;
		case BF_ONE:					return GL_ONE;
		case BF_SRC_COLOR:				return GL_SRC_COLOR;
		case BF_SRC_ALPHA:				return GL_SRC_ALPHA;
		case BF_DST_COLOR:				return GL_DST_COLOR;
		case BF_DST_ALPHA:				return GL_DST_ALPHA;
		case BF_ONE_MINUS_SRC_COLOR:	return GL_ONE_MINUS_SRC_COLOR;
		case BF_ONE_MINUS_SRC_ALPHA:	return GL_ONE_MINUS_SRC_ALPHA;
		default:
			assert(false);
	}
}

inline E_BLEND_FACTOR BlendFactor_GL_2_DGLE(GLenum glFactor)
{
	switch (glFactor)
	{
		case GL_ZERO:				return BF_ZERO;
		case GL_ONE:				return BF_ONE;
		case GL_SRC_COLOR:			return BF_SRC_COLOR;
		case GL_SRC_ALPHA:			return BF_SRC_ALPHA;
		case GL_DST_COLOR:			return BF_DST_COLOR;
		case GL_DST_ALPHA:			return BF_DST_ALPHA;
		case GL_ONE_MINUS_SRC_COLOR:return BF_ONE_MINUS_SRC_COLOR;
		case GL_ONE_MINUS_SRC_ALPHA:return BF_ONE_MINUS_SRC_ALPHA;
		default:
			assert(false);
	}
}
#pragma warning(pop)

static void LogToDGLE(const char *pcTxt, E_LOG_TYPE eType, const char *pcSrcFileName, int iSrcLineNumber)
{
	_core->WriteToLogEx(pcTxt, eType, pcSrcFileName, iSrcLineNumber);
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

void GLShader::Init(const ShaderSrc& parent)
{
	E_GUARDS();
	p = &parent;
	LOG_INFO("GLShader() for ");
	programID = glCreateProgram();
	vertID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertID, p->linesVertexShader, p->ppTxtVertex, nullptr);
	glCompileShader(vertID);
	checkShaderError(vertID, GL_COMPILE_STATUS);
	fragID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragID, p->linesFragmentShader, p->ppTxtFragment, nullptr);
	glCompileShader(fragID);
	checkShaderError(fragID, GL_COMPILE_STATUS);
	glAttachShader(programID, vertID);
	glAttachShader(programID, fragID);
	glLinkProgram(programID);
	checkShaderError(programID, GL_LINK_STATUS);
	E_GUARDS();
}

void GLShader::Free()
{
	LOG_INFO("~GLShader()");
	E_GUARDS();
	if (vertID != 0) glDeleteShader(vertID);
	if (fragID != 0) glDeleteShader(fragID);
	if (programID != 0) glDeleteProgram(programID);
	E_GUARDS();
}

bool GLShader::bPositionIs2D() const { return p->bPositionIs2D; }
bool GLShader::bInputNormals() const { return (p->attribs & CGAP_NORM) != CGAP_NONE; }
bool GLShader::bInputTextureCoords() const { return (p->attribs & CGAP_TEX) != CGAP_NONE; }
bool GLShader::bUniformNM() const { return p->bUniformNM; }
bool GLShader::bUniformnL() const { return p->bUniformnL; }
bool GLShader::bUniformTexture0() const { return p->bUniformTexture; }

bool GLShader::operator<(const GLShader & r) const
{
	auto i = hash();
	auto j = r.hash();
	return i < j;
}

bool GLShader::operator==(const GLShader & r) const
{
	return hash() == r.hash();
}

unsigned int GLShader::hash() const
{
	unsigned int y = 0;
	y = (int)bInputNormals() +
		((int)bInputTextureCoords() << 1) +
		((int)bPositionIs2D() << 2);
	return y;
}

static void getGLFormats(E_TEXTURE_DATA_FORMAT eDataFormat, GLint& VRAMFormat, GLenum& sourceFormat)
{
	switch (eDataFormat)
	{
		case TDF_RGB8:				VRAMFormat = GL_RGB8;	sourceFormat = GL_RGB;  break;
		case TDF_RGBA8:				VRAMFormat = GL_RGB8;	sourceFormat = GL_RGBA; break;
		case TDF_ALPHA8:			VRAMFormat = GL_R8;		sourceFormat = GL_RED; break;
		case TDF_BGR8:				VRAMFormat = GL_RGB8;	sourceFormat = GL_BGR; break;
		case TDF_BGRA8:				VRAMFormat = GL_RGBA8;	sourceFormat = GL_BGRA; break;
		//case TDF_DXT1:				break; ??
		//case TDF_DXT5: break;
		//case TDF_DEPTH_COMPONENT24: break;
		//case TDF_DEPTH_COMPONENT32: break;
		default: assert(false); break;
	}
}


////////////////////////////
//        Geometry        //
////////////////////////////

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
	CORE_GEOMETRY_ATTRIBUTES_PRESENTED _attribs_presented;
	int activated_attributes[3];
	bool _b2dPosition;

public:

	const static int POSITION_ATTRIBUTE = 0;
	const static int NORMALS_ATTRIBUTE = 1;
	const static int TEXCOORDS_ATTRIBUTE = 2;

	inline GLuint VAO_ID() { return _vao; }
	inline bool IndexDrawing() { return _ibo > 0; }
	inline GLsizei VertexCount() { return _vertexCount; }
	inline GLsizei IndexCount() { return _indexCount; }
	inline CORE_GEOMETRY_ATTRIBUTES_PRESENTED GetAttributes() { return _attribs_presented; }
	inline bool Is2dPosition() { return _b2dPosition; }
	inline GLenum GLDrawMode()
	{
		GLenum mode;
		switch (_eDrawMode)
		{
			case CRDM_POINTS: mode = GL_POINTS; break;
			case CRDM_LINES: mode = GL_LINES; break;
			case CRDM_LINE_STRIP: mode = GL_LINE_STRIP; break;
			case CRDM_TRIANGLES: mode = GL_TRIANGLES; break;
			case CRDM_TRIANGLE_STRIP: mode = GL_TRIANGLE_STRIP; break;
			case CRDM_TRIANGLE_FAN: mode = GL_TRIANGLE_FAN; break;
		}
		return mode;
	}
	inline void ToggleAttribInVAO(int attrib_ind, bool value)
	{
		E_GUARDS();
		if (value && !activated_attributes[attrib_ind])
		{
			activated_attributes[attrib_ind] = true;
			glEnableVertexAttribArray(attrib_ind);
		}
		else if (!value && activated_attributes[attrib_ind])
		{
			activated_attributes[attrib_ind] = false;
			glDisableVertexAttribArray(attrib_ind);
		}
		E_GUARDS();
	}

	GLsizei vertexSize(const TDrawDataDesc& stDrawDesc)
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

	GLGeometryBuffer(E_CORE_RENDERER_BUFFER_TYPE eType, bool indexBuffer, GL3XCoreRender *pRnd) :
		_bAlreadyInitalized(false), _vertexCount(0), _indexCount(0), _vao(0), _vbo(0), _ibo(0), _eBufferType(eType), _pRnd(pRnd), _attribs_presented(CGAP_NONE), activated_attributes{0}, _b2dPosition(false)
	{		
		E_GUARDS();
		glGenVertexArrays(1, &_vao);
		glGenBuffers(1, &_vbo);	
		if (indexBuffer) glGenBuffers(1, &_ibo);
		//LOG_INFO("GLGeometryBuffer()");
		E_GUARDS();
	}

	~GLGeometryBuffer()
	{		
		E_GUARDS();
		if (_ibo!=0) glDeleteBuffers(1, &_ibo);
		glDeleteBuffers(1, &_vbo);
		glDeleteVertexArrays(1, &_vao);
		//LOG_INFO("~GLGeometryBuffer()");
		E_GUARDS();
	}

	DGLE_RESULT DGLE_API GetGeometryData(TDrawDataDesc& stDesc, uint uiVerticesDataSize, uint uiIndexesDataSize) override {return S_OK;}
	DGLE_RESULT DGLE_API SetGeometryData(const TDrawDataDesc& stDrawDesc, uint uiVerticesDataSize, uint uiIndexesDataSize) override	{ return S_OK; } // what is purpose if Reallocate() exists?
	DGLE_RESULT DGLE_API Reallocate(const TDrawDataDesc& stDrawDesc, uint uiVerticesCount, uint uiIndicesCount, E_CORE_RENDERER_DRAW_MODE eMode) override 
	{
		E_GUARDS();
		_eDrawMode = eMode;
		_vertexCount = uiVerticesCount;
		_indexCount = uiIndicesCount;
		_vertexBytes = vertexSize(stDrawDesc);
		const GLsizei vertex_data_bytes = uiVerticesCount * _vertexBytes;
		_indexBytes = (stDrawDesc.bIndexBuffer32 ? sizeof(uint32) : sizeof(uint16));
		const GLsizei indexes_data_bytes = uiIndicesCount * _indexBytes;
		_b2dPosition = stDrawDesc.bVertices2D;

		if (!_bAlreadyInitalized)
		{
			if (_eBufferType == CRBT_SOFTWARE) return E_FAIL; // not implemented

			glBindVertexArray(_vao);
			glBindBuffer(GL_ARRAY_BUFFER, _vbo);

			const GLenum glBufferType = _eBufferType == CRBT_HARDWARE_STATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
			glBufferData(GL_ARRAY_BUFFER, vertex_data_bytes, reinterpret_cast<const void*>(stDrawDesc.pData), glBufferType); // send data to VRAM

			glVertexAttribPointer(0, _b2dPosition ? 2 : 3, GL_FLOAT, GL_FALSE, stDrawDesc.uiVertexStride, reinterpret_cast<void*>(0));
			ToggleAttribInVAO(POSITION_ATTRIBUTE, true);
			_attribs_presented = static_cast<CORE_GEOMETRY_ATTRIBUTES_PRESENTED>(_attribs_presented | CGAP_POS);

			if (stDrawDesc.uiNormalOffset != -1)
			{
				glVertexAttribPointer(NORMALS_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, stDrawDesc.uiNormalStride, reinterpret_cast<void*>(stDrawDesc.uiNormalOffset));
				ToggleAttribInVAO(NORMALS_ATTRIBUTE, true);
				_attribs_presented = static_cast<CORE_GEOMETRY_ATTRIBUTES_PRESENTED>(_attribs_presented | CGAP_NORM);
			}
			if (stDrawDesc.uiTextureVertexOffset != -1)
			{
				glVertexAttribPointer(TEXCOORDS_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, stDrawDesc.uiTextureVertexStride, reinterpret_cast<void*>(stDrawDesc.uiTextureVertexOffset));
				ToggleAttribInVAO(TEXCOORDS_ATTRIBUTE, true);
				_attribs_presented = static_cast<CORE_GEOMETRY_ATTRIBUTES_PRESENTED>(_attribs_presented | CGAP_TEX);
			}
			assert(_attribs_presented & CGAP_POS);
			// TODO: implement tangent and binormal

			if (indexes_data_bytes > 0)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes_data_bytes, reinterpret_cast<const void*>(stDrawDesc.pIndexBuffer), glBufferType); // send data to VRAM
			}

			glBindVertexArray(0);
			_bAlreadyInitalized = true;
		}
		else // update data in buffer
		{ // TODO
		}
		E_GUARDS();
		return S_OK;
	}
	DGLE_RESULT DGLE_API GetBufferDimensions(uint& uiVerticesDataSize, uint& uiVerticesCount, uint& uiIndexesDataSize, uint& uiIndexesCount) override 
	{
		uiVerticesCount = _vertexCount;
		uiIndexesCount = _indexCount;
		uiVerticesDataSize = _vertexCount * _vertexBytes;
		uiIndexesDataSize = _indexCount * _indexBytes;
		return S_OK;
	}
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
	GLuint _textureID;
	bool _bGenerateMipmaps;

public:

	inline GLuint Texture_ID() { return _textureID; }

	GLTexture(bool bGenerateMipmaps) :
		_bGenerateMipmaps(bGenerateMipmaps)
	{
		E_GUARDS();
		glGenTextures(1, &_textureID);
		E_GUARDS();
		//LOG_INFO("GLTexture()");
	}
	~GLTexture()
	{
		E_GUARDS();
		glDeleteTextures(1, &_textureID);
		E_GUARDS();
		//LOG_INFO("~GLTexture()");
	}

	DGLE_RESULT DGLE_API GetSize(uint& width, uint& height) override {return S_OK;}
	DGLE_RESULT DGLE_API GetDepth(uint& depth) override {return S_OK;}
	DGLE_RESULT DGLE_API GetType(E_TEXTURE_TYPE& eType) override {return S_OK;}
	DGLE_RESULT DGLE_API GetFormat(E_TEXTURE_DATA_FORMAT& eFormat) override {return S_OK;}
	DGLE_RESULT DGLE_API GetLoadFlags(E_TEXTURE_LOAD_FLAGS& eLoadFlags) override {return S_OK;}
	DGLE_RESULT DGLE_API GetPixelData(uint8* pData, uint& uiDataSize, uint uiLodLevel) override {return S_OK;}
	DGLE_RESULT DGLE_API SetPixelData(const uint8* pData, uint uiDataSize, uint uiLodLevel) override {return S_OK;}
	DGLE_RESULT DGLE_API Reallocate(const uint8* pData, uint uiWidth, uint uiHeight, bool bMipMaps, E_TEXTURE_DATA_FORMAT eDataFormat) override 
	{
		E_GUARDS();
		GLint VRAMFormat;
		GLenum sourceFormat;
		GLenum sourceType = GL_UNSIGNED_BYTE;
		getGLFormats(eDataFormat, VRAMFormat, sourceFormat);

		glBindTexture(GL_TEXTURE_2D, Texture_ID());

		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, uiWidth, uiHeight, sourceFormat, sourceType, pData);
		if (_bGenerateMipmaps) glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		E_GUARDS();
		return S_OK;
	}
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

GL3XCoreRender::GL3XCoreRender(IEngineCore *pCore) : _iMaxAnisotropy(0)
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
	E_GUARDS();
	#define OGLI "Initialized at OpenGL " 
	GLint major, minor;
	char buffer[sizeof(OGLI) + 4];	
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	sprintf(buffer, OGLI"%i.%i", major, minor);
	LOG_INFO(string(buffer));
	E_GUARDS();

	for each (const ShaderSrc& sh in getShaderSources())
	{
		GLShader s;
		s.Init(sh);
		_shaders.insert(s);
	}

	E_GUARDS();
	if (GLEW_EXT_texture_filter_anisotropic)
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &_iMaxAnisotropy);
	E_GUARDS();
	if (stWin.eMultisampling != MM_NONE) glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST); E_GUARDS();
	glClearDepth(1.0);	
	/*glActiveTexture(GL_TEXTURE0 + 0); E_GUARDS();
	glAlphaFunc(GL_GREATER, 0);*/ E_GUARDS();
	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::Finalize()
{

	while (_shaders.size() != 0)
	{
		auto it = _shaders.erase(_shaders.begin());
		GLShader &s = const_cast<GLShader&>(*it);
		s.Free();
	}
	_shaders.clear();
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
	E_GUARDS();
	SwapBuffer();
	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetClearColor(const TColor4& stColor)
{ 
	E_GUARDS();
	glClearColor(stColor.r, stColor.g, stColor.b, stColor.a);
	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetClearColor(TColor4& stColor)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::Clear(bool bColor, bool bDepth, bool bStencil)
{ 
	E_GUARDS();
	GLbitfield mask = 0;
	if (bColor) mask |= GL_COLOR_BUFFER_BIT;
	if (bDepth) mask |= GL_DEPTH_BUFFER_BIT;
	if (bStencil) mask |= GL_STENCIL_BUFFER_BIT;
	glClear(mask);
	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetViewport(uint x, uint y, uint width, uint height)
{ 
	E_GUARDS();
	glViewport(x, y, width, height);
	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetViewport(uint& x, uint& y, uint& width, uint& height)
{ 
	E_GUARDS();
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	x = vp[0]; y = vp[1];
	width = vp[2]; height = vp[3];
	E_GUARDS();
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
	E_GUARDS();
	glLineWidth(fWidth);
	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetLineWidth(float& fWidth)
{ 
	E_GUARDS();
	GLfloat value;
	glGetFloatv(GL_LINE_WIDTH, &value);
	fWidth = value;
	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetPointSize(float fSize)
{ 
	E_GUARDS();
	glPointSize(fSize);
	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetPointSize(float& fSize)
{ 
	E_GUARDS();
	GLfloat value;
	glGetFloatv(GL_POINT_SIZE, &value);
	fSize = value;
	E_GUARDS();
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

DGLE_RESULT DGLE_API GL3XCoreRender::CreateTexture(ICoreTexture*& pTex, const uint8* pData, uint uiWidth, uint uiHeight, bool bMipmapsPresented, E_CORE_RENDERER_DATA_ALIGNMENT eDataAlignment, E_TEXTURE_DATA_FORMAT eDataFormat, E_TEXTURE_LOAD_FLAGS eLoadFlags)
{ 
	E_GUARDS();

	// TODO: implenment NPOT texture
	const bool powerOfTwo_h = !(uiHeight == 0) && !(uiHeight & (uiHeight - 1));
	const bool powerOfTwo_w = !(uiWidth == 0) && !(uiWidth & (uiWidth - 1));
	assert(powerOfTwo_h && powerOfTwo_w);

	// if bMipmapsPresented == true, we ignore this
	// because this plugin supports hardware mipmap generation
	// glGenerateMipmap() avaliable since OpenGL 3.0

	// 1. Filtering
	// 2. Wraping
	// 3. Compressing
	// 4. Generate mipmaps

	GLTexture* pGLTexture = new GLTexture( (eLoadFlags & TLF_GENERATE_MIPMAPS) != 0 );

	glBindTexture(GL_TEXTURE_2D, pGLTexture->Texture_ID());

	E_GUARDS();
	// filtering
	if (eLoadFlags & TLF_FILTERING_ANISOTROPIC)
	{
		assert(GLEW_EXT_texture_filter_anisotropic);E_GUARDS();

		GLint anisotropic_level = 4;E_GUARDS();
		if (eLoadFlags & TLF_ANISOTROPY_2X)	anisotropic_level = 2;
		else if (eLoadFlags & TLF_ANISOTROPY_4X) anisotropic_level = 4;
		else if (eLoadFlags & TLF_ANISOTROPY_8X) anisotropic_level = 8;
		else if (eLoadFlags & TLF_ANISOTROPY_16X) anisotropic_level = 16;

		if (anisotropic_level > _iMaxAnisotropy) anisotropic_level = _iMaxAnisotropy;E_GUARDS();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic_level);E_GUARDS();

		if (eLoadFlags & TLF_GENERATE_MIPMAPS)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);E_GUARDS();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);E_GUARDS();
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);E_GUARDS();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);E_GUARDS();			
		}
	}
	else
	{
		GLint glFilter;E_GUARDS();
		if (eLoadFlags & TLF_GENERATE_MIPMAPS)
		{
			switch (eLoadFlags)
			{
				case TLF_FILTERING_NONE:		glFilter = GL_NEAREST_MIPMAP_NEAREST;E_GUARDS(); break;E_GUARDS();
				case TLF_FILTERING_BILINEAR:	glFilter = GL_LINEAR_MIPMAP_NEAREST;E_GUARDS(); break;E_GUARDS();
				case TLF_FILTERING_TRILINEAR:
				default:						glFilter = GL_LINEAR_MIPMAP_LINEAR;E_GUARDS(); break;E_GUARDS();
			}
		}
		else
		{
			switch (eLoadFlags)
			{
				case TLF_FILTERING_NONE:		glFilter = GL_NEAREST;E_GUARDS(); break;E_GUARDS();
				case TLF_FILTERING_BILINEAR:
				case TLF_FILTERING_TRILINEAR: // mustn't happen without mipmaps
				default:						glFilter = GL_LINEAR;E_GUARDS(); break;E_GUARDS();
			}
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter); 
		GLenum err = glGetError(); E_GUARDS();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);E_GUARDS();

	}

	// wraping
	GLint glWrap;
	switch (eLoadFlags)
	{
		case TLF_COORDS_CLAMP:			glWrap = GL_CLAMP_TO_BORDER; break;
		case TLF_COORDS_MIRROR_REPEAT:	glWrap = GL_MIRRORED_REPEAT; break;
		case TLF_COORDS_MIRROR_CLAMP:	glWrap = GL_MIRROR_CLAMP_TO_EDGE; break;
		case TLF_COORDS_REPEAT:			
		default:						glWrap = GL_REPEAT; break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrap);

	
	GLint internalFormat;
	GLenum sourceFormat;
	GLenum sourceType = GL_UNSIGNED_BYTE;
	getGLFormats(eDataFormat, internalFormat, sourceFormat);
	
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, uiWidth, uiHeight, 0, sourceFormat, sourceType, nullptr); // allocate
	//glTexStorage2D(GL_TEXTURE_2D, mipmaps, VRAMFormat, uiWidth, uiHeight); // 4.2	

	auto result = pGLTexture->Reallocate(pData, uiWidth, uiHeight, bMipmapsPresented, eDataFormat); // upload

	glBindTexture(GL_TEXTURE_2D, 0);

	pTex = pGLTexture;

	E_GUARDS();
	return result;
}

DGLE_RESULT DGLE_API GL3XCoreRender::CreateGeometryBuffer(ICoreGeometryBuffer*& prBuffer, const TDrawDataDesc& stDrawDesc, uint uiVerticesCount, uint uiIndicesCount, E_CORE_RENDERER_DRAW_MODE eMode, E_CORE_RENDERER_BUFFER_TYPE eType)
{ 
	E_GUARDS();

	GLGeometryBuffer* pGLBuffer = new GLGeometryBuffer(eType, uiIndicesCount > 0, this);
	prBuffer = pGLBuffer;
	auto res = pGLBuffer->Reallocate(stDrawDesc, uiVerticesCount, uiIndicesCount, eMode);

	E_GUARDS();
	return res;
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
	E_GUARDS();

	State state;

	GLboolean enabled;
	glGetBooleanv(GL_BLEND, &enabled);
	state.blend.bEnabled = enabled != 0;
	GLint blendSrc;
	GLint blendDst;
	glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
	glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);
	state.blend.eSrcFactor = BlendFactor_GL_2_DGLE(blendSrc);
	state.blend.eDstFactor = BlendFactor_GL_2_DGLE(blendDst);

	//glGetBooleanv(GL_ALPHA_TEST, &enabled);
	//state.alphaTest = enabled != 0;

	state.tex_ID_last_binded = tex_ID_last_binded;

	_states.push(state);

	E_GUARDS();

	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::PopStates()
{ 
	E_GUARDS();

	State state = _states.top();
	_states.pop();

	if (state.blend.bEnabled)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	glBlendFunc(BlendFactor_DGLE_2_GL(state.blend.eSrcFactor), BlendFactor_DGLE_2_GL(state.blend.eDstFactor));

	//if (state.alphaTest)
	//	glEnable(GL_ALPHA_TEST);
	//else
	//	glDisable(GL_ALPHA_TEST);

	tex_ID_last_binded = state.tex_ID_last_binded;
	
	E_GUARDS();
	
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetMatrix(const TMatrix4x4& stMatrix, E_MATRIX_TYPE eMatType)
{ 
	switch (eMatType)
	{
		case MT_MODELVIEW: MV = stMatrix; break;
		case MT_PROJECTION: P = stMatrix; break;
		case MT_TEXTURE: /* TODO: */ break;
	}
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetMatrix(TMatrix4x4& stMatrix, E_MATRIX_TYPE eMatType)
{ 
	switch (eMatType)
	{
		case MT_MODELVIEW: stMatrix = MV; break;
		case MT_PROJECTION: stMatrix = P; break;
		case MT_TEXTURE: /* TODO: */ break;
	}
	return S_OK;
}

GLShader* GL3XCoreRender::chooseShader(CORE_GEOMETRY_ATTRIBUTES_PRESENTED model_attributes, bool texture_binded, bool light_on, bool is2D)
{
	//if (is2D)
	//{
	//	assert(model_attributes & CGAP_POS);
	//	assert(model_attributes & CGAP_TEX);
	//	return _p_PT2D_shader.get();
	//}

	//if (texture_binded && light_on)
	//{		
	//	if (model_attributes == CGAP_POS_NORM) return _p_PN_shader.get();
	//	if (model_attributes == CGAP_POS_NORM_TEX) return _p_PNT_shader.get();
	//	if (model_attributes == CGAP_POS_TEX) return _p_PT_shader.get();
	//	assert(false); // unreachable
	//	return _p_P_shader.get();
	//}
	//else if (!texture_binded && light_on)
	//{
	//	if (model_attributes == CGAP_POS_NORM) return _p_PN_shader.get();
	//	if (model_attributes == CGAP_POS_NORM_TEX) return _p_PN_shader.get();
	//	if (model_attributes == CGAP_POS_TEX) return _p_PT_shader.get();
	//	assert(false); // unreachable
	//	return _p_P_shader.get();
	//}
	//else if (texture_binded && !light_on)
	//{
	//	if (model_attributes == CGAP_POS_NORM) return _p_P_shader.get();
	//	if (model_attributes == CGAP_POS_NORM_TEX) return _p_PT_shader.get();
	//	if (model_attributes == CGAP_POS_TEX) return _p_PT_shader.get();
	//	assert(false); // unreachable
	//	return _p_P_shader.get();
	//}
	//else
	//	return _p_P_shader.get();

	bool norm = (model_attributes & CGAP_NORM) != CGAP_NONE;
	bool tex = (model_attributes & CGAP_TEX) != CGAP_NONE;
	
	auto it = std::find_if(_shaders.begin(), _shaders.end(),
		[norm, tex, texture_binded, light_on, is2D](const GLShader& shd) -> bool
	{
		return
			shd.bPositionIs2D() == is2D &&
			//s.bUniformnL() == light_on &&
			//s.bUniformNM() == light_on &&
			shd.bUniformTexture0() == texture_binded &&
			shd.bInputNormals() == (light_on && norm);
	});
	return &(const_cast<GLShader&>(*it));
}

DGLE_RESULT DGLE_API GL3XCoreRender::Draw(const TDrawDataDesc& stDrawDesc, E_CORE_RENDERER_DRAW_MODE eMode, uint uiCount)
{ 
	E_GUARDS();

	GLGeometryBuffer buffer(CRBT_HARDWARE_STATIC, false, this);
	buffer.Reallocate(stDrawDesc, uiCount, 0, eMode);
	DrawBuffer(&buffer);

	E_GUARDS();
	
		return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::DrawBuffer(ICoreGeometryBuffer* pBuffer)
{ 
	E_GUARDS();

	GLGeometryBuffer *b = dynamic_cast<GLGeometryBuffer*>(pBuffer);
	if (b == nullptr) return S_OK;	

	const bool texture_binded = tex_ID_last_binded != 0;
	const bool light_on = true;
	
	const GLShader* pChoosenShader = chooseShader(b->GetAttributes(), texture_binded, light_on, b->Is2dPosition());

	glUseProgram(pChoosenShader->ID_Program());

	glBindVertexArray(b->VAO_ID());

	b->ToggleAttribInVAO(GLGeometryBuffer::NORMALS_ATTRIBUTE, pChoosenShader->bInputNormals());
	b->ToggleAttribInVAO(GLGeometryBuffer::TEXCOORDS_ATTRIBUTE, pChoosenShader->bInputTextureCoords());

	if (!pChoosenShader->bPositionIs2D())
	{
		const TMatrix4x4 MVP = MV * P;
		const GLuint MVP_ID = glGetUniformLocation(pChoosenShader->ID_Program(), "MVP");
		glUniformMatrix4fv(MVP_ID, 1, GL_FALSE, &MVP._1D[0]);
	}
	if (pChoosenShader->bUniformNM())
	{
		const TMatrix4x4 NM = MatrixTranspose(MatrixInverse(MV)); // Normal matrix = (MV^-1)^T
		const GLuint NM_ID = glGetUniformLocation(pChoosenShader->ID_Program(), "NM");
		glUniformMatrix4fv(NM_ID, 1, GL_FALSE, &NM._1D[0]);
	}
	if (pChoosenShader->bUniformnL())
	{
		const TVector3 L = { 0.2f, 1.0f, 1.0f };
		const TVector3 nL = L / L.Length();
		const TVector3 nL_eyeSpace = MV.ApplyToVector(nL);
		const GLuint nL_ID = glGetUniformLocation(pChoosenShader->ID_Program(), "nL");
		glUniform3f(nL_ID, nL.x, nL.y, nL.z);
	}
	if (pChoosenShader->bUniformTexture0())
	{
		glBindTexture(GL_TEXTURE_2D, tex_ID_last_binded);
		const GLuint tex_ID = glGetUniformLocation(pChoosenShader->ID_Program(), "texture0");
		glUniform1i(tex_ID, 0);
	}
	if (pChoosenShader->bPositionIs2D())
	{
		const GLuint width_ID = glGetUniformLocation(pChoosenShader->ID_Program(), "screenWidth");
		const GLuint height_ID = glGetUniformLocation(pChoosenShader->ID_Program(), "screenHeight");
		uint x, y, w, h;
		GetViewport(x, y, w, h);
		glUniform1ui(width_ID, w);
		glUniform1ui(height_ID, h);
	}

	if (b->IndexDrawing())
		glDrawElements(b->GLDrawMode(), b->IndexCount(), ((b->IndexCount() > 65535) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT), nullptr);
	else if (b->VertexCount() > 0)
		glDrawArrays(b->GLDrawMode(), 0, b->VertexCount());

	glBindVertexArray(0);

	E_GUARDS();
	
		return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetColor(const TColor4& stColor)
{
	//glColor4f(stColor.r, stColor.g, stColor.b, stColor.a); //1281
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetColor(TColor4& stColor)
{
	E_GUARDS();

	GLfloat color[4];
	glGetFloatv(GL_CURRENT_COLOR, color);
	stColor = color;
	
	E_GUARDS(); 

	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::ToggleBlendState(bool bEnabled)
{
	E_GUARDS();

	if (bEnabled)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	
	E_GUARDS();

	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::ToggleAlphaTestState(bool bEnabled)
{ 
	E_GUARDS();
	//if (bEnabled)
	//	glEnable(GL_ALPHA_TEST);
	//else
	//	glDisable(GL_ALPHA_TEST);
	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetBlendState(const TBlendStateDesc& stState)
{ 
	E_GUARDS();
	glBlendFunc(BlendFactor_DGLE_2_GL(stState.eSrcFactor), BlendFactor_DGLE_2_GL(stState.eDstFactor));
	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetBlendState(TBlendStateDesc& stState)
{ 
	E_GUARDS();

	GLint blendSrc;
	GLint blendDst;
	glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
	glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);
	stState.eSrcFactor = BlendFactor_GL_2_DGLE(blendSrc);
	stState.eDstFactor = BlendFactor_GL_2_DGLE(blendDst);
	
	E_GUARDS();
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
	assert(
		uiTextureLayer == 0 ||						// bind 0 
		(uiTextureLayer != 0 && pTex == nullptr) ); // unbind every
	
	GLTexture *pGLTex = static_cast<GLTexture*>(pTex);
	
	if (pGLTex == nullptr)
		tex_ID_last_binded = 0;
	else
		tex_ID_last_binded = pGLTex->Texture_ID();

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
	E_GUARDS();

	iValue = 0;
	switch (eMetric)
	{
		case CRMT_MAX_TEXTURE_RESOLUTION: glGetIntegerv(GL_MAX_TEXTURE_SIZE, &iValue); break;
		case CRMT_MAX_TEXTURE_LAYERS: glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &iValue); break;
		case CRMT_MAX_ANISOTROPY_LEVEL: if (GLEW_EXT_texture_filter_anisotropic) glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &iValue); break;
		default: break;
	}

	E_GUARDS();

	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::IsFeatureSupported(E_CORE_RENDERER_FEATURE_TYPE eFeature, bool& bIsSupported)
{ 
	E_GUARDS();

	bIsSupported = false;
	switch (eFeature)
	{
	case CRFT_BUILTIN_FULLSCREEN_MODE: break;
	case CRFT_BUILTIN_STATE_FILTER: break;
	case CRFT_MULTISAMPLING: break;
	case CRFT_VSYNC: break;
	case CRFT_PROGRAMMABLE_PIPELINE: bIsSupported = true; break;
	case CRFT_LEGACY_FIXED_FUNCTION_PIPELINE_API: bIsSupported = false; break;
	case CRFT_BGRA_DATA_FORMAT: break;
	case CRFT_TEXTURE_COMPRESSION: break;
	case CRFT_NON_POWER_OF_TWO_TEXTURES: break;
	case CRFT_DEPTH_TEXTURES: break;
	case CRFT_TEXTURE_ANISOTROPY: bIsSupported = true; break;
	case CRFT_TEXTURE_MIPMAP_GENERATION: bIsSupported = true; break;
	case CRFT_TEXTURE_MIRRORED_REPEAT: bIsSupported = true; break;
	case CRFT_TEXTURE_MIRROR_CLAMP: bIsSupported = true; break;
	case CRFT_GEOMETRY_BUFFER: break;
	case CRFT_FRAME_BUFFER: break;
	default: break;
	}

	E_GUARDS();
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
