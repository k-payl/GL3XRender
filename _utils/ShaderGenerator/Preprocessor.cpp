#include "Preprocessor.h"
#include <iterator>
#include <assert.h>


void Preprocessor::set_define(const string& def)
{
	defines.push_back(def);
}

void Preprocessor::erase_define(const string& def)
{
	defines.remove(def);
}

bool Preprocessor::define_exist(const string& def)
{
	return find(defines.begin(), defines.end(), def) != defines.end();
}

DIRECTIVE Preprocessor::get_directive(string::iterator str_it, string::iterator str_end)
{
	string str = string(str_it, str_end);

	if (str.compare(0, 5, "ifdef") == 0)
		return IFDEF;
	else if (str.compare(0, 4, "else") == 0)
		return ELSE;
	else if (str.compare(0, 4, "elif") == 0)
		return ELSEIF;
	else if (str.compare(0, 5, "endif") == 0)
		return ENDIF;

	return UNKNOWN;
}


string Preprocessor::get_next_str(string::iterator& it, string::iterator str_end)
{
	while (it != str_end && *it == ' ') it++; //skip whitespace
	if (it == str_end) return string();

	if (*it == '&' && *(it + 1) == '&')
	{
		it+=2;
		return string("&&");
	}

	if (*it == '|' && *(it + 1) == '|')
	{
		it+=2;
		return string("||");
	}

	if (*it == '!' || isalpha(*it) || *it == '_')
	{
		string::iterator def_begin = it;
		it++;
		while (it != str_end && (isalnum(*it) || *it == '_')) it++;
		return string(def_begin, it);
	}

	assert(false); // invalid string
}

bool Preprocessor::evaluate_def_value(string::iterator& it, string::iterator str_end)
{
	bool l_operand = false;	
	bool prev_was_operation = false;
	bool op_is_and = false;

	while(true)
	{
		string str = get_next_str(it, str_end);
		if (str.empty()) break;

		if (str == "&&")
		{
			prev_was_operation = true;
			op_is_and = true;
		}
		else if (str == "||")
		{
			prev_was_operation = true;
			op_is_and = false;
		}
		else
		{
			bool value;

			if (str[0] == '!')
			{
				string def = string(str.begin() + 1, str.end());
				value = !define_exist(def);
			}
			else
				value = define_exist(str);


			if (prev_was_operation)
			{
				if (op_is_and) 
					l_operand = l_operand && value;
				else 
					l_operand = l_operand || value;
				prev_was_operation = false;
			}
			else
			{
				l_operand = value;
			}
		}			
	}

	return l_operand;
}

void Preprocessor::move_it_to_end_dirictive(string::iterator& it, string::iterator str_end)
{
	while (it != str_end && *it != ' ') it++;
}

list<string> Preprocessor::run(const vector<string>& text)
{
	list<string> ret(text.begin(), text.end());

	struct TxtBlock
	{
		list<string>::iterator it_start;
		string::iterator str_it_start;
		list<string>::iterator it_end;
		string::iterator str_it_end;
	};

	bool fisrt_block_fifished = false;
	bool second_block_started = false;
	TxtBlock block_tmp;
	vector<TxtBlock> text_to_remove;


	for (auto it = ret.begin(); it != ret.end(); it++)
	{
		for (auto str_it = it->begin(); str_it != it->end(); )
		{
			DIRECTIVE directive = UNKNOWN;

			if (*str_it == '#')
			{
				directive = get_directive(str_it + 1, it->end());
			}

			switch (directive)
			{
				case UNKNOWN: str_it++; break;
				case IFDEF:

					block_tmp.it_start = it;
					block_tmp.str_it_start = str_it;

					move_it_to_end_dirictive(str_it, it->end());
										
					if (evaluate_def_value(str_it, it->end()))
					{
						block_tmp.it_end = it;
						block_tmp.str_it_end = str_it;
						fisrt_block_fifished = true;
						text_to_remove.push_back(block_tmp);
					}
					break;
				case ELSE: 
					if (fisrt_block_fifished && !second_block_started)
					{
						block_tmp.it_start = it;
						block_tmp.str_it_start = str_it;
						second_block_started = true;
						move_it_to_end_dirictive(str_it, it->end());
					}
					else if (fisrt_block_fifished && second_block_started)
					{
						move_it_to_end_dirictive(str_it, it->end());
					}
					else
					{
						move_it_to_end_dirictive(str_it, it->end());

						block_tmp.it_end = it;
						block_tmp.str_it_end = str_it;
						text_to_remove.push_back(block_tmp);
						fisrt_block_fifished = true;
					}
					
					break;
				case ELSEIF:
					if (fisrt_block_fifished && !second_block_started)
					{
						block_tmp.it_start = it;
						block_tmp.str_it_start = str_it;
						second_block_started = true;
						move_it_to_end_dirictive(str_it, it->end());
						evaluate_def_value(str_it, it->end());
					}
					else if (fisrt_block_fifished && second_block_started)
					{
						move_it_to_end_dirictive(str_it, it->end());
						evaluate_def_value(str_it, it->end());
					}
					else // fisrt_block_fifished = false
					{
						move_it_to_end_dirictive(str_it, it->end());

						if (evaluate_def_value(str_it, it->end()))
						{
							block_tmp.it_end = it;
							block_tmp.str_it_end = str_it;
							fisrt_block_fifished = true;
							text_to_remove.push_back(block_tmp);
						}
					}
					break;
				case ENDIF: 
					if (fisrt_block_fifished && !second_block_started)
					{
						block_tmp.it_start = it;
						block_tmp.str_it_start = str_it;
					}

					move_it_to_end_dirictive(str_it, it->end());
					
					block_tmp.it_end = it;
					block_tmp.str_it_end = str_it;

					text_to_remove.push_back(block_tmp);

					fisrt_block_fifished = false;
					second_block_started = false;

					break;
				default: str_it++; break;
			}
		}
	}

	// remove text blocks
	for (auto& txt_block : text_to_remove)
	{
		if (txt_block.it_start == txt_block.it_end)
		{
			if (txt_block.str_it_start == txt_block.it_start->begin() &&
				txt_block.str_it_end == txt_block.it_end->end())
				ret.erase(txt_block.it_start);
			else
				txt_block.it_start->erase(txt_block.str_it_start, txt_block.str_it_end);
			
		}
		else
		{
			int dist = distance(txt_block.it_start, txt_block.it_end);
			auto it1 = txt_block.it_start; ++it1;
			auto it2 = txt_block.it_end;

			if (txt_block.str_it_start == txt_block.it_start->begin())
				ret.erase(txt_block.it_start);
			else
				txt_block.it_start->erase(txt_block.str_it_start, txt_block.it_start->end());

			if (dist > 1)
				ret.erase(it1, it2);

			if (txt_block.str_it_end == txt_block.it_end->end())
				ret.erase(txt_block.it_end);
			else
				txt_block.it_end->erase(txt_block.it_end->begin(), txt_block.str_it_end);
		}
	}


	// remove empty lines
	vector<list<string>::iterator> empty_lines;	
	for (auto it = ret.begin(); it != ret.end(); ++it)
		if (*it == "" | *it == "\t" || *it == "\n") 
			empty_lines.push_back(it);
	
	for (auto& line : empty_lines)
		ret.erase(line);


	return ret;
}