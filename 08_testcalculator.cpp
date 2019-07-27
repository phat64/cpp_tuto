#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <assert.h>

using namespace std;

enum TokenType
{
	NONE, INTEGER, OPERATOR, PARENTHESIS
};

struct Token
{
	TokenType type;

	Token()
	{
		type = NONE;
		strvalue[0] = '\0';
		cvalue = '?';
		ivalue = 0;
	}

	Token(char c)
	{
		if (c == '(' || c == ')')
		{
			type = PARENTHESIS;
			cvalue = c;
		}
		else if (c == '+' || c == '-' || c == '*' || c == '/')
		{
			type = OPERATOR;
			cvalue = 'O';
		}
		else
		{
			type = NONE;
			cvalue = '?';
		}
		strvalue[0] = c;
		strvalue[1] = '\0';
		ivalue = 0;
	}

	char strvalue[16];
	char cvalue;
	int ivalue;
};


bool GetParenthesedExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx);


void Tokenize(vector<Token> & tokens, const string & str)
{
	char * s = (char*)str.c_str();
	char c;

	while (c = *s++)
	{
		while (c && (c == ' ' || c == '\t'))
		{
			c = *s++;
		}

		if (c == '\0')
		{
			break;
		}
		else if (isdigit(c))
		{
			char * start;
			char * end;
			start = end = s-1;

			while(isdigit(c))
			{
				c = *++end;
			}

			Token t;
			t.type = INTEGER;
			t.cvalue = 'I';
			s = end;
			strncpy(t.strvalue, start, end - start);
			t.strvalue[end - start] = '\0';
			t.ivalue = atoi(t.strvalue);
			tokens.push_back(t);
		}
		else if (c == '+' || c == '-' || c == '*' || c == '/')
		{
			Token t;
			t.type = OPERATOR;
			t.cvalue = t.ivalue = 'O';
			t.strvalue[0] = c;
			t.strvalue[1] = '\0';
			tokens.push_back(t);
		}
		else if (c == '(' || c ==')')
		{
			Token t;
			t.type = PARENTHESIS;
			t.cvalue = t.ivalue = c;
			t.strvalue[0] = c;
			t.strvalue[1] = '\0';
			tokens.push_back(t);
		}
		else
		{
			cout << "UNKNOW CHARACTER" << c << endl;
			exit(-1);
		}
	}
}

bool Check1(const vector<Token> & tokens, int idx)
{
	return tokens[idx].type == INTEGER;
}

bool Check2(const vector<Token> & tokens, int idx)
{
	return false;
}

bool Check3(const vector<Token> & tokens, int idx)
{
	const char * validcombo[] = {"(I)", "IOI", NULL};

	for (int i = 0; validcombo[i]; i++)
	{
		if (tokens[idx + 0].cvalue == validcombo[i][0] && tokens[idx + 1].cvalue == validcombo[i][1] && tokens[idx + 2].cvalue == validcombo[i][2])
		{
			return true;
		}
	}

	return false;
}

bool CheckCombo(const vector<Token> & tokens, int idx0, int idx1)
{
	const char * validcombo[] = {"((", "))", "(I", "I)", "IO", "OI", "O(", ")O", NULL};

	for (int i = 0; validcombo[i]; i++)
	{
		if (tokens[idx0].cvalue == validcombo[i][0] && tokens[idx1].cvalue == validcombo[i][1])
		{
			return true;
		}
	}

	return false;
}

bool CheckParenthesis(const vector<Token> & tokens, int first, int last)
{
	int depth = 0;

	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].cvalue == '(')
		{
			depth++;
		}

		if (tokens[i].cvalue == ')')
		{
			depth--;
			if (depth < 0)
				return false;
		}
	}

	return depth == 0;
}

bool Check(const vector<Token> & tokens, int first, int last)
{
	int size = last - first;
	if (size == 1)
	{
		return Check1(tokens, first);
	}
	else if (size == 2)
	{
		return Check2(tokens, first);
	}
	else if (size == 3)
	{
		return Check3(tokens, first);
	}

	if (!CheckParenthesis(tokens, first, last))
	{
		return false;
	}

	for (int i = 0; i < (int)tokens.size() - 1; i++)
	{
		if (!CheckCombo(tokens, i, i + 1))
		{
			return false;
		}
	}

	return true;
}

bool Compute2(int & result, int val, vector<Token> & tokens, int first)
{
	//if (Check3(tokens, first))
	{
		if (tokens[first].type == OPERATOR)
		{
			int a = val;
			int b = tokens[first+1].ivalue;
			switch(tokens[first].strvalue[0])
			{
				case '+': result = a + b; break;
				case '-': result = a - b; break;
				case '*': result = a * b; break;
				case '/': result = a / b; break;
			}
			return true;
		}
	}
	return false;
}


bool Compute3(int & result, const vector<Token> & tokens, int first)
{
	if (Check3(tokens, first))
	{
		// IOI
		if (tokens[first + 1].type == OPERATOR)
		{
			int a = tokens[first].ivalue;
			int b = tokens[first+2].ivalue;
			switch(tokens[first + 1].strvalue[0])
			{
				case '+': result = a + b; break;
				case '-': result = a - b; break;
				case '*': result = a * b; break;
				case '/': result = a / b; break;
			}
			return true;
		}
		// (I)
		if (tokens[first + 1].type == INTEGER)
		{
			result = tokens[first+1].ivalue;
			return true;
		}
	}
	return false;
}


int FindChar(const vector<Token> & tokens, int first, int last, char c, int dir)
{
	if (dir > 0)
	{
		for (int idx = first; idx < last; idx++)
		{
			if (tokens[idx].strvalue[0] == c)
				return idx;
		}
	}
	else if (dir < 0)
	{
		for (int idx = last-1; idx >= first; idx--)
		{
			if (tokens[idx].strvalue[0] == c)
				return idx;
		}
	}

	return -1;
}

int EvaluateWithoutParenthesis(const vector<Token> & tokens, int first, int last)
{
	int result = tokens[first].ivalue;

	for (int i = first + 1; i < last; i+= 2)
	{
		int nextValue = tokens[i+1].ivalue;
		switch(tokens[i].strvalue[0])
		{
			case '+': result += nextValue; break;
			case '-': result -= nextValue; break;
			case '*': result *= nextValue; break;
			case '/': result /= nextValue; break;
		}
	}

	return result;
}

int Evaluate(const vector<Token> & tokens, int first, int last)
{
	for (int i = first; i < last; i++)
	{
		cout << tokens[i].strvalue;
	}
	cout << endl;
	int size = last - first;

	int result = 0;

	if (size == 1)
	{
		return tokens[first].ivalue;
	}
	else if (size == 2)
	{
		return 0;
	}
	else if (size == 3)
	{
		Compute3(result, tokens, first);
		return result;
	}

	// no parenthesis => easy to evaluate
	if (FindChar(tokens, first, last, '(', 1) < 0)
	{
		result = EvaluateWithoutParenthesis(tokens, first, last);
		return result;
	}

	// evaluate the first expression
	if (tokens[first].type == PARENTHESIS && tokens[first].strvalue[0] == '(')
	{
		int expressionFirstIdx;
		int expressionLastIdx;
		if (GetParenthesedExpression(tokens, first, last, first, expressionFirstIdx, expressionLastIdx))
		{
			result = Evaluate(tokens, expressionFirstIdx + 1, expressionLastIdx - 1);
			first = expressionLastIdx;
		}
	}
	else if (tokens[first].type == INTEGER)
	{
		result = tokens[first].ivalue;
		first++;
	}

	if (first != last)
	{
		int nextValue = Evaluate(tokens, first + 1, last);
		switch(tokens[first].strvalue[0])
		{
			case '+': result += nextValue; break;
			case '-': result -= nextValue; break;
			case '*': result *= nextValue; break;
			case '/': result /= nextValue; break;
		}
	}
	return result;
}

bool GetParenthesedExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx)
{
	if (idx < first)
	{
		return false;
	}

	if (idx >= last)
	{
		return false;
	}

	if (tokens[idx].type == PARENTHESIS)
	{
		int depth = 0;
		if (tokens[idx].strvalue[0] == '(')
		{
			firstIdx = idx;
			for(;idx < last;idx++)
			{
				if (tokens[idx].type == PARENTHESIS)
				{
					if (tokens[idx].strvalue[0] == '(')
					{
						depth++;
					}
					else if (tokens[idx].strvalue[0] == ')')
					{
						depth--;
						if (depth == 0)
						{
							lastIdx = idx + 1;
							return true; // found
						}
					}
				}
			}
		}
		else if (tokens[idx].strvalue[0] == ')')
		{
			lastIdx = idx + 1;
			for(;idx >= first;idx--)
			{
				if (tokens[idx].type == PARENTHESIS)
				{
					if (tokens[idx].strvalue[0] == '(')
					{
						depth++;
						if (depth == 0)
						{
							firstIdx = idx;
							return true; // found
						}

					}
					else if (tokens[idx].strvalue[0] == ')')
					{
						depth--;
					}
				}
			}
		}
	}

	return false; // not found
}

void Priorize(vector<Token> & tokens, int first, int last)
{
	int mulIdx = FindChar(tokens, first, last, '*', 1);
	int divIdx = FindChar(tokens, first, last, '/', 1);
	int mulDivIdx = (mulIdx != -1 && divIdx != -1 ? std::min(mulIdx, divIdx): (mulIdx != -1? mulIdx : divIdx));  
	while (mulDivIdx != -1)
	{
		if (tokens[mulDivIdx - 1].type == INTEGER && tokens[mulDivIdx + 1].type == INTEGER)
		{
			tokens.insert(tokens.begin() + mulDivIdx - 1, Token('('));
			tokens.insert(tokens.begin() + mulDivIdx + 3, Token(')'));
			last += 2;
			mulDivIdx += 2;
		}
		else if (tokens[mulDivIdx - 1].type == INTEGER && tokens[mulDivIdx + 1].type == PARENTHESIS)
		{
			int expressionFirstIdx;
			int expressionLastIdx;

			if (GetParenthesedExpression(tokens, first, last, mulDivIdx + 1, expressionFirstIdx, expressionLastIdx))
			{
				tokens.insert(tokens.begin() + mulDivIdx - 1, Token('('));
				tokens.insert(tokens.begin() + expressionLastIdx + 1, Token(')'));
				last += 2;
				mulDivIdx += 2;
			}
		}
		else if (tokens[mulDivIdx - 1].type == PARENTHESIS && tokens[mulDivIdx + 1].type == INTEGER)
		{
			int expressionFirstIdx;
			int expressionLastIdx;

			if (GetParenthesedExpression(tokens, first, last, mulDivIdx - 1, expressionFirstIdx, expressionLastIdx))
			{
				tokens.insert(tokens.begin() + expressionFirstIdx, Token('('));
				tokens.insert(tokens.begin() + mulDivIdx + 3, Token(')'));
				last += 2;
				mulDivIdx += 2;
			}
		}

		

		mulIdx = FindChar(tokens, mulDivIdx, last, '*', 1);
		divIdx = FindChar(tokens, mulDivIdx, last, '/', 1);
		mulDivIdx = (mulIdx != -1 && divIdx != -1 ? std::min(mulIdx, divIdx): (mulIdx != -1? mulIdx : divIdx));  

		//mulIdx = FindChar(tokens, mulIdx, last, '*', 1);

		cout << "mulIdx = " << mulIdx << endl;
		cout <<endl;
		for (size_t i = 0; i < tokens.size(); i++)
		{
			cout << tokens[i].strvalue;
		}
		cout <<endl;
	}

	return;
	divIdx = FindChar(tokens, first, last, '/', 1);
	while (divIdx != -1)
	{
		if (tokens[divIdx - 1].type == INTEGER && tokens[divIdx + 1].type == INTEGER)
		{
			tokens.insert(tokens.begin() + divIdx - 1, Token('('));
			tokens.insert(tokens.begin() + divIdx + 3, Token(')'));
			last += 2;
		}
		divIdx += 2;
		divIdx = FindChar(tokens, divIdx, last, '/', 1);

		cout << "divIdx = " << divIdx << endl;
		for (size_t i = 0; i < tokens.size(); i++)
		{
			cout << tokens[i].strvalue <<endl;
		}
	}

	for (size_t i = 0; i < tokens.size(); i++)
	{
		cout << tokens[i].strvalue;
	}
	cout <<endl;
}

int Evaluate(const string & str)
{
	vector<Token> tokens;
	Tokenize(tokens, str);
	if (Check(tokens, 0, tokens.size()))
	{
		Priorize(tokens, 0, tokens.size());
		return Evaluate(tokens, 0, tokens.size());
	}
	cout << "error : " << str << endl;
	return 0;
}


int main(int argc, char ** argv)
{
	assert(36 + 50 * 100 / 2 * 3 == Evaluate("36 + 50 * 100 / 2 * 3"));

	assert(99 * 56 + 25 * 37 / 3 * 5 == Evaluate("99 * 56 + 25 * 37 / 3 * 5"));


	/*assert(42 == Evaluate("42"));
	assert(99 + 42 == Evaluate("99 + 42"));
	assert(99 * 42 == Evaluate("99 * 42"));
	assert(11 + 22 * 33 == Evaluate("11 + 22 * 33"));
	assert(42 == Evaluate("42"));
	assert(42 == Evaluate("42"));
	*/
	while (true)
	{
		string in;
		vector<Token> tokens;
	
		cout << "expr ?" << endl;
		cin >> in; 

		int result = Evaluate(in);
		cout << "result = "<<  result <<endl;
	}

	return 0;
}
