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
#include <memory>
#include <map>
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

int calculateDataSize(uint uiWidth, uint uiHeight, E_TEXTURE_DATA_FORMAT eDataFormat)
{
	//TODO: support align
	if (eDataFormat == TDF_DXT1 || eDataFormat == TDF_DXT5)
	{
		int blockSize;
		if (eDataFormat == TDF_DXT1)
			blockSize = 8;
		else
			blockSize = 16;
		if (uiWidth < 4 || uiHeight < 4) return blockSize;
		return (uiWidth / 4) * (uiHeight / 4) * blockSize;
	}
	int bytePerPixel;
	switch(eDataFormat)
	{
		case TDF_ALPHA8: bytePerPixel = 1; break;
		case TDF_BGR8: bytePerPixel = 3;
		case TDF_RGB8: bytePerPixel = 3; break;
		case TDF_RGBA8: bytePerPixel = 4;
		case TDF_BGRA8: bytePerPixel = 4; break;		
		default: assert(false);
	}
	int n = uiWidth * uiHeight * bytePerPixel;
	return n;
}

static void LogToDGLE(const char *pcTxt, E_LOG_TYPE eType, const char *pcSrcFileName, int iSrcLineNumber)
{
	_core->WriteToLogEx(pcTxt, eType, pcSrcFileName, iSrcLineNumber);
}

static void checkShaderError(uint id, GLenum type)
{
	int iStatus;

	if (type == GL_COMPILE_STATUS)
		glGetShaderiv(id, GL_COMPILE_STATUS, &iStatus);
	else if (type == GL_LINK_STATUS)
		glGetProgramiv(id, GL_LINK_STATUS, &iStatus);

	if (iStatus == GL_FALSE)
	{
		GLint length = 0;
		if (type == GL_COMPILE_STATUS)
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		else if (type == GL_LINK_STATUS)
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
		auto msg = make_unique<char[]>(length);
		if (type == GL_COMPILE_STATUS)
			glGetShaderInfoLog(id, length, &length, msg.get());
		else if (type == GL_LINK_STATUS)
			glGetProgramInfoLog(id, length, &length, msg.get());

		assert(false);
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

bool GLShader::bPositionIsVec2() const { return p->bPositionIsVec2; }

bool GLShader::hasUniform(string u) const
{
	E_GUARDS();
	int id = glGetUniformLocation(programID, u.c_str());
	E_GUARDS();
	return id != -1;
}

bool GLShader::bAlphaTest() const
{
	return p->bAlphaTest;
}

bool GLShader::bInputNormals() const { return (p->attribs & NORM) > 0; }
bool GLShader::bInputTextureCoords() const { return (p->attribs & TEX_COORD) > 0; }

static void getGLFormats(E_TEXTURE_DATA_FORMAT eDataFormat, GLint& VRAMFormat, GLenum& sourceFormat)
{
	switch (eDataFormat)
	{
		case TDF_RGB8:				VRAMFormat = GL_RGB8;	sourceFormat = GL_RGB;  break;
		case TDF_RGBA8:				VRAMFormat = GL_RGBA8;	sourceFormat = GL_RGBA; break;
		case TDF_ALPHA8:			VRAMFormat = GL_R8;		sourceFormat = GL_RED; break;
		case TDF_BGR8:				VRAMFormat = GL_RGB8;	sourceFormat = GL_BGR; break;
		case TDF_BGRA8:				VRAMFormat = GL_RGBA8;	sourceFormat = GL_BGRA; break;
		case TDF_DXT1:				VRAMFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;	sourceFormat = GL_RGB; break;
		case TDF_DXT5: assert(false); /*implement*/ break;
		//case TDF_DEPTH_COMPONENT24: break;
		//case TDF_DEPTH_COMPONENT32: break;
		default: assert(false); break;
	}
}


////////////////////////////
//        Geometry        //
////////////////////////////
	
inline GLuint GLGeometryBuffer::input_attrib_to_uint(INPUT_ATTRIBUTE attrib)
{
	const static map<INPUT_ATTRIBUTE, GLuint> enum_to_ind =
	{
		{ POS, 0 },
		{ NORM, 1 },
		{ TEX_COORD, 2 }
	};
	return enum_to_ind.at(attrib);
}
inline GLenum GLGeometryBuffer::GLDrawMode()
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
inline void GLGeometryBuffer::ToggleAttribInVAO(INPUT_ATTRIBUTE attrib, bool value)
{
	E_GUARDS();

	const GLuint i = input_attrib_to_uint(attrib);

	if (value && !activated_attributes[i])
	{
		activated_attributes[i] = true;
		glEnableVertexAttribArray(i);
	}
	else if (!value && activated_attributes[i])
	{
		activated_attributes[i] = false;
		glDisableVertexAttribArray(i);
	}
	E_GUARDS();
}

GLsizei GLGeometryBuffer::vertexSize(const TDrawDataDesc& stDrawDesc)
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

GLGeometryBuffer::GLGeometryBuffer(E_CORE_RENDERER_BUFFER_TYPE eType, bool indexBuffer, GL3XCoreRender *pRnd) :
	_bAlreadyInitalized(false), _vertexCount(0), _indexCount(0), _vao(0), _vbo(0), _ibo(0), _eBufferType(eType), _pRnd(pRnd), _attribs_presented(NONE), activated_attributes{0}, _b2dPosition(false)
{		
	E_GUARDS();
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);	
	if (indexBuffer) glGenBuffers(1, &_ibo);
	//LOG_INFO("GLGeometryBuffer()");
	E_GUARDS();
}

GLGeometryBuffer::~GLGeometryBuffer()
{		
	E_GUARDS();
	if (_ibo!=0) glDeleteBuffers(1, &_ibo);
	glDeleteBuffers(1, &_vbo);
	glDeleteVertexArrays(1, &_vao);
	//LOG_INFO("~GLGeometryBuffer()");
	E_GUARDS();
}

DGLE_RESULT DGLE_API GLGeometryBuffer::GetGeometryData(TDrawDataDesc& stDesc, uint uiVerticesDataSize, uint uiIndexesDataSize) {return S_OK;}
DGLE_RESULT DGLE_API GLGeometryBuffer::SetGeometryData(const TDrawDataDesc& stDrawDesc, uint uiVerticesDataSize, uint uiIndexesDataSize)	{ return S_OK; } // what is purpose if Reallocate() exists?
DGLE_RESULT DGLE_API GLGeometryBuffer::Reallocate(const TDrawDataDesc& stDrawDesc, uint uiVerticesCount, uint uiIndicesCount, E_CORE_RENDERER_DRAW_MODE eMode)
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

		glVertexAttribPointer(input_attrib_to_uint(POS), _b2dPosition ? 2 : 3, GL_FLOAT, GL_FALSE, stDrawDesc.uiVertexStride, reinterpret_cast<void*>(0));
		_attribs_presented = POS;

		if (stDrawDesc.uiNormalOffset != -1)
		{
			glVertexAttribPointer(input_attrib_to_uint(NORM), 3, GL_FLOAT, GL_FALSE, stDrawDesc.uiNormalStride, reinterpret_cast<void*>(stDrawDesc.uiNormalOffset));
			_attribs_presented = _attribs_presented | NORM;
		}
		if (stDrawDesc.uiTextureVertexOffset != -1)
		{
			glVertexAttribPointer(input_attrib_to_uint(TEX_COORD), 2, GL_FLOAT, GL_FALSE, stDrawDesc.uiTextureVertexStride, reinterpret_cast<void*>(stDrawDesc.uiTextureVertexOffset));
			_attribs_presented = _attribs_presented | TEX_COORD;
		}
		assert(_attribs_presented & POS);
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
DGLE_RESULT DGLE_API GLGeometryBuffer::GetBufferDimensions(uint& uiVerticesDataSize, uint& uiVerticesCount, uint& uiIndexesDataSize, uint& uiIndexesCount)
{
	uiVerticesCount = _vertexCount;
	uiIndexesCount = _indexCount;
	uiVerticesDataSize = _vertexCount * _vertexBytes;
	uiIndexesDataSize = _indexCount * _indexBytes;
	return S_OK;
}
DGLE_RESULT DGLE_API GLGeometryBuffer::GetBufferDrawDataDesc(TDrawDataDesc& stDesc) {return S_OK;}
DGLE_RESULT DGLE_API GLGeometryBuffer::GetBufferDrawMode(E_CORE_RENDERER_DRAW_MODE& eMode) {return S_OK;}
DGLE_RESULT DGLE_API GLGeometryBuffer::GetBufferType(E_CORE_RENDERER_BUFFER_TYPE& eType) {return S_OK;}
DGLE_RESULT DGLE_API GLGeometryBuffer::GetBaseObject(IBaseRenderObjectContainer*& prObj)	{return S_OK;}
DGLE_RESULT DGLE_API GLGeometryBuffer::Free() 
{
	delete this;
	return S_OK;
}


////////////////////////////
//         Texture        //
////////////////////////////


GLTexture::GLTexture() :
	_bMipmapsAllocated(false)
{
	E_GUARDS();
	glGenTextures(1, &_textureID);
	E_GUARDS();
}
GLTexture::~GLTexture()
{
	E_GUARDS();
	glDeleteTextures(1, &_textureID);
	E_GUARDS();
}

DGLE_RESULT DGLE_API GLTexture::GetSize(uint& width, uint& height)
{
	E_GUARDS();

	int w, h;
	glBindTexture(GL_TEXTURE_2D, Texture_ID());
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	glBindTexture(GL_TEXTURE_2D, 0);
	width = w;
	height = h;

	E_GUARDS();

	return S_OK;
}
DGLE_RESULT DGLE_API GLTexture::GetDepth(uint& depth) {return S_OK;}
DGLE_RESULT DGLE_API GLTexture::GetType(E_TEXTURE_TYPE& eType) {return S_OK;}
DGLE_RESULT DGLE_API GLTexture::GetFormat(E_TEXTURE_DATA_FORMAT& eFormat) {return S_OK;}
DGLE_RESULT DGLE_API GLTexture::GetLoadFlags(E_TEXTURE_LOAD_FLAGS& eLoadFlags) {return S_OK;}
DGLE_RESULT DGLE_API GLTexture::GetPixelData(uint8* pData, uint& uiDataSize, uint uiLodLevel) {return S_OK;}
DGLE_RESULT DGLE_API GLTexture::SetPixelData(const uint8* pData, uint uiDataSize, uint uiLodLevel) {return S_OK;}
DGLE_RESULT DGLE_API GLTexture::Reallocate(const uint8* pData, uint uiWidth, uint uiHeight, bool bMipMaps, E_TEXTURE_DATA_FORMAT eDataFormat) 
{ 
	// TODO: 
	// load mipmaps
	// check if foormat changed then recreate texture
	// check if mipmap: true -> false => recreate texture; false->true => glGenerateMipmap()
	E_GUARDS();
	GLint VRAMFormat;
	GLenum sourceFormat;
	GLenum sourceType = GL_UNSIGNED_BYTE;
	getGLFormats(eDataFormat, VRAMFormat, sourceFormat);

	glBindTexture(GL_TEXTURE_2D, Texture_ID());
	E_GUARDS();

	const bool compressed = eDataFormat == TDF_DXT1 || eDataFormat == TDF_DXT5;
		
	if (compressed)
	{
		int nSize = calculateDataSize(uiWidth, uiHeight, eDataFormat);
		glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, uiWidth, uiHeight, VRAMFormat, nSize, pData);
	}
	else
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, uiWidth, uiHeight, sourceFormat, sourceType, pData);
	E_GUARDS();
	if (bMipMaps) glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	E_GUARDS();
	return S_OK;
}
DGLE_RESULT DGLE_API GLTexture::GetBaseObject(IBaseRenderObjectContainer*& prObj) {return S_OK;}

DGLE_RESULT DGLE_API GLTexture::Free()
{
	delete this;
	return S_OK;
}


void FBO::Init()
{
	E_GUARDS();
	glGenFramebuffers(1, &ID);
	E_GUARDS();
}

void FBO::GenerateDepthRenderbuffer(uint w, uint h)
{
	E_GUARDS();
	glGenRenderbuffers(1, &depth_renderbuffer_ID);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_ID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	E_GUARDS();
}

void FBO::Free()
{
	E_GUARDS();
	if (depth_renderbuffer_ID)
		glDeleteRenderbuffers(1, &depth_renderbuffer_ID);
	glDeleteFramebuffers(1, &ID);
	E_GUARDS();
}

//////////////////////////
//         Render       //
//////////////////////////

GL3XCoreRender::GL3XCoreRender(IEngineCore *pCore) : 
	tex_ID_last_binded(0), alphaTest(false), pCurrentRenderTarget(nullptr),
	_clearColor(0, 0, 0, 0)
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
	GLfloat clColor[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, clColor);
	_clearColor.SetColorF(clColor[0], clColor[1], clColor[2], clColor[3]);
	E_GUARDS();

	for each (const ShaderSrc& sh in getShaderSources())
	{
		GLShader s;
		s.Init(sh);
		_shaders.push_back(s);
	}

	E_GUARDS();
	if (stWin.eMultisampling != MM_NONE) glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST); E_GUARDS();
	glClearDepth(1.0);	
	
	GLfloat r1[2];
	glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, r1);

	GLfloat r2[2];
	glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, r2);
	
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	viewportX = vp[0];
	viewportY = vp[1];
	viewportHeight = vp[2];
	viewportWidth = vp[3];

	//glEnable(GL_FRAMEBUFFER_SRGB);

	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::Finalize()
{
	for each (GLShader shd in _shaders)
		shd.Free();
	_shaders.clear();

	for each (FBO fbo in _fboPool)
		fbo.Free();

	_fboPool.clear();

	FreeGL();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::AdjustMode(TEngineWindow& stNewWin)
{ 
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::MakeCurrent()
{ 
	E_GUARDS();
	::MakeCurrent();
	E_GUARDS();
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
	_clearColor = stColor;
	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetClearColor(TColor4& stColor)
{ 
	GLfloat clColor[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, clColor);
	_clearColor.SetColorF(clColor[0], clColor[1], clColor[2], clColor[3]);
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
	// INVALID_OPERATION for fWidth > 1 in 3.2
	// No sense call glSetLineWidth() with values other than [0, 1]
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetLineWidth(float& fWidth)
{ 
	fWidth = 1.0f;
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
	E_GUARDS();

	if (pTexture == pCurrentRenderTarget)
		return S_OK;

	if (pTexture != nullptr)
	{
		uint h, w;
		pTexture->GetSize(w, h);

		int fbo_idx = -1;

		for(size_t i = 0; i < _fboPool.size(); i++)
		{
			if (h == _fboPool[i].height && w == _fboPool[i].width)
			{
				fbo_idx = i; 
				break;
			}
		}

		if (fbo_idx == -1)
		{
			FBO fbo;
			fbo.Init();
			fbo.GenerateDepthRenderbuffer(w, h);
			fbo.width = w;
			fbo.height = h;
			fbo_idx = _fboPool.size();
			_fboPool.push_back(fbo);
		}

		FBO& fbo = _fboPool[fbo_idx];

		glBindFramebuffer(GL_FRAMEBUFFER, fbo.ID);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo.depth_renderbuffer_ID);

		GLTexture *pGLTexture = static_cast<GLTexture*>(pTexture);
		GLuint tex_id = pGLTexture->Texture_ID();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		assert(status == GL_FRAMEBUFFER_COMPLETE);

		glViewport(0, 0, w, h);

		pCurrentRenderTarget = pTexture;
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(viewportX, viewportY, viewportHeight, viewportWidth);
		pCurrentRenderTarget = nullptr;
	}

	E_GUARDS();

	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetRenderTarget(ICoreTexture*& prTexture)
{
	prTexture = pCurrentRenderTarget;
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::CreateTexture(ICoreTexture*& pTex, const uint8* pData, uint uiWidth, uint uiHeight, bool bMipmapsPresented, E_CORE_RENDERER_DATA_ALIGNMENT eDataAlignment, E_TEXTURE_DATA_FORMAT eDataFormat, E_TEXTURE_LOAD_FLAGS eLoadFlags)
{ 
	E_GUARDS();

	// TODO: implenment NPOT texture
	const bool powerOfTwo_h = !(uiHeight == 0) && !(uiHeight & (uiHeight - 1));
	const bool powerOfTwo_w = !(uiWidth == 0) && !(uiWidth & (uiWidth - 1));
	//assert(powerOfTwo_h && powerOfTwo_w);

	bool bGenerateMipMaps = (eLoadFlags & TLF_GENERATE_MIPMAPS) != 0;
	if (bMipmapsPresented && bGenerateMipMaps) bGenerateMipMaps = false;

	const bool willBeMipMaps = bMipmapsPresented || bGenerateMipMaps;

	GLTexture* pGLTexture = new GLTexture();

	glBindTexture(GL_TEXTURE_2D, pGLTexture->Texture_ID());

	if (eLoadFlags & TLF_FILTERING_ANISOTROPIC)
	{
		assert(GLEW_EXT_texture_filter_anisotropic);

		GLint anisotropic_level = 4;
		if (eLoadFlags & TLF_ANISOTROPY_2X)	anisotropic_level = 2;
		else if (eLoadFlags & TLF_ANISOTROPY_4X) anisotropic_level = 4;
		else if (eLoadFlags & TLF_ANISOTROPY_8X) anisotropic_level = 8;
		else if (eLoadFlags & TLF_ANISOTROPY_16X) anisotropic_level = 16;

		GLint maxAnisotropy;
		if (GLEW_EXT_texture_filter_anisotropic)
			glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);

		if (anisotropic_level > maxAnisotropy) anisotropic_level = maxAnisotropy;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic_level);

		if (willBeMipMaps)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);			
		}
	}
	else
	{
		E_GUARDS();

		GLint glMinFilter;
		if (willBeMipMaps)
		{
			if (eLoadFlags & TLF_FILTERING_NONE) 
				glMinFilter = GL_NEAREST_MIPMAP_NEAREST; 
			else if (eLoadFlags & TLF_FILTERING_BILINEAR) 
				glMinFilter = GL_LINEAR_MIPMAP_NEAREST; 
			else if (eLoadFlags & TLF_FILTERING_TRILINEAR) 
				glMinFilter = GL_LINEAR_MIPMAP_LINEAR;
			else glMinFilter = GL_NEAREST_MIPMAP_NEAREST;
		}
		else
		{
			if (eLoadFlags & TLF_FILTERING_NONE) 
				glMinFilter = GL_NEAREST;
			else if (eLoadFlags & TLF_FILTERING_BILINEAR) 
				glMinFilter = GL_LINEAR;
			else glMinFilter = GL_NEAREST;
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glMinFilter);
		E_GUARDS();


		GLint glMagFilter;
		if (eLoadFlags & TLF_FILTERING_NONE) glMagFilter = GL_NEAREST;
		else glMagFilter = GL_LINEAR;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glMagFilter);
		E_GUARDS();
	}

	GLint glWrap;
	if (eLoadFlags & TLF_COORDS_CLAMP) 
		glWrap = GL_CLAMP_TO_BORDER;
	else if (eLoadFlags & TLF_COORDS_MIRROR_REPEAT)	
		glWrap = GL_MIRRORED_REPEAT;
	else if (eLoadFlags & TLF_COORDS_MIRROR_CLAMP) 
		glWrap = GL_MIRROR_CLAMP_TO_EDGE;
	else glWrap = GL_REPEAT;
	
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrap);

	if (eDataFormat == TDF_ALPHA8)
	{
		GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
	}

	GLint internalFormat;
	GLenum sourceFormat;
	GLenum sourceType = GL_UNSIGNED_BYTE;
	getGLFormats(eDataFormat, internalFormat, sourceFormat);
	
	const bool compressed = eDataFormat == TDF_DXT1 || eDataFormat == TDF_DXT5;

	int mipmaps = 1;
	if (bMipmapsPresented)
		mipmaps = static_cast<int>(log2(uiWidth)) + 1;
	
	int nOffset = 0;
	
	for (int i = 0; i < mipmaps; i++)
	{
		int nSize = calculateDataSize(uiWidth, uiHeight, eDataFormat);

		if (!compressed)
			glTexImage2D(GL_TEXTURE_2D, i, internalFormat, uiWidth, uiHeight, 0, sourceFormat, sourceType, pData + nOffset);
		else
			glCompressedTexImage2D(GL_TEXTURE_2D, i, internalFormat, uiWidth, uiHeight, 0, nSize, pData + nOffset);

		uiWidth /= 2;
		uiHeight /= 2;
		nOffset += nSize;
		E_GUARDS();
	}

	if (mipmaps > 1) pGLTexture->SetMipmapAllocated();

	if (bGenerateMipMaps && !bMipmapsPresented)
	{
		pGLTexture->SetMipmapAllocated();
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	pTex = pGLTexture;
	
	E_GUARDS();
	return S_OK;
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

	state.tex_ID_last_binded = tex_ID_last_binded;
	
	state.alphaTest = alphaTest;

	glGetBooleanv(GL_DEPTH_TEST, &enabled);
	state.depth.bDepthTestEnabled = enabled > 0;
	//TODO: depth stencil

	state.color = _color;
	state.clearColor = _clearColor;
 
	GLint i[2];
	glGetIntegerv(GL_POLYGON_MODE, i);
	state.poligonMode = i[0];

	state.cullingOn = glIsEnabled(GL_CULL_FACE);
	if (state.cullingOn == GL_TRUE)
		glGetIntegerv(GL_CULL_FACE_MODE, &state.cullingMode);

	state.pRenderTarget = pCurrentRenderTarget;

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
	
	alphaTest = state.alphaTest;
	
	tex_ID_last_binded = state.tex_ID_last_binded;
	
	if (state.depth.bDepthTestEnabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	//TODO: depth stencil

	_color = state.color;
	SetColor(_color);

	_clearColor = state.clearColor;
	SetClearColor(_clearColor);

	glPolygonMode(GL_FRONT_AND_BACK, state.poligonMode);
	
	if (state.cullingOn == GL_FALSE)
		glDisable(GL_CULL_FACE);
	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(state.cullingMode);
	}

	if (state.pRenderTarget != pCurrentRenderTarget)
		SetRenderTarget(state.pRenderTarget);

	E_GUARDS();
	
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetMatrix(const TMatrix4x4& stMatrix, E_MATRIX_TYPE eMatType)
{ 
	switch (eMatType)
	{
		case MT_MODELVIEW: MV = stMatrix; break;
		case MT_PROJECTION: P = stMatrix; break;
		case MT_TEXTURE: T = stMatrix; break;
	}
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetMatrix(TMatrix4x4& stMatrix, E_MATRIX_TYPE eMatType)
{ 
	switch (eMatType)
	{
		case MT_MODELVIEW: stMatrix = MV; break;
		case MT_PROJECTION: stMatrix = P; break;
		case MT_TEXTURE: stMatrix = T; break;
	}
	return S_OK;
}

GLShader* GL3XCoreRender::chooseShader(INPUT_ATTRIBUTE attrib, bool texture_binded, bool light_on, bool is2D, bool alphaTest)
{
	bool norm = (attrib & NORM) > 0;
	bool tex = (attrib & TEX_COORD) > 0;
	
	auto it = std::find_if(_shaders.begin(), _shaders.end(),
		[norm, tex, texture_binded, light_on, is2D, alphaTest](const GLShader& shd) -> bool
	{
		return
			shd.bPositionIsVec2() == is2D &&
			shd.hasUniform("texture0") == (texture_binded && tex) &&
			shd.bAlphaTest() == alphaTest &&
			shd.bInputNormals() == (light_on && norm);
	});

	return &(*it);
}

DGLE_RESULT DGLE_API GL3XCoreRender::Draw(const TDrawDataDesc& stDrawDesc, E_CORE_RENDERER_DRAW_MODE eMode, uint uiCount)
{ 
	E_GUARDS();

	PushStates();

	TDepthStencilDesc depthState;
	GetDepthStencilState(depthState);
	depthState.bDepthTestEnabled = false;
	SetDepthStencilState(depthState);

	GLGeometryBuffer buffer(CRBT_HARDWARE_STATIC, false, this);
	buffer.Reallocate(stDrawDesc, uiCount, 0, eMode);

	DrawBuffer(&buffer);

	PopStates();

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
	
	const GLShader* pShd = chooseShader(b->GetAttributes(), texture_binded, light_on, b->Is2dPosition(), alphaTest);

	glUseProgram(pShd->ID_Program());

	glBindVertexArray(b->VAO_ID());

	b->ToggleAttribInVAO(POS, true);
	b->ToggleAttribInVAO(NORM, pShd->bInputNormals());
	b->ToggleAttribInVAO(TEX_COORD, pShd->bInputTextureCoords());

	if (pShd->hasUniform("MV"))
	{
		const GLuint MV_ID = glGetUniformLocation(pShd->ID_Program(), "MV");
		glUniformMatrix4fv(MV_ID, 1, GL_FALSE, &MV._1D[0]);
	}
	if (pShd->hasUniform("MVP"))
	{
		const TMatrix4x4 MVP = MV * P;
		const GLuint MVP_ID = glGetUniformLocation(pShd->ID_Program(), "MVP");
		glUniformMatrix4fv(MVP_ID, 1, GL_FALSE, &MVP._1D[0]);
	}
	if (pShd->hasUniform("NM"))
	{
		const TMatrix4x4 NM = MatrixTranspose(MatrixInverse(MV)); // Normal matrix = (MV^-1)^T
		const GLuint NM_ID = glGetUniformLocation(pShd->ID_Program(), "NM");
		glUniformMatrix4fv(NM_ID, 1, GL_FALSE, &NM._1D[0]);
	}
	if (pShd->hasUniform("nL"))
	{
		const TVector3 L = { 0.2f, 1.0f, 1.0f };
		const TVector3 nL = L / L.Length();
		const TVector3 nL_eyeSpace = MV.ApplyToVector(nL);
		const GLuint nL_ID = glGetUniformLocation(pShd->ID_Program(), "nL");
		glUniform3f(nL_ID, nL.x, nL.y, nL.z);
	}
	if (pShd->hasUniform("texture0"))
	{
		glBindTexture(GL_TEXTURE_2D, tex_ID_last_binded);
		const GLuint tex_ID = glGetUniformLocation(pShd->ID_Program(), "texture0");
		glUniform1i(tex_ID, 0);
	}
	if (pShd->hasUniform("main_color"))
	{
		const GLuint main_color_ID = glGetUniformLocation(pShd->ID_Program(), "main_color");
		glUniform4f(main_color_ID, _color.r, _color.g, _color.b, _color.a);
	}
	/*
	if (pShd->hasUniform("screenWidth"))
	{
		const GLuint width_ID = glGetUniformLocation(pShd->ID_Program(), "screenWidth");
		glUniform1ui(width_ID, viewportWidth);
	}
	if (pShd->hasUniform("screenHeight"))
	{
		const GLuint height_ID = glGetUniformLocation(pShd->ID_Program(), "screenHeight");
		glUniform1ui(height_ID, viewportHeight);
	}
	*/

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
	_color = stColor;
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
	alphaTest = bEnabled;
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetBlendState(const TBlendStateDesc& stState)
{ 
	E_GUARDS();

	if (stState.bEnabled)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
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
	GLboolean enabled;
	glGetBooleanv(GL_BLEND, &enabled);
	stState.bEnabled = enabled > 0;

	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetDepthStencilState(const TDepthStencilDesc& stState)
{ 
	E_GUARDS();

	if (stState.bDepthTestEnabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	//TODO: depth stencil
	
	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetDepthStencilState(TDepthStencilDesc& stState)
{ 
	E_GUARDS();

	GLboolean enabled;
	glGetBooleanv(GL_DEPTH_TEST, &enabled);
	stState.bDepthTestEnabled = enabled == GL_TRUE;
	//TODO: depth stencil

	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::SetRasterizerState(const TRasterizerStateDesc& stState)
{ 
	E_GUARDS();

	alphaTest = stState.bAlphaTestEnabled;
	if (stState.bWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else 
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (stState.eCullMode == PCM_NONE)
		glDisable(GL_CULL_FACE);
	else
	{
		glEnable(GL_CULL_FACE);
		if (stState.eCullMode == PCM_BACK) glCullFace(GL_BACK);
		else if (stState.eCullMode == PCM_FRONT) glCullFace(GL_FRONT);
	}
	// TODO: rest
	
	E_GUARDS();
	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetRasterizerState(TRasterizerStateDesc& stState)
{ 
	E_GUARDS();

	stState.bAlphaTestEnabled = alphaTest;

	GLint poligonMode[2];
	glGetIntegerv(GL_POLYGON_MODE, poligonMode);
	stState.bWireframe = poligonMode[0] == GL_LINE;

	GLboolean enabledCulling;
	enabledCulling = glIsEnabled(GL_CULL_FACE);
	if (enabledCulling == GL_FALSE)
		stState.eCullMode = PCM_NONE;
	else
	{
		GLint cullMode;
		glGetIntegerv(GL_CULL_FACE_MODE, &cullMode);
		if (cullMode == GL_BACK) stState.eCullMode = PCM_BACK;
		else if (cullMode == GL_FRONT) stState.eCullMode = PCM_FRONT;
	}	
	
	// TODO: rest

	E_GUARDS();
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
	assert(uiTextureLayer == 0);
	
	glActiveTexture(GL_TEXTURE0 + 0);

	GLint tex_id = tex_ID_last_binded;
	//glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_id);

	IResourceManager *resMan;
	_core->GetSubSystem(ESS_RESOURCE_MANAGER, reinterpret_cast<IEngineSubSystem *&>(resMan));

	uint count;
	resMan->GetResourcesCount(count);

	for (uint i = 0; i < count; ++i)
	{
		IEngineBaseObject *p_obj;
		resMan->GetResourceByIndex(i, p_obj);

		E_ENGINE_OBJECT_TYPE type;
		p_obj->GetType(type);

		if (type == EOT_TEXTURE)
		{
			ICoreTexture *p_ctex;
			((ITexture *)p_obj)->GetCoreTexture(p_ctex);

			if (((GLTexture*)p_ctex)->Texture_ID() == tex_id)
			{
				prTex = p_ctex;
				return S_OK;
			}
		}
	}

	return S_OK;
}

DGLE_RESULT DGLE_API GL3XCoreRender::GetFixedFunctionPipelineAPI(IFixedFunctionPipeline*& prFFP)
{ 
	prFFP = nullptr;
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
	case CRFT_BGRA_DATA_FORMAT: bIsSupported = true; break;
	case CRFT_TEXTURE_COMPRESSION: bIsSupported = (GLEW_ARB_texture_compression == GL_TRUE && GLEW_EXT_texture_compression_s3tc == GL_TRUE); break;
	case CRFT_NON_POWER_OF_TWO_TEXTURES: bIsSupported = true; break;
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

