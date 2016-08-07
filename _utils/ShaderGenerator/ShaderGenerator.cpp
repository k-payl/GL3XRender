//
// This application load shaders
// with different input attributes such as
// Position, Normal, Texture coordiantes, Tangent...
// and makes one text file.
// Then this file you can paste to main .cpp plugin file.
//
#include <string>
#include <fstream>
#include <array>
#include <set>
using namespace std;

#define DIR "..\\..\\src\\shaders\\"
#define OUT_CPP "../../src/shaderSources.cpp"


const array<string, 9> head= 
{ {
	"#include \"GL3XCoreRender.h\"",
	"#include \"shaderSources.h\"",
	"",
	"template<int N>",
	"static const char** exact_ptrptr(const char *(&v)[N])",
	"{",
	"	return &v[0];",
	"}",
} };

#define SH string, string, string, bool, set<string>

void _write_shader_text(ofstream& file, string shd_name, char var, int ind)
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

void write_shader_text(ofstream& file, const tuple<SH>& shdr)
{	
	static int ind = 0;

	_write_shader_text(file, get<0>(shdr), 'v', ind);
	_write_shader_text(file, get<1>(shdr), 'f', ind);

	ind++;
}

void write_shader_fields(ofstream& file, const tuple<SH>& shdr)
{
	int static ind = 0;

	file << "{" << endl;
	file << "\t\"Shader" << ind << "\"," << endl;
	file << "\texact_ptrptr(v" << ind << ")," << endl;
	file << "\texact_ptrptr(f" << ind << ")," << endl;
	file << "\t_countof(v" << ind << ") - 1," << endl;
	file << "\t_countof(f" << ind << ") - 1," << endl;
	file << '\t' << get<2>(shdr) << ',' << endl;
	file << (get<3>(shdr) ? "\ttrue," : "\tfalse,") << endl;
	file << "\t{";
	for (auto it = get<4>(shdr).begin(); it != get<4>(shdr).end(); it++)
	{
		file << "\"" <<*it << "\"";
		if (it != (--get<4>(shdr).end())) file << ", ";
	}
	file << "}" << endl;
	file << "}," << endl;

	ind++;
}

int main()
{	
	const array<tuple<SH>, 5> shdrs =
	{ {
		tuple<SH>{ "camera_p_vert.shader",		"camera_p_frag.shader",		"CGAP_POS",			false,	{"MVP"}},
		tuple<SH>{ "camera_pn_vert.shader",		"camera_pn_frag.shader",	"CGAP_POS_NORM",	false,	{"MVP", "NM", "nL"}},
		tuple<SH>{ "camera_pnt_vert.shader",	"camera_pnt_frag.shader",	"CGAP_POS_NORM_TEX",false,	{"MVP", "NM", "nL", "texture0"}},
		tuple<SH>{ "camera_pt_vert.shader",		"camera_pt_frag.shader",	"CGAP_POS_TEX",		false,	{"MVP", "texture0"}},
		tuple<SH>{ "camera_pt_2d_vert.shader",	"camera_pt_2d_frag.shader",	"CGAP_POS_TEX",		true,	{"screenWidth", "screenHeight", "texture0"}}
	} };
	

	ofstream out_cpp(OUT_CPP);
	for each (const string& l in head)
		out_cpp << l << endl;
	out_cpp << endl;
	for each (const auto& v in shdrs)
		write_shader_text(out_cpp, v);
	out_cpp << "static std::vector<ShaderSrc> _shadersGenerated =" << endl;
	out_cpp << "{{" << endl;
	for each (const auto& v in shdrs)
		write_shader_fields(out_cpp, v);
	out_cpp << "}};" << endl << endl;
	out_cpp << "const std::vector<ShaderSrc>& getShaderSources()" << endl;
	out_cpp << "{" << endl;
	out_cpp << "	return _shadersGenerated;" << endl;
	out_cpp << "}" << endl;
	out_cpp.close();

}

#undef SH