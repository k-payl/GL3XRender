#include "GL3XCoreRender.h"
#include "shaderSources.h"

template<int N>
static const char** exact_ptrptr(const char *(&v)[N])
{
	return &v[0];
}


static const char *v0[] = {
 "#version 330\n",
 "\n",
 "layout(location = 0) in vec3 Position;\n",
 "\n",
 "uniform mat4 MVP;\n",
 "\n",
 "\n",
 "void main()\n",
 "{\n",
 "	gl_Position = MVP * vec4(Position, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f0[] = {
 "#version 330\n",
 "\n",
 "out vec4 color;\n",
 "\n",
 "\n",
 "void main()\n",
 "{\n",
 "	color = vec4(1, 1, 1, 1);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v1[] = {
 "#version 330\n",
 "\n",
 "layout(location = 0) in vec3 Position;\n",
 "layout(location = 1) in vec3 Normal;\n",
 "\n",
 "uniform mat4 MVP;\n",
 "uniform mat4 NM;\n",
 "\n",
 "smooth out vec3 N;\n",
 "\n",
 "\n",
 "void main()\n",
 "{\n",
 "	N = (NM * vec4(Normal, 0)).xyz;\n",
 "\n",
 "	gl_Position = MVP * vec4(Position, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f1[] = {
 "#version 330\n",
 "\n",
 "smooth in vec3 N;\n",
 "\n",
 "uniform vec3 nL;\n",
 "\n",
 "out vec4 color;\n",
 "\n",
 "\n",
 "void main()\n",
 "{\n",
 "	vec3 nN = normalize(N);\n",
 "	color = vec4(vec3(max(dot(nN, nL), 0)), 1);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v2[] = {
 "#version 330\n",
 "\n",
 "layout(location = 0) in vec3 Position;\n",
 "layout(location = 1) in vec3 Normal;\n",
 "layout(location = 2) in vec2 TexCoord;\n",
 "\n",
 "uniform mat4 MVP;\n",
 "uniform mat4 NM;\n",
 "\n",
 "smooth out vec3 N;\n",
 "smooth out vec2 UV;\n",
 "\n",
 "\n",
 "void main()\n",
 "{\n",
 "	N = (NM * vec4(Normal, 0)).xyz;\n",
 "	UV = TexCoord;\n",
 "\n",
 "	gl_Position = MVP * vec4(Position, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f2[] = {
 "#version 330\n",
 "\n",
 "smooth in vec3 N;\n",
 "smooth in vec2 UV;\n",
 "\n",
 "uniform vec3 nL;\n",
 "uniform sampler2D texture0;\n",
 "\n",
 "out vec4 color;\n",
 "\n",
 "\n",
 "void main()\n",
 "{\n",
 "	vec3 nN = normalize(N);\n",
 "	vec4 Texture0Color = texture(texture0, UV);\n",
 "	color = vec4(vec3(max(dot(nN, nL), 0)), 1) * Texture0Color;\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v3[] = {
 "#version 330\n",
 "\n",
 "layout(location = 0) in vec3 Position;\n",
 "layout(location = 2) in vec2 TexCoord;\n",
 "\n",
 "uniform mat4 MVP;\n",
 "\n",
 "smooth out vec2 UV;\n",
 "\n",
 "\n",
 "void main()\n",
 "{\n",
 "	UV = TexCoord;\n",
 "	gl_Position = MVP * vec4(Position, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f3[] = {
 "#version 330\n",
 "\n",
 "smooth in vec2 UV;\n",
 "\n",
 "uniform sampler2D texture0;\n",
 "\n",
 "out vec4 color;\n",
 "\n",
 "\n",
 "void main()\n",
 "{\n",
 "	color = texture(texture0, UV);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v4[] = {
 "#version 330\n",
 "\n",
 "layout(location = 0) in vec2 Position;\n",
 "layout(location = 2) in vec2 TexCoord;\n",
 "\n",
 "uniform uint screenWidth;\n",
 "uniform uint screenHeight;\n",
 "\n",
 "smooth out vec2 UV;\n",
 "\n",
 "\n",
 "void main()\n",
 "{\n",
 "	UV = TexCoord;\n",
 "	float x = (Position.x / screenWidth) * 2 - 1;\n",
 "	float y = - (Position.y / screenHeight) * 2 + 1;\n",
 "	gl_Position = vec4(x, y, 0.0, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f4[] = {
 "#version 330\n",
 "\n",
 "smooth in vec2 UV;\n",
 "\n",
 "uniform sampler2D texture0;\n",
 "\n",
 "out vec4 color;\n",
 "\n",
 "\n",
 "void main()\n",
 "{\n",
 "	vec4 tex = texture(texture0, UV);\n",
 "	color = vec4(1, 1, 1, tex.r);\n",
 "}\n",
 "\n",
 nullptr
};

static std::vector<ShaderSrc> _shadersGenerated =
{{
{
	"Shader0",
	exact_ptrptr(v0),
	exact_ptrptr(f0),
	_countof(v0) - 1,
	_countof(f0) - 1,
	CGAP_POS,
	false,
	false,
	false,
	false,
},
{
	"Shader1",
	exact_ptrptr(v1),
	exact_ptrptr(f1),
	_countof(v1) - 1,
	_countof(f1) - 1,
	CGAP_POS_NORM,
	false,
	true,
	true,
	false,
},
{
	"Shader2",
	exact_ptrptr(v2),
	exact_ptrptr(f2),
	_countof(v2) - 1,
	_countof(f2) - 1,
	CGAP_POS_NORM_TEX,
	false,
	true,
	true,
	true,
},
{
	"Shader3",
	exact_ptrptr(v3),
	exact_ptrptr(f3),
	_countof(v3) - 1,
	_countof(f3) - 1,
	CGAP_POS_TEX,
	false,
	false,
	false,
	true,
},
{
	"Shader4",
	exact_ptrptr(v4),
	exact_ptrptr(f4),
	_countof(v4) - 1,
	_countof(f4) - 1,
	CGAP_POS_TEX,
	true,
	false,
	false,
	true,
},
}};

const std::vector<ShaderSrc>& getShaderSources()
{
	return _shadersGenerated;
}