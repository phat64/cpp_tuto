#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <assert.h>

/*
 * A simple calculator with 4 binary operations *, /, +, - and parenthesis ( and ).
 * It works fine.
 * [TODO] => It's only for the tutorial. It's not for advanced developers and it's obviously not high optimized ^^
 *	- It's my old code. It needs more OOP.
 * 	- need the negative operator (unary operator)
 *	- need math functions like sine or cosine
 */


using namespace std;

// ---------------------------- CONSTANTES -----------------------------------

static const size_t NUMBER_DIGITS_MAX = 32;
static const size_t NAME_NB_CHARS_MAX = 32;

// ------------------------------- TOKEN -------------------------------------


enum TokenType
{
	NONE, NUMBER, OPERATOR, PARENTHESIS, COMMA,
	NAME, VARIABLE_NAME, FUNCTION_NAME, /* NAME =  VARIABLE_NAME or FUNCTION_NAME*/
	FUNCTION_ARGS_BEGIN, FUNCTION_ARGS_SEPARATOR, FUNCTION_ARGS_END /* ( , ) */
};

struct Token
{
	TokenType type;

	Token()
	{
		type = NONE;
		strvalue[0] = '\0';
		cvalue = '?';
		dvalue = 0.0;
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
		else if (c == ',')
		{
			type = COMMA;
			cvalue = ',';
		}
		else
		{
			type = NONE;
			cvalue = '?';
		}
		strvalue[0] = c;
		strvalue[1] = '\0';
		dvalue = 0.0;
	}

	Token(const char * str)
	{
		assert(str != NULL);
		strncpy(strvalue, str, sizeof(strvalue) - 1); 	// strncpy is safer than strcpy
		strvalue[sizeof(strvalue) - 1] = '\0';		// mandatory if str is bigger than strvalue

		char c = *str;
		if (c == '(' || c == ')')
		{
			*this = Token(c);
		}
		else if (c == '+' || c == '-' || c == '*' || c == '/')
		{
			*this = Token(c);
		}
		else if (c == ',')
		{
			*this = Token(c);
		}
		else if (isdigit(c))
		{
			type = NUMBER;
			cvalue = 'N';
			dvalue = atof(str);
		}
		else if (isalpha(c))
		{
			type = NAME; // NAME =  VARIABLE_NAME or FUNCTION_NAME
			cvalue = 'N';
			dvalue = 0.0;
		}
		else
		{
			cerr << "unknown token : " << str << endl;
			assert(0);
		}
	}

	char strvalue[NAME_NB_CHARS_MAX + 1];
	char cvalue;
	double dvalue;
};


bool GetParenthesedExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx);


// ------------------------------ TOKENIZER ----------------------------------
void TokenizePostProcess(vector<Token> & tokens);

void Tokenize(vector<Token> & tokens, const string & str)
{
	char * s = (char*)str.c_str();
	char c;

	// Step 1 : simple tokenization
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
			char numberstr[NUMBER_DIGITS_MAX + 1] = {0};
			size_t numberstrlen = 0;
			char * start;
			char * end;
			start = end = s-1;

			while(isdigit(c))
			{
				c = *++end;
			}
			if (c == '.')
			{
				c = *++end;
				while(isdigit(c))
				{
					c = *++end;
				}
			}
			s = end;
			numberstrlen = min(size_t(end - start), NUMBER_DIGITS_MAX); 
			strncpy(numberstr, start, numberstrlen);
			tokens.push_back(Token(numberstr));
		}
		else if (isalpha(c))
		{
			char namestr[NAME_NB_CHARS_MAX + 1] = {0};
			size_t namestrlen = 0;
			char * start;
			char * end;
			start = end = s-1;

			while(isalnum(c))
			{
				c = *++end;
			}
			s = end;
			namestrlen = min(size_t(end - start), NAME_NB_CHARS_MAX);
			strncpy(namestr, start, namestrlen);
			tokens.push_back(Token(namestr));
		}
		else if (c == '+' || c == '-' || c == '*' || c == '/')
		{
			tokens.push_back(Token(c));
		}
		else if (c == '(' || c ==')')
		{
			tokens.push_back(Token(c));
		}
		else if (c == ',')
		{
			tokens.push_back(Token(c));
		}
		else
		{
			cout << "UNKNOW CHARACTER" << c << endl;
			exit(-1);
		}
	}

	TokenizePostProcess(tokens);
}

void TokenizePostProcess(vector<Token> & tokens)
{
	if (tokens.size() > 0)
	{
		// Step 2
		// Post process :
		// convert NAME to VARIABLE_NAME or FUNCTION_NAME


		// Step 2.1
		// check if the next Token is a '(' for a FUNCTION_NAME
		for (size_t i = 0; i < tokens.size() - 1; i++)
		{
			Token & currentToken = tokens[i];
			Token & nextToken = tokens[i + 1];

			if (currentToken.type == NAME)
			{
				if (nextToken.cvalue == '(')
				{
					currentToken.type = FUNCTION_NAME;
					currentToken.cvalue = 'F';
					nextToken.type = FUNCTION_ARGS_BEGIN;
					size_t depth = 1;
					size_t nbSeparators = 0;
					size_t nbLoops = 0;

					// Manage the FUNCTION ARGS LIST
					for (size_t j = i + 1; j < tokens.size() && depth != 0; j++, nbLoops++)
					{
						Token & currentToken2 = tokens[j];

						if (currentToken2.type == PARENTHESIS)
						{
							if (currentToken2.cvalue == '(')
							{
								depth++;
							}
							else if (currentToken2.cvalue == ')')
							{
								depth--;
								if (depth == 0)
								{
									currentToken2.type = FUNCTION_ARGS_END;
								}
							}
						}
						// smart convert COMMA to FUNCTION ARGS SEPARATOR
						else if (currentToken2.type == COMMA && depth == 1)
						{
							currentToken2.type = FUNCTION_ARGS_SEPARATOR;
							nbSeparators++;
						}
					}

					// dvalue contains the nb of args of the function
					if (nbSeparators == 0 && nbLoops <= 2)
					{
						// function()
						currentToken.dvalue = 0;
					}
					else
					{
						// function(1,2,3,...,nbArgs)
						size_t nbArgs = nbSeparators + 1;
						currentToken.dvalue = double(nbArgs);
					}
#if 0
					// debug function
					cout << "fname = " << currentToken.strvalue;
					cout << " nbArgs = "<< currentToken.dvalue;
					cout << " nbLoops = "<< nbLoops << endl;
#endif
				}
				else
				{
					currentToken.type = VARIABLE_NAME;
					currentToken.cvalue = 'N';
				}
			}
		}

		// Step 2.2
		// check the last Token for a VARIABLE_NAME
		Token & lastToken = tokens[tokens.size() - 1];
		if (lastToken.type == NAME)
		{
			lastToken.type = VARIABLE_NAME;
		}
	}
}

// ------------------------------ CHECKER ------------------------------------

bool Check1(const vector<Token> & tokens, int idx)
{
	return tokens[idx].type == NUMBER || tokens[idx].type == VARIABLE_NAME;
}

bool Check2(const vector<Token> & tokens, int idx)
{
	return false;
}

bool Check3(const vector<Token> & tokens, int idx)
{
	const char * validcombo[] = {"(N)", "NON", NULL};

	for (int i = 0; validcombo[i]; i++)
	{
		if (tokens[idx + 0].cvalue == validcombo[i][0]
			&& tokens[idx + 1].cvalue == validcombo[i][1]
			&& tokens[idx + 2].cvalue == validcombo[i][2])
		{
			return true;
		}
	}

	return false;
}

bool CheckCombo(const vector<Token> & tokens, int idx0, int idx1)
{
	const char * validcombo[] = {"((", "))", "(N", "N)", "NO", "ON", "O(", ")O", NULL};

	for (int i = 0; validcombo[i]; i++)
	{
		if (tokens[idx0].cvalue == validcombo[i][0]
			&& tokens[idx1].cvalue == validcombo[i][1])
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

bool CheckVariables(const vector<Token> & tokens, int first, int last)
{
	for (int i = first; i < last; i++)
	{
		const Token & currentToken = tokens[i];
		if (currentToken.type == VARIABLE_NAME)
		{
			if (strcmp(currentToken.strvalue, "pi") != 0)
			{
				cout << "error : variable not found : " << currentToken.strvalue << endl;
				return false;
			}
		}
	}
	return true;
}

bool CheckFunctions(const vector<Token> & tokens, int first, int last)
{
	for (int i = first; i < last; i++)
	{
		const Token & currentToken = tokens[i];
		if (currentToken.type == FUNCTION_NAME)
		{
			cout << "toto" << endl;
			if (strcmp(currentToken.strvalue, "max") != 0 || currentToken.dvalue != 2.0)
			{
				cout << "error : function not found : " << currentToken.strvalue << endl;
				return false;
			}
		}
	}
	return true;
}

bool Check(const vector<Token> & tokens, int first, int last)
{
	int size = last - first;
	if (size == 1)
	{
		return Check1(tokens, first)
			&& CheckVariables(tokens, first, last)
			&& CheckFunctions(tokens, first, last);
	}
	else if (size == 2)
	{
		return Check2(tokens, first)
			&& CheckVariables(tokens, first, last)
			&& CheckFunctions(tokens, first, last);
	}
	else if (size == 3)
	{
		return Check3(tokens, first)
			&& CheckVariables(tokens, first, last)
			&& CheckFunctions(tokens, first, last);
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

	if (!CheckVariables(tokens, first, last))
	{
		return false;
	}

	if (!CheckFunctions(tokens, first, last))
	{
		return false;
	}

	return true;
}

// ------------------------------ COMPUTER -----------------------------------

double Compute(double a, char op, double b)
{
	double result = 0.0;

	switch(op)
	{
		case '+': result = a + b; break;
		case '-': result = a - b; break;
		case '*': result = a * b; break;
		case '/': result = a / b; break;
		default : cout << "[Compute] unknown operator " << op << endl;
	}

	return result;
}

bool Compute2(double & result, double val, const vector<Token> & tokens, int first)
{
	//if (Check3(tokens, first))
	{
		if (tokens[first].type == OPERATOR)
		{
			double a = val;
			double b = tokens[first+1].dvalue;
			result = Compute(a, tokens[first].strvalue[0], b);
			/*switch(tokens[first].strvalue[0])
			{
				case '+': result = a + b; break;
				case '-': result = a - b; break;
				case '*': result = a * b; break;
				case '/': result = a / b; break;
			}*/
			return true;
		}
	}
	return false;
}


bool Compute3(double & result, const vector<Token> & tokens, int first)
{
	if (Check3(tokens, first))
	{
		// NON
		if (tokens[first + 1].type == OPERATOR)
		{
			double a = tokens[first].dvalue;
			double b = tokens[first+2].dvalue;
			result = Compute(a, tokens[first + 1].strvalue[0], b);
			/*switch(tokens[first + 1].strvalue[0])
			{
				case '+': result = a + b; break;
				case '-': result = a - b; break;
				case '*': result = a * b; break;
				case '/': result = a / b; break;
			}*/
			return true;
		}
		// (N)
		if (tokens[first + 1].type == NUMBER || tokens[first + 1].type == VARIABLE_NAME)
		{
			result = tokens[first+1].dvalue;
			return true;
		}
	}
	return false;
}

// -------------------------------- MISC -------------------------------------

int FindChar(const vector<Token> & tokens, int first, int last, char c, int dir = 1)
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

// ------------------------------ VARIABLES ----------------------------------

void UpdateVariables(vector<Token> & tokens, int first, int last)
{
	for (int i = first; i < last; i++)
	{
		Token & currentToken = tokens[i];
		if (currentToken.type == VARIABLE_NAME)
		{
			if (strcmp(currentToken.strvalue, "pi") == 0)
			{
				currentToken.dvalue = 3.1415;
			}
		}
	}
}

// ------------------------------ EVALUATOR ----------------------------------

int EvaluateWithoutParenthesis(const vector<Token> & tokens, int first, int last)
{
	double result = tokens[first].dvalue;

	for (int i = first + 1; i < last; i+= 2)
	{
		double nextValue = tokens[i+1].dvalue;
		result = Compute(result, tokens[i].strvalue[0], nextValue);
		/*switch(tokens[i].strvalue[0])
		{
			case '+': result += nextValue; break;
			case '-': result -= nextValue; break;
			case '*': result *= nextValue; break;
			case '/': result /= nextValue; break;
		}*/
	}

	return result;
}

double Evaluate(const vector<Token> & tokens, int first, int last)
{
	for (int i = first; i < last; i++)
	{
		cout << tokens[i].strvalue;
	}
	cout << endl;
	int size = last - first;

	double result = 0.0;

	if (size == 1)
	{
		return tokens[first].dvalue;
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
	if (FindChar(tokens, first, last, '(') < 0)
	{
		result = EvaluateWithoutParenthesis(tokens, first, last);
		return result;
	}

	// evaluate the first expression
	if (tokens[first].type == PARENTHESIS && tokens[first].cvalue == '(')
	{
		int expressionFirstIdx;
		int expressionLastIdx;
		if (GetParenthesedExpression(tokens, first, last, first, expressionFirstIdx, expressionLastIdx))
		{
			result = Evaluate(tokens, expressionFirstIdx + 1, expressionLastIdx - 1);
			first = expressionLastIdx;
		}
	}
	else if (tokens[first].type == NUMBER || tokens[first].type == VARIABLE_NAME)
	{
		result = tokens[first].dvalue;
		first++;
	}

	if (first != last)
	{
		double nextValue = Evaluate(tokens, first + 1, last);
		result = Compute(result, tokens[first].strvalue[0], nextValue);
		/*switch(tokens[first].strvalue[0])
		{
			case '+': result += nextValue; break;
			case '-': result -= nextValue; break;
			case '*': result *= nextValue; break;
			case '/': result /= nextValue; break;
		}*/
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
		if (tokens[idx].cvalue == '(')
		{
			firstIdx = idx;
			for(;idx < last;idx++)
			{
				if (tokens[idx].type == PARENTHESIS)
				{
					if (tokens[idx].cvalue == '(')
					{
						depth++;
					}
					else if (tokens[idx].cvalue == ')')
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
		else if (tokens[idx].cvalue == ')')
		{
			lastIdx = idx + 1;
			for(;idx >= first;idx--)
			{
				if (tokens[idx].type == PARENTHESIS)
				{
					if (tokens[idx].cvalue == '(')
					{
						depth++;
						if (depth == 0)
						{
							firstIdx = idx;
							return true; // found
						}

					}
					else if (tokens[idx].cvalue == ')')
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
	int mulIdx = FindChar(tokens, first, last, '*');
	int divIdx = FindChar(tokens, first, last, '/');
	int mulDivIdx = (mulIdx != -1 && divIdx != -1 ? std::min(mulIdx, divIdx): std::max(mulIdx, divIdx));
	while (mulDivIdx != -1)
	{
		if ((tokens[mulDivIdx - 1].type == NUMBER || tokens[mulDivIdx - 1].type == VARIABLE_NAME)
		&& (tokens[mulDivIdx + 1].type == NUMBER || tokens[mulDivIdx + 1].type == VARIABLE_NAME))
		{
			tokens.insert(tokens.begin() + mulDivIdx - 1, Token('('));
			tokens.insert(tokens.begin() + mulDivIdx + 3, Token(')'));
			last += 2;
			mulDivIdx += 2;
		}
		else if ((tokens[mulDivIdx - 1].type == NUMBER || tokens[mulDivIdx - 1].type == VARIABLE_NAME)
			&& tokens[mulDivIdx + 1].type == PARENTHESIS)
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
		else if (tokens[mulDivIdx - 1].type == PARENTHESIS
			&& (tokens[mulDivIdx + 1].type == NUMBER || tokens[mulDivIdx + 1].type == VARIABLE_NAME))
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

		

		mulIdx = FindChar(tokens, mulDivIdx, last, '*');
		divIdx = FindChar(tokens, mulDivIdx, last, '/');
		mulDivIdx = (mulIdx != -1 && divIdx != -1 ? std::min(mulIdx, divIdx): std::max(mulIdx, divIdx));

		cout << "mulIdx = " << mulIdx << endl;
		cout <<endl;
		for (size_t i = 0; i < tokens.size(); i++)
		{
			cout << tokens[i].strvalue;
		}
		cout <<endl;
	}
}

double Evaluate(const string & str)
{
	vector<Token> tokens;
	Tokenize(tokens, str);
	if (Check(tokens, 0, tokens.size()))
	{
		UpdateVariables(tokens, 0, tokens.size());
		Priorize(tokens, 0, tokens.size());
		return Evaluate(tokens, 0, tokens.size());
	}
	cout << "error : " << str << endl;
	return 0;
}

// -------------------------------- TESTER -----------------------------------

int main(int argc, char ** argv)
{
	assert(42 == Evaluate("42"));
	assert(2.5 == Evaluate("5/2"));
	assert(99 + 42 == Evaluate("99 + 42"));
	assert(99 * 42 == Evaluate("99 * 42"));
	assert(11 + 22 * 33 == Evaluate("11 + 22 * 33"));
	assert(36.0 + 50.0 * 100.0 / 2.0 * 3.0 == Evaluate("36 + 50 * 100 / 2 * 3"));
	assert(99.0 * 56.0 + 25.0 * 37.0 / 3.0 * 5.0 == Evaluate("99 * 56 + 25 * 37 / 3 * 5"));


	while (true)
	{
		string in;
		vector<Token> tokens;
	
		cout << "expr ?" << endl;
		cin >> in; 

		double result = Evaluate(in);
		cout << "result = "<<  result <<endl;
	}

	return 0;
}
