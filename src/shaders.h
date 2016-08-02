#include<array>

struct ShaderGenerated
{
	const char *descr;
	const char **ppTxtVertex;
	const char **ppTxtFragment;
	const unsigned int linesVertexShader;
	const unsigned int linesFragmentShader;
	const CORE_GEOMETRY_ATTRIBUTES_PRESENTED attribs;
	const bool bPositionIs2D;
	const bool bUniformNM;
	const bool bUniformnL;
	const bool bUniformTexture;
};

extern std::array<ShaderGenerated, 5> _shadersGenerated;
