#include<vector>

enum CORE_GEOMETRY_ATTRIBUTES_PRESENTED;

struct ShaderSrc
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

const std::vector<ShaderSrc>& getShaderSources();