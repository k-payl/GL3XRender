#version 330

layout(location = 0) in vec2 Position;

uniform mat4 MVP;


void main()
{
	gl_Position = MVP * vec4(Position.x, Position.y, 0.0, 1.0);
}
