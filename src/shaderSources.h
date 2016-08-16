#include<vector>
#include <unordered_set>

enum INPUT_ATTRIBUTE;

struct ShaderSrc
{
	const char *descr;
	const char **ppTxtVertex;
	const char **ppTxtFragment;
	const unsigned int linesVertexShader;
	const unsigned int linesFragmentShader;
	const INPUT_ATTRIBUTE attribs;
	const bool bPositionIsVec2;
	const bool bAlphaTest;
	const std::unordered_set<std::string> uniforms;
};

const std::vector<ShaderSrc>& getShaderSources();