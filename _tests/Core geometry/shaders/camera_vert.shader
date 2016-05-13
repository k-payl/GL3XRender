#version 330

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoordinata;

uniform mat4 MVP;

smooth out vec3 N;


void main()
{
	N = Normal;
	gl_Position = MVP * vec4(Position, 1.0);
}
