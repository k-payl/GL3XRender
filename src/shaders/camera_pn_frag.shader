#version 330

smooth in vec3 N;

uniform vec3 nL;

out vec4 color;


void main()
{
	vec3 nN = normalize(N);
	color = vec4(vec3(max(dot(nN, nL), 0)), 1);
}