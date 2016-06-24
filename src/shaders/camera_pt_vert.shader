#version 330

layout(location = 0) in vec3 Position;
layout(location = 2) in vec2 TexCoord;

uniform mat4 MVP;

smooth out vec2 UV;


void main()
{
	UV = TexCoord;
	gl_Position = MVP * vec4(Position, 1.0);
}
