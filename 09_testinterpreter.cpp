#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include <assert.h>

/*
 * A simple calculator with 4 binary operations *, /, +, - and parenthesis ( and ).
 * It works fine.
 *
 * ps : I don't use yacc or lex but it can help you if you want to dev a compiler or a calculator
 *
 * [TODO] => It's only for the tutorial. It's not for advanced developers and it's obviously not high optimized ^^
 *	- It's my old code. It needs more OOP.
 * 	- need the negative operator (unary operator)
 *	- need math functions like sine or cosine
 */


using namespace std;

// ---------------------------- CONSTANTES -----------------------------------

static const double pi = 3.14159265358979323846;
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
bool GetFunctionExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx);

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
					nextToken.cvalue = '[';
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
									currentToken2.cvalue = ']';
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
	const char * validcombo[] = {"(N)", "NON", "F[]", NULL};

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
	const char * validcombo[] = {"((", "))",
		"(N", "N)", "NO", "ON", "O(", ")O",
		"F[", "[F", ",F", "],", ")]", "]], ""[(", "[]", ",(", "),",
		"[N", "N]", "N,", ",N", "OF", "]O", NULL};

	for (int i = 0; validcombo[i]; i++)
	{
		if (tokens[idx0].cvalue == validcombo[i][0]
			&& tokens[idx1].cvalue == validcombo[i][1])
		{
			return true;
		}
	}

	cout << "[CheckCombo] wrong combo " << tokens[idx0].cvalue << tokens[idx1].cvalue << endl;

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
			cout << "check function : " << currentToken.strvalue << endl;
			if (strcmp(currentToken.strvalue, "max") == 0 && currentToken.dvalue == 2.0)
			{
				return true;
			}
			else if (strcmp(currentToken.strvalue, "cos") == 0 && currentToken.dvalue == 1.0)
			{
				return true;
			}
			else
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

int FindChar(const vector<Token> & tokens, int first, int last, char c, int dir = 1, bool checkDepth = false)
{
	int depth = 0;

	if (dir > 0)
	{
		for (int idx = first; idx < last; idx++)
		{
			if (checkDepth)
			{
				if (tokens[idx].cvalue == '(' || tokens[idx].cvalue == '[')
				{
					depth++;
				}

				if (tokens[idx].cvalue == ')' || tokens[idx].cvalue == ']')
				{
					depth--;
				}
			}

			if (depth == 0 && (tokens[idx].strvalue[0] == c || tokens[idx].cvalue == c))
			{

				return idx;
			}
		}
	}
	else if (dir < 0)
	{
		for (int idx = last-1; idx >= first; idx--)
		{
			if (checkDepth)
			{
				if (tokens[idx].cvalue == '(' || tokens[idx].cvalue == '[')
				{
					depth++;
				}

				if (tokens[idx].cvalue == ')' || tokens[idx].cvalue == ']')
				{
					depth--;
				}
			}

			if (depth == 0 && (tokens[idx].strvalue[0] == c || tokens[idx].cvalue == c))
			{

				return idx;
			}
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
				currentToken.dvalue = pi;
			}
		}
	}
}

// ------------------------------ FUNCTIONS ----------------------------------

double CallFunction(const Token& function, vector<double> & args)
{
	assert(function.dvalue == args.size());

	if (strcmp("max", function.strvalue) == 0)
	{
		return std::max(args[0], args[1]);
	}

	if (strcmp("cos", function.strvalue) == 0)
	{
		return cos(args[0]);
	}

	cout << "error : Call Function : function not found : " << function.strvalue <<endl;
	assert(0);

	return 0.0;
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
	else if (tokens[first].type == FUNCTION_NAME)
	{
		vector<double> args;

		// parse args list
		int functionExpressionFirst = -1;
		int functionExpressionLast = -1;
		assert(GetFunctionExpression(tokens, first, last, first, functionExpressionFirst, functionExpressionLast));
		int token_idx_in_args_list = functionExpressionFirst + 2;
		int end_idx_args_list = functionExpressionLast - 1;
		assert(end_idx_args_list > 0);
		assert(end_idx_args_list < last);

		int next_token_idx_in_args_list = FindChar(tokens, token_idx_in_args_list + 1, last, ',', 1, true);
		while (next_token_idx_in_args_list > 0)
		{
			double currentArg = Evaluate(tokens, token_idx_in_args_list, next_token_idx_in_args_list);
			args.push_back(currentArg);
			token_idx_in_args_list = next_token_idx_in_args_list + 1;
			next_token_idx_in_args_list = FindChar(tokens, token_idx_in_args_list + 1, last, ',', 1, true);
			cout << "arg = " << currentArg << endl;
		}
		if (token_idx_in_args_list != end_idx_args_list)
		{
			double currentArg = Evaluate(tokens, token_idx_in_args_list, end_idx_args_list);
			args.push_back(currentArg);
			cout << "arg = " << currentArg << endl;
		}
		cout << "args.size() = " << args.size() << endl;
		result = CallFunction(tokens[first], args);
		first = end_idx_args_list + 1;
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

bool GetFunctionExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx)
{
	if (idx < first)
	{
		return false;
	}

	if (idx >= last)
	{
		return false;
	}

	if (tokens[idx].type == FUNCTION_NAME || tokens[idx].type == FUNCTION_ARGS_END)
	{
		int depth = 0;
		if (tokens[idx + 1].cvalue == '[')
		{
			firstIdx = idx;
			idx++;
			for(;idx < last;idx++)
			{
				if (tokens[idx].type == FUNCTION_ARGS_BEGIN || tokens[idx].type == FUNCTION_ARGS_END)
				{
					if (tokens[idx].cvalue == '[')
					{
						depth++;
					}
					else if (tokens[idx].cvalue == ']')
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
		else if (tokens[idx].cvalue == ']')
		{
			lastIdx = idx + 1;
			for(;idx >= first;idx--)
			{
				if (tokens[idx].type == FUNCTION_ARGS_BEGIN || tokens[idx].type == FUNCTION_ARGS_END)
				{
					if (tokens[idx].cvalue == '[')
					{
						depth++;
						if (depth == 0)
						{
							firstIdx = idx - 1;
							return firstIdx >= first && tokens[firstIdx].type == FUNCTION_NAME; // found
						}

					}
					else if (tokens[idx].cvalue == ']')
					{
						depth--;
					}
				}
			}
		}
	}

	return false; // not found
}

void PriorizeFunctions(vector<Token> & tokens, int & first, int & last);

void Priorize(vector<Token> & tokens, int first, int last)
{
	PriorizeFunctions(tokens, first, last);

	int mulIdx = FindChar(tokens, first, last, '*');
	int divIdx = FindChar(tokens, first, last, '/');
	int mulDivIdx = (mulIdx != -1 && divIdx != -1 ? std::min(mulIdx, divIdx): std::max(mulIdx, divIdx));
	while (mulDivIdx != -1)
	{
		const Token & tokenBeforeOperator = tokens[mulDivIdx - 1];
		const Token & tokenAfterOperator = tokens[mulDivIdx + 1];
		if ((tokenBeforeOperator.type == NUMBER || tokenBeforeOperator.type == VARIABLE_NAME)
		&& (tokenAfterOperator.type == NUMBER || tokenAfterOperator.type == VARIABLE_NAME))
		{
			tokens.insert(tokens.begin() + mulDivIdx - 1, Token('('));
			tokens.insert(tokens.begin() + mulDivIdx + 3, Token(')'));
			last += 2;
			mulDivIdx += 2;
		}
		else if ((tokenBeforeOperator.type == NUMBER || tokenBeforeOperator.type == VARIABLE_NAME)
			&& tokenAfterOperator.type == PARENTHESIS)
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
		else if (tokenBeforeOperator.type == PARENTHESIS
			&& (tokenAfterOperator.type == NUMBER || tokenAfterOperator.type == VARIABLE_NAME))
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

void PriorizeFunctions(vector<Token> & tokens, int & first, int & last)
{
int cpt = 0;
	int idx = FindChar(tokens, first, last, 'F');
	while (idx >= first)
	{
		int expressionFirstIdx;
		int expressionLastIdx;

		if (GetFunctionExpression(tokens, first, last, idx, expressionFirstIdx, expressionLastIdx))
		{
	for (size_t i = 0; i < tokens.size(); i++)
	{
		cout << tokens[i].strvalue << " ";
	}
	cout << endl;
		cout << "expressionFirstIdx = "<< expressionFirstIdx <<endl;
		cout << "expressionLastIdx = "<< expressionLastIdx <<endl;
		cout << "A";	tokens.insert(tokens.begin() + expressionFirstIdx, Token('('));
		cout << "B";	tokens.insert(tokens.begin() + expressionLastIdx + 1, Token(')'));
		cout << "C";	idx = expressionLastIdx;
			first --;
			last += 2;
		}
		else
		{
			break;
		}

		idx = FindChar(tokens, idx + 3, last, 'F');

		//if (cpt++ >= 5) break;
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
	// assertion for constants check
	assert(42 == Evaluate("42"));
	assert(2.5 == Evaluate("5/2"));

	// assertion of binary operations check
	assert(99 + 42 == Evaluate("99 + 42"));
	assert(99 * 42 == Evaluate("99 * 42"));
	assert(11 + 22 * 33 == Evaluate("11 + 22 * 33"));
	assert(36.0 + 50.0 * 100.0 / 2.0 * 3.0 == Evaluate("36 + 50 * 100 / 2 * 3"));
	assert(99.0 * 56.0 + 25.0 * 37.0 / 3.0 * 5.0 == Evaluate("99 * 56 + 25 * 37 / 3 * 5"));

	// assertion for variables check
	assert(pi == Evaluate("pi"));
	assert(2.0 * pi == Evaluate("2 * pi"));

	// assertion for functions check (simple)
	assert(55.0 == Evaluate("max(55,22)"));
	assert(55.0 == Evaluate("max(22,55)"));
	assert(1.0 == Evaluate("cos(0)"));
	assert(cos(pi) == Evaluate("cos(pi)"));
	assert(cos(2 * pi) == Evaluate("cos(2 * pi)"));

	// assertion for functions check (multiple functions or "function inception")
	assert(cos(cos(0.5 * pi)) == Evaluate("cos(cos(0.5 * pi))"));
	assert(cos(0) + cos(0) + cos(cos(0.5*pi)) == Evaluate("cos(0) + cos(0) + cos(cos(0.5*pi))"));
	assert(99.0 == Evaluate("max(22,max(99,55))"));
	assert(99.0 == Evaluate("max(max(99,55), 22)"));
	assert(44.0 == Evaluate("max(max(11,22), max(33,44))"));
	assert(44.0 == Evaluate("max(max(33,44), max(11,22))"));
	assert(45.0 == Evaluate("max(max(11,22) + 1, max(33,44) + 1)"));
	assert(45.0 == Evaluate("max(max(33,44) + 1, max(11,22) + 1)"));

	while (true)
	{
		string in;
		vector<Token> tokens;
	
		cout << "expr ?" << endl;
		getline(cin, in);	// *fix: use "getline(cin, in)" instead of "cin >> in"
					// cuz cin split the str with space

		double result = Evaluate(in);
		cout << "result = "<<  result <<endl;
	}

	return 0;
}
