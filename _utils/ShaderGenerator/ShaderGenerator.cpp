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
#include <set>
#include <vector>
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

void _write_shader_text(ofstream& file, vector<string> lines_vec, char var, int ind)
{
	file << "static const char *" << var << ind << "[] = ";
	file << "{" << endl;
	for each (const string& s in lines_vec)
	{
		file << ' ' << '\"' << s << "\"," << endl;
	}
	file << ' ' << "\"\\n\"," << endl;
	file << ' ' << "nullptr" << endl;
	file << "};" << endl << endl;
}

void write_shader_text(ofstream& file, const vector<string>& v, const vector<string>& f)
{	
	static int ind = 0;

	_write_shader_text(file, v, 'v', ind);
	_write_shader_text(file, f, 'f', ind);

	ind++;
}

vector<string> get_vector(string in)
{
	vector<string> res;
	string line;
	ifstream shd(string(DIR) + in);
	
	while (getline(shd, line))
	{
		line.append("\\n");
		res.push_back(line);
	}

	shd.close();
	return res;
}
#define SH vector<string>, vector<string>, string, bool, bool, set<string>

void write_shader_fields(ofstream& file, tuple<SH> shdr)
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
	file << (get<4>(shdr) ? "\ttrue," : "\tfalse,") << endl;
	file << "\t{";
	for (auto it = get<5>(shdr).begin(); it != get<5>(shdr).end(); it++)
	{
		file << "\"" <<*it << "\"";
		if (it != (--get<5>(shdr).end())) file << ", ";
	}
	file << "}" << endl;
	file << "}," << endl;

	ind++;
}

int main()
{		
	vector<tuple<SH>> shdrs =
	{ {
		tuple<SH>{ get_vector("camera_p_vert.shader"),		get_vector("camera_p_frag.shader"),		"CGAP_POS",			false,	false, {"MVP"}},
		tuple<SH>{ get_vector("camera_pn_vert.shader"),		get_vector("camera_pn_frag.shader"),	"CGAP_POS_NORM",	false,	false, {"MVP", "NM", "nL"}},
		tuple<SH>{ get_vector("camera_pnt_vert.shader"),	get_vector("camera_pnt_frag.shader"),	"CGAP_POS_NORM_TEX",false,	false, {"MVP", "NM", "nL", "texture0"}},
		tuple<SH>{ get_vector("camera_pt_vert.shader"),		get_vector("camera_pt_frag.shader"),	"CGAP_POS_TEX",		false,	false, {"MVP", "texture0"}},
		tuple<SH>{ get_vector("camera_pt_2d_vert.shader"),	get_vector("camera_pt_2d_frag.shader"),	"CGAP_POS_TEX",		true,	false, {"screenWidth", "screenHeight", "texture0"}}
	} };
	
	const int size = shdrs.size();
	shdrs.resize(size * 2);
	transform(shdrs.begin(), shdrs.begin() + size, shdrs.begin() + size, 
		[](const tuple<SH>& s)
		{
			tuple<SH> copy(s);

			get<4>(copy) = true;

			vector<string>& vert_vec = get<1>(copy);
			vert_vec.insert((vert_vec.begin() + 1), "#define ALPHA_TEST 1\\n");
		
			return copy;
		});
	
	ofstream out_cpp(OUT_CPP);
	for each (const string& l in head)
		out_cpp << l << endl;
	out_cpp << endl;
	for each (const auto& v in shdrs)
		write_shader_text(out_cpp, get<0>(v), get<1>(v));

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
