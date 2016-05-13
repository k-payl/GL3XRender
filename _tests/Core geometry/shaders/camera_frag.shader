#version 330

smooth in vec3 N;

const vec3 L = vec3(1.0, -0.0, 2.0);
const vec3 nL = normalize(L);

out vec4 color;


void main()
{
	vec3 nN = normalize(N);
	color = vec4(vec3(max(dot(nN, nL), 0.0)), 1.0);
}