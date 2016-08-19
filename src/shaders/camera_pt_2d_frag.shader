#version 330

smooth in vec2 UV;

uniform vec4 main_color;
uniform sampler2D texture0;

out vec4 color;


void main()
{
	vec4 tex = texture(texture0, UV);

#if ALPHA_TEST
	if (tex.a <= 0.5)
		discard;
#endif

	color = tex * main_color;
}