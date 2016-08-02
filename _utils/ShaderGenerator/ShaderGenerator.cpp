//
// This application load shaders
// with different input attributes such as
// Position, Normal, Texture coordiantes, Tangent...
// and makes one text file.
// Then this file you can paste to main .cpp plugin file.
//
#include <string>
#include <fstream>
#include <iostream>
#include <array>
#include <string>
using namespace std;

#define DIR "..\\..\\src\\shaders\\"
#define OUT_H "../../src/shaders.h"
#define OUT_CPP "../../src/shaders.cpp"


const array<string, 9> head= 
{ {
	"#include \"GL3XCoreRender.h\"",
	"#include \"shaders.h\"",
	"#include <utility>",
	"",
	"template<int N>",
	"const char** exact_ptrptr(const char *(&v)[N])",
	"{",
	"	return &v[0];",
	"}",
} };

const array<string, 15> head1 =
{ {
	"#include<array>",
	"",
	"struct ShaderGenerated",
	"{",
	"	const char *descr;",
	"	const char **ppTxtVertex;",
	"	const char **ppTxtFragment;",
	"	const unsigned int linesVertexShader;",
	"	const unsigned int linesFragmentShader;",
	"	const CORE_GEOMETRY_ATTRIBUTES_PRESENTED attribs;",
	"	const bool bPositionIs2D;",
	"	const bool bUniformNM;",
	"	const bool bUniformnL;",
	"	const bool bUniformTexture;",
	"};"
} };

void write_block(ofstream& file, string shd_name, char var, int ind)
{
	string line;
	ifstream shd(string(DIR) + shd_name);
	file << "static const char *" << var << ind << "[] = ";
	file << "{" << endl;
	while (getline(shd, line))
	{
		file << ' ' << '\"' << line << "\\n\"," << endl;
	}
	file << ' ' << "\"\\n\"," << endl;
	file << ' ' << "nullptr" << endl;
	file << "};" << endl << endl;
	shd.close();
}

void write_shader_text(ofstream& file, const tuple<string, string, string, bool, bool, bool, bool>& shdr)
{
	
	static int ind = 0;

	write_block(file, get<0>(shdr), 'v', ind);
	write_block(file, get<1>(shdr), 'f', ind);

	ind++;
}

void write_shader_info(ofstream& file, const tuple<string, string, string, bool, bool, bool, bool>& shdr)
{
	int static ind = 0;

	file << "{" << endl;
	file << "\t\"Shader"<< ind << "\"," << endl;
	file << "\texact_ptrptr(v" << ind << ")," << endl;
	file << "\texact_ptrptr(f" << ind << ")," << endl;
	file << "\t_countof(v" << ind << ") - 1," << endl;
	file << "\t_countof(f" << ind << ") - 1," << endl;
	file << '\t' << get<2>(shdr) <<',' << endl;
	file << (get<3>(shdr)? "\ttrue," : "\tfalse,") << endl;
	file << (get<4>(shdr)? "\ttrue," : "\tfalse,") << endl;
	file << (get<5>(shdr)? "\ttrue," : "\tfalse,") << endl;
	file << (get<6>(shdr)? "\ttrue," : "\tfalse,") << endl;
	file << "}," << endl;

	ind++;
}

int main()
{
	const array<tuple<string, string, string, bool, bool, bool, bool>, 5> data = 
	{{
		{ "camera_p_vert.shader",		"camera_p_frag.shader",		"CGAP_POS",			false,	false,	false,	false},
		{ "camera_pn_vert.shader" ,		"camera_pn_frag.shader",	"CGAP_POS_NORM",	false,	true,	true,	false},
		{ "camera_pnt_vert.shader",		"camera_pnt_frag.shader",	"CGAP_POS_NORM_TEX",false,	true,	true,	true },
		{ "camera_pt_vert.shader" ,		"camera_pt_frag.shader",	"CGAP_POS_TEX",		false,	false,	false,	true },
		{ "camera_pt_2d_vert.shader",	"camera_pt_2d_frag.shader",	"CGAP_POS_TEX",		true,	false,	false,	true }
	}};


	ofstream out_h(OUT_H);
	for each (const string& l in head1)
		out_h << l << endl;
	out_h << endl;
	out_h << "extern std::array<ShaderGenerated, " <<  data.size() << "> _shadersGenerated;" <<endl; 
	out_h.close();

	ofstream out_cpp(OUT_CPP);
	for each (const string& l in head)
		out_cpp << l << endl;
	out_cpp << endl;
	for each (const auto& v in data)
		write_shader_text(out_cpp, v);
	out_cpp << "std::array<ShaderGenerated, " << data.size() << "> _shadersGenerated =" << endl;
	out_cpp << "{{" << endl;
	for each (const auto& v in data)
		write_shader_info(out_cpp, v);
	out_cpp << "}};" << endl;
	out_cpp.close();

}

