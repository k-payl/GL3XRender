#version 330

layout(location = 0) in vec2 Position;
layout(location = 2) in vec2 TexCoord;

uniform uint screenWidth;
uniform uint screenHeight;

smooth out vec2 UV;


void main()
{
	UV = TexCoord;
	float x = Position.x / screenWidth;
	float y = Position.y / screenHeight;
	gl_Position = vec4(x, y, 0.0, 1.0);
}
