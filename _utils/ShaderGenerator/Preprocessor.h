#pragma once
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>
#include <algorithm>

using std::string;
using std::list;
using std::vector;

enum DIRECTIVE
{
	UNKNOWN,
	IFDEF,
	ELSE,
	ELSEIF,
	ENDIF
};

class Preprocessor
{
	list<string> defines;


public:

	void set_define(const string& def);
	void erase_define(const string& def);
	bool define_exist(const string& def);

	DIRECTIVE get_directive(string::iterator it, string::iterator str_end);
	string get_next_str(string::iterator& it, string::iterator str_end);
	bool evaluate_def_value(string::iterator& it, string::iterator str_end);
	void move_it_to_end_dirictive(string::iterator& it, string::iterator str_end);
	list<string> run(const vector<string>& text);
};

