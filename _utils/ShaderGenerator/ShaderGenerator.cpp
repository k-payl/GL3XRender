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
#include "Preprocessor.h"
using namespace std;

#define DIR "..\\..\\src\\shaders\\"
#define OUT_CPP "../../src/shaderSources.cpp"
#define SHADER_VERT_NAME "mesh_vertex.shader"
#define SHADER_FRAG_NAME "mesh_fragment.shader"

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

template<typename T>
void _write_shader_text(ofstream& file, T lines_vec, char var, int ind, bool line_ending = false)
{
	file << "static const char *" << var << ind << "[] = ";
	file << "{" << endl;
	for each (const string& s in lines_vec)
	{
		file << ' ' << '\"' << s << (line_ending ? "\\n\"," : "\",") << endl;
	}
	file << ' ' << "\"\\n\"," << endl;
	file << ' ' << "nullptr" << endl;
	file << "};" << endl << endl;
}

void write_shader_text(ofstream& file, const vector<string>& v, const vector<string>& f)
{	
	static int ind = 0;

	_write_shader_text<vector<string>>(file, v, 'v', ind);
	_write_shader_text<vector<string>>(file, f, 'f', ind);

	ind++;
}

vector<string> get_vector(string in, bool line_ending)
{
	vector<string> res;
	string line;
	ifstream shd(string(DIR) + in);
	
	while (getline(shd, line))
	{
		if (line_ending)
			line.append("\\n");
		res.push_back(line);
	}

	shd.close();
	return res;
}
#define SH string, bool, bool

void write_shader_fields(ofstream& file, tuple<SH>& shdr, int ind)
{
	file << "{" << endl;
	file << "\t\"Shader" << ind << "\"," << endl;
	file << "\texact_ptrptr(v" << ind << ")," << endl;
	file << "\texact_ptrptr(f" << ind << ")," << endl;
	file << "\t_countof(v" << ind << ") - 1," << endl;
	file << "\t_countof(f" << ind << ") - 1," << endl;
	file << '\t' << get<0>(shdr) << ',' << endl;
	file << (get<1>(shdr) ? "\ttrue," : "\tfalse,") << endl;
	file << (get<2>(shdr) ? "\ttrue," : "\tfalse,") << endl;
	file << "}," << endl;

}

void generate_recursively(ofstream& file, Preprocessor& processor, vector<string>& frag, vector<string>& vert, vector<string>& defs, int i)
{
	static int j = 0;

	if (i >= defs.size())
	{
		auto shader_text_list_v = processor.run(vert);
		_write_shader_text<list<string>>(file, shader_text_list_v, 'v', j, true);

		auto shader_text_list_f = processor.run(frag);
		_write_shader_text<list<string>>(file, shader_text_list_f, 'f', j, true);

		j++;

		return;
	}

	generate_recursively(file, processor, vert, frag, defs, i + 1);
	processor.set_define(defs[i]);

	generate_recursively(file, processor, vert, frag, defs, i + 1);
	processor.erase_define(defs[i]);
}

void generate_structs_recursively(ofstream& file, Preprocessor& processor, vector<string>& defs, int i)
{
	static int j = 0;

	if (i >= defs.size())
	{

		const bool is2d = processor.define_exist("ENG_INPUT_2D");
		const bool alphaTest = processor.define_exist("ENG_ALPHA_TEST");
		string attrs = "POS";
		if (processor.define_exist("ENG_INPUT_NORMAL")) attrs += " | NORM";
		if (processor.define_exist("ENG_INPUT_TEXCOORD")) attrs += " | TEX_COORD";
		
		tuple<SH> t = (std::make_tuple(attrs, is2d, alphaTest));
		write_shader_fields(file, t, j);

		j++;

		return;
	}

	generate_structs_recursively(file, processor, defs, i + 1);
	processor.set_define(defs[i]);

	generate_structs_recursively(file, processor, defs, i + 1);
	processor.erase_define(defs[i]);
}

int main()
{	
	ofstream out_cpp(OUT_CPP);

	for each (const string& l in head)
		out_cpp << l << endl;
	out_cpp << endl;


	vector<string> defs = { "ENG_INPUT_2D", "ENG_INPUT_NORMAL", "ENG_INPUT_TEXCOORD", "ENG_ALPHA_TEST"};
	auto shader_text_vec_frag = get_vector(SHADER_FRAG_NAME, false);
	auto shader_text_vec_vert = get_vector(SHADER_VERT_NAME, false);

	Preprocessor processor;
	
	generate_recursively(out_cpp, processor, shader_text_vec_frag, shader_text_vec_vert, defs, 0);


	out_cpp << "static std::vector<ShaderSrc> _shadersGenerated =" << endl;
	out_cpp << "{{" << endl;

	generate_structs_recursively(out_cpp, processor, defs, 0);

	out_cpp << "}};" << endl;

	out_cpp << "const std::vector<ShaderSrc>& getShaderSources()" << endl;
	out_cpp << "{" << endl;
	out_cpp << "	return _shadersGenerated;" << endl;
	out_cpp << "}" << endl;
	out_cpp.close();

}
