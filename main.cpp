#include <algorithm>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <cmath>
#include <map>

#define RED     "\033[31m"      /* Red */

using namespace std;

const int MXLEN = 100000;

//Type: '-', '+', '.', 'd', 'e', 'f', '*', '/', '(', ')'

bool IsOperand(char c)
{
	if(c=='-' || c=='+' || c=='*' || c=='/') return true;
	else return false;
}

int OprPriority(char c)
{
	if(c=='-' || c=='+') return 1;
	else if(c=='*' || c=='/') return 2;
	else return 0;
}

char GetType(char c)
{
	if(c == '-') return '-';
	else if(c == '+') return '+';
	else if(c == '.') return '.';
	else if(isdigit(c)) return 'd';
	else if(c=='E' || c=='e') return 'e';
	else if(c == EOF) return 'f';
	else if(c == '*') return '*';
	else if(c == '/') return '/';
	else if(c == '(') return '(';
	else if(c == ')') return ')';
	else throw make_pair(string("Illegal Character "), c);
}

struct FormatAutomatonNode
{
	map<char, int> trans;

	int get_trans(char c)
	{
		if(!trans.count(c))
		{
			throw make_pair(string("Unsuccessful Trans To "), c);
			return -1;
		}
		else return trans[c];
	}
};

struct FormatAutomaton	//输入科学计数法用的格式自动机
{
	FormatAutomatonNode Node[10];
	int cur;
	double val, mul;
	double val_sgn, mul_sgn;
	double decimal;


	void Init()
	{
		for(auto &tmp : Node) tmp.trans.clear();
		cur = 0, val = 0, mul = 0;
		val_sgn = mul_sgn = 1;
		decimal = 1;

		Node[0].trans['-'] = 1;
		Node[0].trans['+'] = 1;
		Node[0].trans['d'] = 2;
		Node[0].trans['.'] = 3;

		Node[1].trans['d'] = 2;
		Node[1].trans['.'] = 3;

		Node[2].trans['d'] = 2;
		Node[2].trans['.'] = 3;
		Node[2].trans['e'] = 5;
		Node[2].trans['f'] = 8;
		Node[2].trans['+'] = 8;
		Node[2].trans['-'] = 8;
		Node[2].trans['*'] = 8;
		Node[2].trans['/'] = 8;
		Node[2].trans['('] = 8;
		Node[2].trans[')'] = 8;
		
		Node[3].trans['d'] = 4;

		Node[4].trans['d'] = 4;
		Node[4].trans['e'] = 5;
		Node[4].trans['f'] = 8;
		Node[4].trans['+'] = 8;
		Node[4].trans['-'] = 8;
		Node[4].trans['*'] = 8;
		Node[4].trans['/'] = 8;
		Node[4].trans['('] = 8;
		Node[4].trans[')'] = 8;

		Node[5].trans['-'] = 6;
		Node[5].trans['+'] = 6;
		Node[5].trans['d'] = 7;

		Node[6].trans['d'] = 7;
		
		Node[7].trans['d'] = 7;
		Node[7].trans['f'] = 8;
		Node[7].trans['+'] = 8;
		Node[7].trans['-'] = 8;
		Node[7].trans['*'] = 8;
		Node[7].trans['/'] = 8;
		Node[7].trans['('] = 8;
		Node[7].trans[')'] = 8;
	}

	void NextChar(char c)
	{
		try
		{
			cur = Node[cur].get_trans(GetType(c));
		}
		catch(pair<string, char> e)
		{
			std::cerr << "In FormatAutomaton.NextChar() : " << e.first << e.second << '\n';
			throw e;
		}
		if(cur == 1)
		{
			if(c == '-') val_sgn = -1;
			else if(c == '+') val_sgn = +1;
		}
		else if(cur == 2)
		{
			val = val*10 + (c-'0');
		}
		else if(cur == 4)
		{
			decimal = decimal / 10.0;
			val = val + decimal * (c-'0');
		}
		else if(cur == 6)
		{
			if(c == '-') mul_sgn = -1;
			else if(c == '+') mul_sgn = +1;
		}
		else if(cur == 7)
		{
			mul = mul*10 + (c-'0');
		}
	}
};

char InputSeq[MXLEN+5];
int InputLen, CurPos;

char GetNextC()
{
	char c = getchar();
	while(c == ' ') c = getchar();
	return c;
}

double GetNextNumber()
{
	double ret = 0;
	FormatAutomaton Num;
	Num.Init();
	while(CurPos <= InputLen)
	{
		char c = InputSeq[CurPos];
		try
		{
			Num.NextChar(c);
		}
		catch(pair<string, char> e)
		{
			std::cerr << "In GetNextNumber() : Pos " << CurPos << " Generates an Illegal Number" << '\n';
			throw make_pair(string("Illegal Number "), CurPos);
		}
		if(Num.cur == 8) break;
		else CurPos++;
	}
	return Num.val_sgn * Num.val * pow(10, Num.mul_sgn * Num.mul);
}

void Input()
{
	char c;
	try
	{
		while(1)
		{
			c = GetNextC();
			if(c == EOF || c == '\n' || c == '\r') break;
			InputSeq[++InputLen] = c;
			if(InputLen > MXLEN) throw string("Expression Length Exceeded ");
		}
		InputSeq[++InputLen] = EOF;
		CurPos = 1;
	}
	catch(string s)
	{
		std::cerr << RED << s << '\n';
	}
}

void Calc()
{
	vector<double> ValStack;
	vector<char> OprStack;
	char last_type = '*';

	auto CalculateTop = [&]()
	{
		if(ValStack.size() < 2) throw make_pair(string("Operand With Only One or Less Parameter (Test) "), CurPos);	//If the lines above are all correct, this line would never happen
		double a = ValStack.back();
		ValStack.pop_back();
		double b = ValStack.back();
		ValStack.pop_back();
		char c = OprStack.back();
		OprStack.pop_back();
		if(c == '+') ValStack.push_back(b+a);
		else if(c == '-') ValStack.push_back(b-a);
		else if(c == '*') ValStack.push_back(b*a);
		else if(a != 0) ValStack.push_back(b/a);
		else throw make_pair(string("Division by 0 "), CurPos);
	};

	try
	{
		while(CurPos <= InputLen)
		{
			char c = InputSeq[CurPos];
			if(last_type == '(')
			{
				if(c=='*' || c=='/') throw make_pair(string("Illegal Combination of Operands "), CurPos);
				else if(c == '(') last_type = '(', CurPos++;
				else if(c == ')') throw make_pair(string("Brackets Enclosing Nothing "), CurPos);
				else if(c == EOF) throw make_pair(string("Unpaired Left Bracket "), CurPos);
				else ValStack.push_back(GetNextNumber()), last_type = 'd';
			}
			else if(last_type == ')')
			{
				if(IsOperand(c)) last_type = c, CurPos++;
				else if(c == ')') last_type = ')', CurPos++;
				else if(c == '(') throw make_pair(string("Adjacent Brackets "), CurPos);
				else if(c == EOF) last_type = 'f', CurPos++;
				else throw make_pair(string("Brackets Followed by a Single Number "), CurPos);
			}
			else if(last_type == 'd')
			{
				if(IsOperand(c)) last_type = c, CurPos++;
				else if(c == '(') throw make_pair(string("Number Followed by a Left Bracket "), CurPos);
				else if(c == ')') last_type = ')', CurPos++;
				else if(c == EOF) last_type = 'f', CurPos++;
				else throw make_pair(string("Adjacent Numbers "), CurPos);
			}
			else if(IsOperand(last_type))
			{
				if(c == '(') last_type = '(', CurPos++;
				else if(c == ')') throw make_pair(string("Operand With Only One or Less Parameter "), CurPos);
				else if(c == EOF) throw make_pair(string("Operand With Only One or Less Parameter "), CurPos);
				else ValStack.push_back(GetNextNumber()), last_type = 'd';
			}

			if(IsOperand(last_type))
			{
				while(OprStack.size() && OprPriority(OprStack.back())>=OprPriority(c)) CalculateTop();
				OprStack.push_back(c);
			}
			else if(last_type == ')')
			{
				while(OprStack.size() && OprStack.back()!='(') CalculateTop();
				if(!OprStack.size()) throw make_pair(string("Unpaired Right Bracket "), CurPos);
				else OprStack.pop_back();
			}
			else if(last_type == '(') OprStack.push_back('(');
		}

		while(OprStack.size() && IsOperand(OprStack.back())) CalculateTop();
		if(OprStack.size()) throw make_pair(string("Unpaired Left Bracket "), CurPos);
		else if(ValStack.size() != 1) throw make_pair(string("Remaining More Numbers than Expected "), CurPos);
		else printf("%.2lf\n", ValStack.back());
	}
	catch(pair<string, int> e)
	{
		std::cerr << RED << "In Calc() : " << e.first << ", Position : " << e.second << '\n';
		exit(0);
	}
}

int main()
{
	Input();
	Calc();
	return 0;
}