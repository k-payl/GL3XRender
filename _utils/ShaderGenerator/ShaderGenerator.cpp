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
using namespace std;

#define SHADER_DIRECTORY "..\\..\\src\\shaders\\"

/*

Example:

const char *s[] =
{
	"#version 330\n",
 	"\n",
 	"smooth in vec3 N;\n"
};

*/

void write_shader(ofstream& file, const string& shader_filename, const string& var_name, const string& comment)
{
	ifstream shd(shader_filename);

	//file << "// Auto generated with ShaderGenerator" << endl;
	file << "static const char *" << var_name << "[] = " ;
	file << "// " << comment << endl;
	file << "{" << endl;

	string line;
	while (getline(shd, line))
	{
		file << ' ' << '\"' << line << "\\n\"," << endl;
	}
	file << ' ' << "\"\\n\"" << endl;

	file << "};" << endl << endl;

	shd.close();
}

int main()
{
	static const string P_shaders[] = { SHADER_DIRECTORY"camera_p_vert.shader", SHADER_DIRECTORY"camera_p_frag.shader" };
	static const string PN_shaders[] = { SHADER_DIRECTORY"camera_pn_vert.shader", SHADER_DIRECTORY"camera_pn_frag.shader" };
	static const string PNT_shaders[] = { SHADER_DIRECTORY"camera_pnt_vert.shader", SHADER_DIRECTORY"camera_pnt_frag.shader" };
	static const string PT_shaders[] = { SHADER_DIRECTORY"camera_pt_vert.shader", SHADER_DIRECTORY"camera_pt_frag.shader" };
	static const string PT2D_shaders[] = { SHADER_DIRECTORY"camera_pt_2d_vert.shader", SHADER_DIRECTORY"camera_pt_2d_frag.shader" };

	ofstream out("out.txt");

	write_shader(out, P_shaders[0], "p_v", "Vertex shader with input: Position");
	write_shader(out, P_shaders[1], "p_f", "Fragment shader with const white color");

	write_shader(out, PN_shaders[0], "pn_v", "Vertex shader with input: Position, Normal");
	write_shader(out, PN_shaders[1], "pn_f", "Fragment shader with interpolated attributes: Normal");

	write_shader(out, PNT_shaders[0], "pnt_v", "Vertex shader with input: Position, Normal, Texture coordiantes");
	write_shader(out, PNT_shaders[1], "pnt_f", "Fragment shader with interpolated attributes: Normal, Texture coordiantes");

	write_shader(out, PT_shaders[0], "pt_v", "Vertex shader with input: Position, Texture coordiantes");
	write_shader(out, PT_shaders[1], "pt_f", "Fragment shader with interpolated attributes: Texture coordiantes");

	write_shader(out, PT2D_shaders[0], "pt2d_v", "Vertex shader with input: Position (vec2), Texture coordiantes");
	write_shader(out, PT2D_shaders[1], "pt2d_f", "Fragment shader with interpolated attributes: Texture coordiantes");

	out.close();

	cout << "all shader text written to " << "out.txt";
	cin.get();
}

