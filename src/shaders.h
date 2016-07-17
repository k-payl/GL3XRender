static const char *p_v[] = // Vertex shader with input: Position
{
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
 "\n"
};

static const char *p_f[] = // Fragment shader with const white color
{
 "#version 330\n",
 "\n",
 "out vec4 color;\n",
 "\n",
 "\n",
 "void main()\n",
 "{\n",
 "	color = vec4(1, 1, 1, 1);\n",
 "}\n",
 "\n"
};

static const char *pn_v[] = // Vertex shader with input: Position, Normal
{
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
 "\n"
};

static const char *pn_f[] = // Fragment shader with interpolated attributes: Normal
{
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
 "\n"
};

static const char *pnt_v[] = // Vertex shader with input: Position, Normal, Texture coordiantes
{
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
 "\n"
};

static const char *pnt_f[] = // Fragment shader with interpolated attributes: Normal, Texture coordiantes
{
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
 "\n"
};

static const char *pt_v[] = // Vertex shader with input: Position, Texture coordiantes
{
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
 "\n"
};

static const char *pt_f[] = // Fragment shader with interpolated attributes: Texture coordiantes
{
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
 "\n"
};

static const char *pt2d_v[] = // Vertex shader with input: Position (vec2), Texture coordiantes
{
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
 "	float x = Position.x / screenWidth;\n",
 "	float y = Position.y / screenHeight;\n",
 "	gl_Position = vec4(x, y, 0.0, 1.0);\n",
 "}\n",
 "\n"
};

static const char *pt2d_f[] = // Fragment shader with interpolated attributes: Texture coordiantes
{
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
 "\n"
};

