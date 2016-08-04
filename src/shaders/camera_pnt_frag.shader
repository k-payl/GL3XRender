#version 330

smooth in vec3 N;
smooth in vec2 UV;

uniform vec3 nL;
uniform sampler2D texture0;

out vec4 color;


void main()
{
	vec3 nN = normalize(N);
	vec4 tex = texture(texture0, UV);

#if ALPHA_TEST
	if (tex.a <= 0.5)
		discard;
#endif
	
	color = vec4(vec3(max(dot(nN, nL), 0)), 1) * tex;
}