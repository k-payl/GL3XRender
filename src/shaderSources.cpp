#include "GL3XCoreRender.h"
#include "shaderSources.h"

template<int N>
static const char** exact_ptrptr(const char *(&v)[N])
{
	return &v[0];
}


static const char *v0[] = {
 "#version 330\n",
 "layout(location = 0) in vec3 Position;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "void main()\n",
 "{\n",
 "		gl_Position = MVP * vec4(Position, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f0[] = {
 "#version 330\n",
 "uniform vec4 main_color;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	color_out = main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v1[] = {
 "#version 330\n",
 "layout(location = 0) in vec3 Position;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "void main()\n",
 "{\n",
 "		gl_Position = MVP * vec4(Position, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f1[] = {
 "#version 330\n",
 "uniform vec4 main_color;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	color_out = main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v2[] = {
 "#version 330\n",
 "layout(location = 0) in vec3 Position;\n",
 "layout(location = 2) in vec2 TexCoord;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "smooth out vec2 UV;\n",
 "void main()\n",
 "{\n",
 "		UV = TexCoord;\n",
 "		gl_Position = MVP * vec4(Position, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f2[] = {
 "#version 330\n",
 "smooth in vec2 UV;\n",
 "uniform vec4 main_color;\n",
 "uniform sampler2D texture0;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	vec4 tex = texture(texture0, UV);\n",
 "	//tex.rgb = pow(tex.rgb, vec3(2.2f, 2.2f, 2.2f));\n",
 "	color_out = tex * main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v3[] = {
 "#version 330\n",
 "layout(location = 0) in vec3 Position;\n",
 "layout(location = 2) in vec2 TexCoord;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "smooth out vec2 UV;\n",
 "void main()\n",
 "{\n",
 "		UV = TexCoord;\n",
 "		gl_Position = MVP * vec4(Position, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f3[] = {
 "#version 330\n",
 "smooth in vec2 UV;\n",
 "uniform vec4 main_color;\n",
 "uniform sampler2D texture0;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	vec4 tex = texture(texture0, UV);\n",
 "	//tex.rgb = pow(tex.rgb, vec3(2.2f, 2.2f, 2.2f));\n",
 "	if (tex.a <= 0.5)\n",
 "		discard;\n",
 "	color_out = tex * main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v4[] = {
 "#version 330\n",
 "layout(location = 0) in vec3 Position;\n",
 "layout(location = 1) in vec3 Normal;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "uniform mat4 NM;\n",
 "smooth out vec3 N;\n",
 "void main()\n",
 "{\n",
 "		N = (NM * vec4(Normal, 0)).xyz;\n",
 "		gl_Position = MVP * vec4(Position, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f4[] = {
 "#version 330\n",
 "smooth in vec3 N;\n",
 "uniform vec3 nL;\n",
 "uniform vec4 main_color;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	vec3 nN = normalize(N);\n",
 "	color_out = vec4(vec3(max(dot(nN, nL), 0)), 1) * main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v5[] = {
 "#version 330\n",
 "layout(location = 0) in vec3 Position;\n",
 "layout(location = 1) in vec3 Normal;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "uniform mat4 NM;\n",
 "smooth out vec3 N;\n",
 "void main()\n",
 "{\n",
 "		N = (NM * vec4(Normal, 0)).xyz;\n",
 "		gl_Position = MVP * vec4(Position, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f5[] = {
 "#version 330\n",
 "smooth in vec3 N;\n",
 "uniform vec3 nL;\n",
 "uniform vec4 main_color;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	vec3 nN = normalize(N);\n",
 "	color_out = vec4(vec3(max(dot(nN, nL), 0)), 1) * main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v6[] = {
 "#version 330\n",
 "layout(location = 0) in vec3 Position;\n",
 "layout(location = 1) in vec3 Normal;\n",
 "layout(location = 2) in vec2 TexCoord;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "uniform mat4 NM;\n",
 "smooth out vec3 N;\n",
 "smooth out vec2 UV;\n",
 "void main()\n",
 "{\n",
 "		N = (NM * vec4(Normal, 0)).xyz;\n",
 "		UV = TexCoord;\n",
 "		gl_Position = MVP * vec4(Position, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f6[] = {
 "#version 330\n",
 "smooth in vec3 N;\n",
 "smooth in vec2 UV;\n",
 "uniform vec3 nL;\n",
 "uniform vec4 main_color;\n",
 "uniform sampler2D texture0;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	vec3 nN = normalize(N);\n",
 "	vec4 tex = texture(texture0, UV);\n",
 "	//tex.rgb = pow(tex.rgb, vec3(2.2f, 2.2f, 2.2f));\n",
 "	color_out = vec4(vec3(max(dot(nN, nL), 0)), 1) * tex * main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v7[] = {
 "#version 330\n",
 "layout(location = 0) in vec3 Position;\n",
 "layout(location = 1) in vec3 Normal;\n",
 "layout(location = 2) in vec2 TexCoord;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "uniform mat4 NM;\n",
 "smooth out vec3 N;\n",
 "smooth out vec2 UV;\n",
 "void main()\n",
 "{\n",
 "		N = (NM * vec4(Normal, 0)).xyz;\n",
 "		UV = TexCoord;\n",
 "		gl_Position = MVP * vec4(Position, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f7[] = {
 "#version 330\n",
 "smooth in vec3 N;\n",
 "smooth in vec2 UV;\n",
 "uniform vec3 nL;\n",
 "uniform vec4 main_color;\n",
 "uniform sampler2D texture0;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	vec3 nN = normalize(N);\n",
 "	vec4 tex = texture(texture0, UV);\n",
 "	//tex.rgb = pow(tex.rgb, vec3(2.2f, 2.2f, 2.2f));\n",
 "	if (tex.a <= 0.5)\n",
 "		discard;\n",
 "	color_out = vec4(vec3(max(dot(nN, nL), 0)), 1) * tex * main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v8[] = {
 "#version 330\n",
 "layout(location = 0) in vec2 Position;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "//uniform uint screenWidth;\n",
 "//uniform uint screenHeight;\n",
 "//\n",
 "void main()\n",
 "{\n",
 "		gl_Position = MVP * vec4(Position.x, Position.y, 0.0, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f8[] = {
 "#version 330\n",
 "uniform vec4 main_color;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	color_out = main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v9[] = {
 "#version 330\n",
 "layout(location = 0) in vec2 Position;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "//uniform uint screenWidth;\n",
 "//uniform uint screenHeight;\n",
 "//\n",
 "void main()\n",
 "{\n",
 "		gl_Position = MVP * vec4(Position.x, Position.y, 0.0, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f9[] = {
 "#version 330\n",
 "uniform vec4 main_color;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	color_out = main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v10[] = {
 "#version 330\n",
 "layout(location = 0) in vec2 Position;\n",
 "layout(location = 2) in vec2 TexCoord;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "//uniform uint screenWidth;\n",
 "//uniform uint screenHeight;\n",
 "//\n",
 "smooth out vec2 UV;\n",
 "void main()\n",
 "{\n",
 "		UV = TexCoord;\n",
 "		gl_Position = MVP * vec4(Position.x, Position.y, 0.0, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f10[] = {
 "#version 330\n",
 "smooth in vec2 UV;\n",
 "uniform vec4 main_color;\n",
 "uniform sampler2D texture0;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	vec4 tex = texture(texture0, UV);\n",
 "	//tex.rgb = pow(tex.rgb, vec3(2.2f, 2.2f, 2.2f));\n",
 "	color_out = tex * main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v11[] = {
 "#version 330\n",
 "layout(location = 0) in vec2 Position;\n",
 "layout(location = 2) in vec2 TexCoord;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "//uniform uint screenWidth;\n",
 "//uniform uint screenHeight;\n",
 "//\n",
 "smooth out vec2 UV;\n",
 "void main()\n",
 "{\n",
 "		UV = TexCoord;\n",
 "		gl_Position = MVP * vec4(Position.x, Position.y, 0.0, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f11[] = {
 "#version 330\n",
 "smooth in vec2 UV;\n",
 "uniform vec4 main_color;\n",
 "uniform sampler2D texture0;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	vec4 tex = texture(texture0, UV);\n",
 "	//tex.rgb = pow(tex.rgb, vec3(2.2f, 2.2f, 2.2f));\n",
 "	if (tex.a <= 0.5)\n",
 "		discard;\n",
 "	color_out = tex * main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v12[] = {
 "#version 330\n",
 "layout(location = 0) in vec2 Position;\n",
 "layout(location = 1) in vec3 Normal;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "//uniform uint screenWidth;\n",
 "//uniform uint screenHeight;\n",
 "//\n",
 "uniform mat4 NM;\n",
 "smooth out vec3 N;\n",
 "void main()\n",
 "{\n",
 "		N = (NM * vec4(Normal, 0)).xyz;\n",
 "		gl_Position = MVP * vec4(Position.x, Position.y, 0.0, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f12[] = {
 "#version 330\n",
 "smooth in vec3 N;\n",
 "uniform vec3 nL;\n",
 "uniform vec4 main_color;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	vec3 nN = normalize(N);\n",
 "	color_out = vec4(vec3(max(dot(nN, nL), 0)), 1) * main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v13[] = {
 "#version 330\n",
 "layout(location = 0) in vec2 Position;\n",
 "layout(location = 1) in vec3 Normal;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "//uniform uint screenWidth;\n",
 "//uniform uint screenHeight;\n",
 "//\n",
 "uniform mat4 NM;\n",
 "smooth out vec3 N;\n",
 "void main()\n",
 "{\n",
 "		N = (NM * vec4(Normal, 0)).xyz;\n",
 "		gl_Position = MVP * vec4(Position.x, Position.y, 0.0, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f13[] = {
 "#version 330\n",
 "smooth in vec3 N;\n",
 "uniform vec3 nL;\n",
 "uniform vec4 main_color;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	vec3 nN = normalize(N);\n",
 "	color_out = vec4(vec3(max(dot(nN, nL), 0)), 1) * main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v14[] = {
 "#version 330\n",
 "layout(location = 0) in vec2 Position;\n",
 "layout(location = 1) in vec3 Normal;\n",
 "layout(location = 2) in vec2 TexCoord;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "//uniform uint screenWidth;\n",
 "//uniform uint screenHeight;\n",
 "//\n",
 "uniform mat4 NM;\n",
 "smooth out vec3 N;\n",
 "smooth out vec2 UV;\n",
 "void main()\n",
 "{\n",
 "		N = (NM * vec4(Normal, 0)).xyz;\n",
 "		UV = TexCoord;\n",
 "		gl_Position = MVP * vec4(Position.x, Position.y, 0.0, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f14[] = {
 "#version 330\n",
 "smooth in vec3 N;\n",
 "smooth in vec2 UV;\n",
 "uniform vec3 nL;\n",
 "uniform vec4 main_color;\n",
 "uniform sampler2D texture0;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	vec3 nN = normalize(N);\n",
 "	vec4 tex = texture(texture0, UV);\n",
 "	//tex.rgb = pow(tex.rgb, vec3(2.2f, 2.2f, 2.2f));\n",
 "	color_out = vec4(vec3(max(dot(nN, nL), 0)), 1) * tex * main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
 "}\n",
 "\n",
 nullptr
};

static const char *v15[] = {
 "#version 330\n",
 "layout(location = 0) in vec2 Position;\n",
 "layout(location = 1) in vec3 Normal;\n",
 "layout(location = 2) in vec2 TexCoord;\n",
 "uniform mat4 MVP;\n",
 "//\n",
 "//uniform uint screenWidth;\n",
 "//uniform uint screenHeight;\n",
 "//\n",
 "uniform mat4 NM;\n",
 "smooth out vec3 N;\n",
 "smooth out vec2 UV;\n",
 "void main()\n",
 "{\n",
 "		N = (NM * vec4(Normal, 0)).xyz;\n",
 "		UV = TexCoord;\n",
 "		gl_Position = MVP * vec4(Position.x, Position.y, 0.0, 1.0);\n",
 "}\n",
 "\n",
 nullptr
};

static const char *f15[] = {
 "#version 330\n",
 "smooth in vec3 N;\n",
 "smooth in vec2 UV;\n",
 "uniform vec3 nL;\n",
 "uniform vec4 main_color;\n",
 "uniform sampler2D texture0;\n",
 "out vec4 color_out;\n",
 "void main()\n",
 "{\n",
 "	vec3 nN = normalize(N);\n",
 "	vec4 tex = texture(texture0, UV);\n",
 "	//tex.rgb = pow(tex.rgb, vec3(2.2f, 2.2f, 2.2f));\n",
 "	if (tex.a <= 0.5)\n",
 "		discard;\n",
 "	color_out = vec4(vec3(max(dot(nN, nL), 0)), 1) * tex * main_color;\n",
 "	//color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));\n",
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
	POS,
	false,
	false,
},
{
	"Shader1",
	exact_ptrptr(v1),
	exact_ptrptr(f1),
	_countof(v1) - 1,
	_countof(f1) - 1,
	POS,
	false,
	true,
},
{
	"Shader2",
	exact_ptrptr(v2),
	exact_ptrptr(f2),
	_countof(v2) - 1,
	_countof(f2) - 1,
	POS | TEX_COORD,
	false,
	false,
},
{
	"Shader3",
	exact_ptrptr(v3),
	exact_ptrptr(f3),
	_countof(v3) - 1,
	_countof(f3) - 1,
	POS | TEX_COORD,
	false,
	true,
},
{
	"Shader4",
	exact_ptrptr(v4),
	exact_ptrptr(f4),
	_countof(v4) - 1,
	_countof(f4) - 1,
	POS | NORM,
	false,
	false,
},
{
	"Shader5",
	exact_ptrptr(v5),
	exact_ptrptr(f5),
	_countof(v5) - 1,
	_countof(f5) - 1,
	POS | NORM,
	false,
	true,
},
{
	"Shader6",
	exact_ptrptr(v6),
	exact_ptrptr(f6),
	_countof(v6) - 1,
	_countof(f6) - 1,
	POS | NORM | TEX_COORD,
	false,
	false,
},
{
	"Shader7",
	exact_ptrptr(v7),
	exact_ptrptr(f7),
	_countof(v7) - 1,
	_countof(f7) - 1,
	POS | NORM | TEX_COORD,
	false,
	true,
},
{
	"Shader8",
	exact_ptrptr(v8),
	exact_ptrptr(f8),
	_countof(v8) - 1,
	_countof(f8) - 1,
	POS,
	true,
	false,
},
{
	"Shader9",
	exact_ptrptr(v9),
	exact_ptrptr(f9),
	_countof(v9) - 1,
	_countof(f9) - 1,
	POS,
	true,
	true,
},
{
	"Shader10",
	exact_ptrptr(v10),
	exact_ptrptr(f10),
	_countof(v10) - 1,
	_countof(f10) - 1,
	POS | TEX_COORD,
	true,
	false,
},
{
	"Shader11",
	exact_ptrptr(v11),
	exact_ptrptr(f11),
	_countof(v11) - 1,
	_countof(f11) - 1,
	POS | TEX_COORD,
	true,
	true,
},
{
	"Shader12",
	exact_ptrptr(v12),
	exact_ptrptr(f12),
	_countof(v12) - 1,
	_countof(f12) - 1,
	POS | NORM,
	true,
	false,
},
{
	"Shader13",
	exact_ptrptr(v13),
	exact_ptrptr(f13),
	_countof(v13) - 1,
	_countof(f13) - 1,
	POS | NORM,
	true,
	true,
},
{
	"Shader14",
	exact_ptrptr(v14),
	exact_ptrptr(f14),
	_countof(v14) - 1,
	_countof(f14) - 1,
	POS | NORM | TEX_COORD,
	true,
	false,
},
{
	"Shader15",
	exact_ptrptr(v15),
	exact_ptrptr(f15),
	_countof(v15) - 1,
	_countof(f15) - 1,
	POS | NORM | TEX_COORD,
	true,
	true,
},
}};
const std::vector<ShaderSrc>& getShaderSources()
{
	return _shadersGenerated;
}
