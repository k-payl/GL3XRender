#include<vector>
#include <unordered_set>

enum CORE_GEOMETRY_ATTRIBUTES_PRESENTED;

struct ShaderSrc
{
	const char *descr;
	const char **ppTxtVertex;
	const char **ppTxtFragment;
	const unsigned int linesVertexShader;
	const unsigned int linesFragmentShader;
	const CORE_GEOMETRY_ATTRIBUTES_PRESENTED attribs;
	const bool bPositionIsVec2;
	const bool bAlphaTest;
	const std::unordered_set<std::string> uniforms;
};

const std::vector<ShaderSrc>& getShaderSources();