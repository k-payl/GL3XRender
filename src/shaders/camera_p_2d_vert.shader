#version 330

layout(location = 0) in vec2 Position;

uniform uint screenWidth;
uniform uint screenHeight;


void main()
{
	float x = (Position.x / screenWidth) * 2 - 1;
	float y = - (Position.y / screenHeight) * 2 + 1;
	gl_Position = vec4(x, y, 0.0, 1.0);
}
