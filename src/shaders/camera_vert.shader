#version 330

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;

uniform mat4 MVP;
uniform mat4 NM;

smooth out vec3 N;
smooth out vec2 UV;


void main()
{
	N = (NM * vec4(Normal, 0)).xyz;
	UV = TexCoord;

	gl_Position = MVP * vec4(Position, 1.0);
}
