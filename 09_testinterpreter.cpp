#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>

/*
 * A simple C-like interpreter with :
 *	- 4 binary operations *, /, +, - and parenthesis ( and ).
 *	- 2 binary boolean operations && and ||.
 *	- 1 comparaison operation ==.
 *	- 1 variable/constante : pi.
 *	- 2 functions : max(double, double) and cos(double).
 *	- 'return' keyword management.
 *	- 'if (<condition>) <statement>;' management.
 *	- 'if (<condition>) <statementA>; else <statementB>;' management.
 * It works fine.
 *
 * ps : I don't use yacc or lex but it can help you if you want to dev a compiler or a calculator
 *
 * [TODO] => It's only for the tutorial. It's not for advanced developers and it's obviously not yet high optimized ^^
 *	- It uses my old code. It needs more OOP.
 * 	- need the negative operator (unary operator).
 *	- need more math functions like sine or cosine.
 *	- need a bytecode translator/compiler and a Virtual Machine.
 */





#if USE_STL
#include <vector>
#include <string>
#include <iostream>
using namespace std;
#else
#include <stdio.h>
using namespace std;
namespace std {
	template <typename T>
	T min(const T& a, const T & b)
	{
		return a < b? a : b;
	}

	template <typename T>
	T max(const T& a, const T & b)
	{
		return a > b? a : b;
	}
};

// minimalist implementation of the stl vector
class string
{
public:
	string(const char * str = "")
	{
		s = strdup(str);
		len = strlen(s);
	}

	string(const string & other)
	{
		s = strdup(other.s);
		len = other.len;
	}

	string& operator=(const string & other)
	{
		free((void*)s);
		s = strdup(other.s);
		len = other.len;
		return *this;
	}

	virtual ~string()
	{
		free((void*)s);
	}

	const char* c_str() const { return s; }
	size_t length() const { return len; }
	const char& operator[](size_t i) const { return s[i];}

private:
	const char * s;
	size_t len;

};

// minimalist implementation of the stl vector
template <typename T>
class vector
{
public:
	vector()
	{
		m_data = NULL;
		m_size = 0;
		m_capacity = 0;
	}

	virtual ~vector()
	{
		if (m_data)
		{
			delete [] m_data;
		}
	}

	size_t size() const { return m_size;}
	T* begin(){ return m_data;}
	T* end() { return m_data + m_size;}

	T& operator[](size_t i) { return m_data[i];}
	T& operator[](int i) { return m_data[i]; }
	const T& operator[](size_t i) const { return m_data[i]; }
	const T& operator[](int i) const { return m_data[i]; }

	void push_back(const T& item)
	{
		if (m_size == m_capacity)
		{
			if (m_capacity == 0)
			{
				m_capacity = 1; // must be optimized with a bigger value like 32
				m_data = new T[m_capacity];
			}
			else
			{
				T* newData = new T[m_capacity * 2]; // need to optimize the ctr calls
				memcpy(newData, m_data, m_capacity * sizeof(T));
				delete [] m_data;
				m_data = newData;
				m_capacity *= 2;
			}
		}

		m_data[m_size++] = item;
	}

	void insert(T* pos, const T& item)
	{
		if (pos == end())
		{
			push_back(item);
		}
		else
		{
			size_t idx = pos - begin();
			size_t n = m_size - idx;
			if (m_size == m_capacity)
			{
				push_back(T());
				m_size--;
			}
			memmove(m_data + idx + 1, m_data + idx, n * sizeof(T));
			m_data[idx] = item;
			m_size++;
		}
	}


private:
	T * m_data;
	size_t m_size;
	size_t m_capacity;
};

static FILE * cin = stdin;

bool getline(FILE * stream, string & in, char delim = '\n')
{
	vector<char> s;
	char c;

	while(true)
	{
		c = getc(stream);

		if (c == EOF) {
		    break;
		}

		if (c == delim) {
		    break;
		}

		s.push_back(c);
	}
	s.push_back('\0');
	in = string(&s[0]);
	return true;
}
#endif
// ---------------------------- CONSTANTES -----------------------------------

static const double pi = 3.14159265358979323846;
static const size_t NUMBER_DIGITS_MAX = 32;
static const size_t NAME_NB_CHARS_MAX = 32;

// ---------------------- VARIABLES (TEMP CODE) ------------------------------

double* GetVariablePtr(void *handle, const char * variableName);
const double* GetConstantePtr(void *handle, const char * constanteName);
unsigned int ComputeCRC32(const void * buffer, size_t len, unsigned int crc = 0xffffffff);

// ------------------------------- TOKEN -------------------------------------


enum TokenType
{
	NONE, NUMBER, STRING, OPERATOR, PARENTHESIS, COMMA,
	NAME, VARIABLE_NAME, FUNCTION_NAME, /* NAME =  VARIABLE_NAME or FUNCTION_NAME*/
	FUNCTION_ARGS_BEGIN, FUNCTION_ARGS_SEPARATOR, FUNCTION_ARGS_END, /* ( , ) */
	IF, ELSE, RETURN, SEMICOLON, SCOPE, /* SEMICOLON = ';'*/
	STORE /* STORE = '=' */
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
		dvaluePtr = NULL;
	}

	Token(char c, char c2 = '\0')
	{
		if (c == '(' || c == ')')
		{
			type = PARENTHESIS;
			cvalue = c;
		}
		else if (c == '+' || c == '-' || c == '*' || c == '/'
			|| (c == '&' && c2 == '&')
			|| (c == '|' && c2 == '|')
			|| (c == '=' && c2 == '='))
		{
			type = OPERATOR;
			cvalue = 'O';
		}
		else if (c == '=')
		{
			type = STORE;
			cvalue = '=';
		}
		else if (c == ',')
		{
			type = COMMA;
			cvalue = ',';
		}
		else if (c == ';')
		{
			type = SEMICOLON;
			cvalue = ';';
		}
		else if (c == '{' || c == '}')
		{
			type = SCOPE;
			cvalue = c;
		}
		else
		{
			type = NONE;
			cvalue = '?';
		}
		strvalue[0] = c;
		strvalue[1] = c2;
		strvalue[2] = '\0';
		dvalue = 0.0;
		dvaluePtr = NULL;
		dvaluePtrIsConstante = true;
	}

	Token(const char * str)
	{
		assert(str != NULL);
		strncpy(strvalue, str, sizeof(strvalue) - 1); 	// strncpy is safer than strcpy
		strvalue[sizeof(strvalue) - 1] = '\0';		// mandatory if str is bigger than strvalue

		char c = *str;
		char c2 = *(str + 1);
		if (c == '(' || c == ')')
		{
			*this = Token(c);
		}
		else if (c == '+' || c == '-' || c == '*' || c == '/')
		{
			*this = Token(c);
		}
		else if	((c == '&' && c2 == '&') || (c == '|' && c2 == '|') || (c == '=' && c2 == '='))
		{
			*this = Token(c, c2);
			strvalue[0] = c;
			strvalue[1] = c2;
			strvalue[2] = '\0';
		}
		else if (c == '=')
		{
			*this = Token(c);
		}
		else if (c == ',' || c == ';')
		{
			*this = Token(c);
		}
		else if (c == '{' || c == '}')
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
			if (strcmp(str, "if") == 0)
			{
				type = IF;
				cvalue = 'I';
				dvalue = 0.0;
			}
			else if (strcmp(str, "return") == 0)
			{
				type = RETURN;
				cvalue = 'R';
				dvalue = 0.0;
			}
			else if (strcmp(str, "else") == 0)
			{
				type = ELSE;
				cvalue = 'E';
				dvalue = 0.0;
			}
			else
			{
				type = NAME; // NAME =  VARIABLE_NAME or FUNCTION_NAME
				cvalue = 'N';
				dvalue = 0.0;
			}
		}
		else if (c == '\"')
		{
			size_t len = std::min(strlen(str) - 2, sizeof(strvalue) - 1);
			type = STRING;
			cvalue = 'S';
			strncpy(strvalue, str + 1, sizeof(strvalue) - 1);// remove the first '"'
			strvalue[len] = '\0';				// remove the last '"'
			dvalue = ComputeCRC32(strvalue, len);		// compute the crc32 only for the string
		}
		else
		{
#if USE_STL
			cerr << "unknown token : " << str << endl;
#else
			printf("unknown token : %s\n", str); //  [TODO] plz use the stderr(2)
#endif
			assert(0);
		}
	}

	double GetDoubleValue() const
	{
		if (type == VARIABLE_NAME && dvaluePtr != NULL && !dvaluePtrIsConstante)
		{
			return *dvaluePtr;
		}
		else
		{
			return dvalue;
		}
	}

	char strvalue[NAME_NB_CHARS_MAX + 1];
	char cvalue;
	double dvalue;
	double* dvaluePtr;
	bool dvaluePtrIsConstante;
};


bool GetParenthesedExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx);
bool GetScopedExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx);
bool GetScopedExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx,
	int & ifScopedIdx, int & elseScopedIdx);
bool GetFunctionExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx);

// ------------------------------ TOKENIZER ----------------------------------
void TokenizePostProcess(vector<Token> & tokens);

void Tokenize(vector<Token> & tokens, const string & str)
{
	char * s = (char*)str.c_str();
	char c;
	char c2;

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

		c2 = *s;
		if (isdigit(c))
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
		else if (c == '\"')
		{
			char _str[NAME_NB_CHARS_MAX + 2 + 1] = {0};
			size_t _strlen = 0;
			char * start;
			char * end;
			start = end = s-1;
			int n = 0;

			c = *++end;
			while(c && c != '\"')
			{
				c = *++end;
			}
			s = end + 1;
			_strlen = min(size_t(end - start) + 1, NAME_NB_CHARS_MAX + 2);
			strncpy(_str, start, _strlen);
			if (_strlen == NAME_NB_CHARS_MAX + 2) _str[_strlen - 1] = '\"';
			tokens.push_back(Token(_str));
		}
		else if (c == '+' || c == '-' || c == '*' || c == '/')
		{
			tokens.push_back(Token(c));
		}
		else if ((c == '&' && c2 == '&') || (c == '|' && c2 == '|') || (c == '=' && c2 == '='))
		{
			char tmp[3] = {c, c2, '\0'};
			tokens.push_back(Token(tmp));
			s++;
		}
		else if (c == '=' && c2!='=')
		{
			tokens.push_back(Token(c));
		}
		else if (c == '(' || c ==')')
		{
			tokens.push_back(Token(c));
		}
		else if (c == ',' || c == ';')
		{
			tokens.push_back(Token(c));
		}
		else if (c == '{' || c == '}')
		{
			tokens.push_back(Token(c));
		}
		else
		{
#if USE_STL
			cout << "UNKNOW CHARACTER " << c << endl;
#else
			printf("UNKNOW CHARACTER %c\n", c);
#endif
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
			lastToken.cvalue = 'N';
		}
	}
}

void PrintTokensList(const vector<Token> & tokens, int first, int last)
{
	for (int i = first; i < last; i++)
	{
#if USE_STL
		cout << tokens[i].strvalue << " ";
#else
		printf("%s ", tokens[i].strvalue);
#endif
	}
#if USE_STL
	cout << endl;
#else
	putchar('\n');
#endif

}

// ------------------------------ CHECKER ------------------------------------

bool Check1(const vector<Token> & tokens, int idx)
{
	return tokens[idx].type == NUMBER || tokens[idx].type == VARIABLE_NAME || tokens[idx].type == STRING;
}

bool Check2(const vector<Token> & tokens, int idx)
{
	return	(
			(tokens[idx].type == RETURN || tokens[idx].type == ELSE)
			&& (tokens[idx + 1].type == NUMBER || tokens[idx + 1].type == VARIABLE_NAME)
		)
		||
			(tokens[idx].type == SCOPE && tokens[idx].cvalue == '{'
			&& tokens[idx + 1].type == SCOPE && tokens[idx + 1].cvalue == '}');
}

bool Check3(const vector<Token> & tokens, int idx)
{
	const char * validcombo[] = {"(N)", "(S)", "NON", "F[]", "ERN", "{N}", "{S}", "{;}", NULL};

	for (int i = 0; validcombo[i]; i++)
	{
		if (tokens[idx + 0].cvalue == validcombo[i][0]
			&& tokens[idx + 1].cvalue == validcombo[i][1]
			&& tokens[idx + 2].cvalue == validcombo[i][2])
		{
			return true;
		}
	}

	// special case : 'Variable = Number' or 'Variable = Variable' or 'Variable = ('
	if (tokens[idx + 0].type == VARIABLE_NAME
		&& tokens[idx + 1].cvalue == '='
		&& tokens[idx + 2].cvalue == 'N')
	{
		return true;
	}

	return false;
}

bool CheckCombo(const vector<Token> & tokens, int idx0, int idx1)
{
	const char * validcombo[] = {"((", "))",
		"(N", "N)", "NO", "ON", "O(", ")O",
		"F[", "[F", ",F", "],", ")]", "]], ""[(", "[]", ",(", "),",
		"[N", "N]", "N,", ",N", "OF", "]O",
		"I(", ")I", "RN", "R(", ")R", "RF", ";R",
		";E", "EN", "ER", "E(", "EF", "E;",
		"N;", ";N", ");", ";(", ";I", ";F", "];",
		"{N", "{F", "{(", "{{", "{R", "{I", "){", "{;", "};", ";;",
		"{}", ";}", "}}", "}N", "}F", "}R", "}I", "}E", "E{",
		"=N", "=(", "=F",
		"S,", ",S", "(S", "S)","[S", "S]", "S;", ";S", NULL};

	for (int i = 0; validcombo[i]; i++)
	{
		if (tokens[idx0].cvalue == validcombo[i][0]
			&& tokens[idx1].cvalue == validcombo[i][1])
		{
			return true;
		}
	}

	// special case : only 'Variable ='  is accepted
	if (tokens[idx0].type == VARIABLE_NAME
		&& tokens[idx1].cvalue == '=')
	{
		return true;
	}

#if USE_STL
	cout << "[CheckCombo] wrong combo " << tokens[idx0].cvalue << tokens[idx1].cvalue << endl;
#else
	printf("[CheckCombo] wrong combo %c%c\n", tokens[idx0].cvalue, tokens[idx1].cvalue);
#endif
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
			if (!GetVariablePtr(NULL, currentToken.strvalue)
				&& !GetConstantePtr(NULL, currentToken.strvalue))
			{
#if USE_STL
				cout << "error : variable not found : " << currentToken.strvalue << endl;
#else
				printf("error : variable not found : %s\n", currentToken.strvalue);
#endif
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
#if defined(DEBUG)
			cout << "check function : " << currentToken.strvalue << endl;
#endif
			if (strcmp(currentToken.strvalue, "print") == 0)
			{
				return true;
			}
			else if (strcmp(currentToken.strvalue, "max") == 0 && currentToken.dvalue == 2.0)
			{
				return true;
			}
			else if (strcmp(currentToken.strvalue, "cos") == 0 && currentToken.dvalue == 1.0)
			{
				return true;
			}
			else
			{
#if USE_STL
				cout << "error : function not found : " << currentToken.strvalue << endl;
#else
				printf("error : function not found : %s\n", currentToken.strvalue);
#endif
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
		case '&': result = double(a != 0.0 && b != 0.0); break;
		case '|': result = double(a != 0.0 || b != 0.0); break;
		case '=': result = double(a == b); break; // TODO: need an epsilon
		default :
#if USE_STL
			cout << "[Compute] unknown operator " << op << endl;
#else
			printf("[Compute] unknown operator %c\n", op);
#endif
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
			double b = tokens[first+1].GetDoubleValue();
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
			double a = tokens[first].GetDoubleValue();
			double b = tokens[first+2].GetDoubleValue();
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
			result = tokens[first+1].GetDoubleValue();
			return true;
		}
	}
	return false;
}

unsigned int ComputeCRC32(const void * buffer, size_t len, unsigned int crc /*= 0xffffffff*/)
{
	static unsigned int crc32_table[256];
	static bool initialized = false;

	unsigned char *ptr = (unsigned char *)buffer;

	if (!initialized)
	{
		initialized = true;

		// Compute the CRC table
		for (unsigned int i = 0; i < 256; i++)
		{
			unsigned int tmp = i;
			for (unsigned int j = 0; j < 8; j++)
			{
				if (tmp & 1)
				{
					tmp >>= 1;
					tmp ^= 0xEDB88320;
				}
				else
				{
					tmp >>= 1;
				}
			}
			crc32_table[i] = tmp;
		}
	}

	while(len--)
	{
		crc = (crc >> 8) ^ crc32_table[(crc & 0xff) ^ *ptr++];
	}

	return ~crc;
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

// [TODO] need to be refactored with FindChar
int FindOperator(const vector<Token> & tokens, int first, int last, char c, int dir = 1, bool checkDepth = false)
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

			if (depth == 0 && tokens[idx].type == OPERATOR && (tokens[idx].strvalue[0] == c || tokens[idx].cvalue == c))
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

			if (depth == 0 && tokens[idx].type == OPERATOR && (tokens[idx].strvalue[0] == c || tokens[idx].cvalue == c))
			{

				return idx;
			}
		}
	}

	return -1;
}

// [TODO] need to be refactored with FindChar
int FindToken(const vector<Token> & tokens, int first, int last, TokenType type, int dir = 1, bool checkDepth = false)
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

			if (depth == 0 && tokens[idx].type == type)
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

			if (depth == 0 && tokens[idx].type == type)
			{

				return idx;
			}
		}
	}

	return -1;
}


// ------------------------------ VARIABLES ----------------------------------

double* GetVariablePtr(void *handle, const char * variableName)
{
	static double abc = 0.0;
	static double counter = 0.0;

	if (strcmp(variableName, "abc") == 0)
	{
		return &abc;
	}
	else if (strcmp(variableName, "counter") == 0)
	{
		return &counter;
	}
	return NULL;
}

const double* GetConstantePtr(void *handle, const char * constanteName)
{
	if (strcmp(constanteName, "pi") == 0)
	{
		return &pi;
	}
	return NULL;
}


bool SetVariable(void * handle, Token & token, double value)
{
	assert(token.type == VARIABLE_NAME);
	token.dvalue = value;

	double* variablePtr = GetVariablePtr(handle, token.strvalue);
	if (variablePtr)
	{
		*variablePtr = value;
	}
	return variablePtr != NULL;
}

void UpdateVariablesAddr(void* handle, vector<Token> & tokens, int first, int last)
{
	for (int i = first; i < last; i++)
	{
		Token & currentToken = tokens[i];
		if (currentToken.type == VARIABLE_NAME)
		{
			// apply relocation : symbole -> addr
			currentToken.dvaluePtr = (double*)GetConstantePtr(handle, currentToken.strvalue);
			currentToken.dvaluePtrIsConstante = true;
			if (currentToken.dvaluePtr == NULL)
			{
				currentToken.dvaluePtr = GetVariablePtr(handle, currentToken.strvalue);
				currentToken.dvaluePtrIsConstante = false;
			}

			// update value
			if (currentToken.dvaluePtr)
			{
				currentToken.dvalue = *currentToken.dvaluePtr;
			}
			else
			{
#if USE_STL
				cout << "variable not found " << currentToken.strvalue << endl;
#else
				printf("variable not found %s\n", currentToken.strvalue);
#endif
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
#if USE_STL
	cout << "error : Call Function : function not found : " << function.strvalue <<endl;
#else
	printf("error : Call Function : function not found : %s\n", function.strvalue);
#endif
	assert(0);

	return 0.0;
}
// ------------------------------ EVALUATOR ----------------------------------

int EvaluateWithoutParenthesis(const vector<Token> & tokens, int first, int last)
{
	double result = tokens[first].GetDoubleValue();

	for (int i = first + 1; i < last; i+= 2)
	{
		double nextValue = tokens[i+1].GetDoubleValue();
		if (tokens[i].type == OPERATOR)
			result = Compute(result, tokens[i].strvalue[0], nextValue);
		else assert(0);
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

double Evaluate1Statement(const vector<Token> & tokens, int first, int last, bool * hasReturn = NULL, bool * hasIfConditionTrue = NULL)
{
	Token firstToken = tokens[first];

#if defined(DEBUG)
	printf("Evaluate1Statement : ");
	PrintTokensList(tokens, first, last);
#endif

	int size = last - first;

	double result = 0.0;
	bool previousHasIfConditionTrue = false;
	if (hasReturn) *hasReturn = false;
	if (hasIfConditionTrue) {previousHasIfConditionTrue = *hasIfConditionTrue; *hasIfConditionTrue = false;}


	if (size == 1)
	{
		return tokens[first].GetDoubleValue();
	}
	else if (size == 2)
	{
		if (tokens[first].type == RETURN)
		{
			if (hasReturn) *hasReturn = true;
			return tokens[first + 1].GetDoubleValue();
		}
		else
		{
			return 0.0;
		}
	}
	else if (size == 3 && tokens[first].type != ELSE && tokens[first + 1].type != STORE)
	{
		Compute3(result, tokens, first);
		return result;
	}

	// good enough with 1 variable but doesnt work with a chain like 'var1 = var2 = 123'
	if (size >= 3 && tokens[first].type == VARIABLE_NAME && tokens[first + 1].type == STORE)
	{
		Token variable = tokens[first];
		double result = Evaluate1Statement(tokens, first + 2, last, hasReturn, hasIfConditionTrue);
		SetVariable(NULL, variable, result);
#ifdef DEBUG
		printf("_STORE %f\n", result);
#endif
		return result;
	}

	// no parenthesis => easy to evaluate
	if (FindChar(tokens, first, last, '(') < 0 && tokens[first].type != ELSE)
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
			result = Evaluate1Statement(tokens, expressionFirstIdx + 1, expressionLastIdx - 1);
			first = expressionLastIdx;
		}
	}
	else if (tokens[first].type == NUMBER || tokens[first].type == VARIABLE_NAME)
	{
		result = tokens[first].GetDoubleValue();
		first++;
	}
	else if (tokens[first].type == FUNCTION_NAME && strcmp(tokens[first].strvalue, "print") == 0)
	{
		// parse args list
		int functionExpressionFirst = -1;
		int functionExpressionLast = -1;
		assert(GetFunctionExpression(tokens, first, last, first, functionExpressionFirst, functionExpressionLast));
		int token_idx_in_args_list = functionExpressionFirst + 2;
		int end_idx_args_list = functionExpressionLast - 1;
		assert(end_idx_args_list > 0);
		assert(end_idx_args_list < last);

		int next_token_idx_in_args_list = FindChar(tokens, token_idx_in_args_list, last, ',', 1, true);
		while (next_token_idx_in_args_list > 0)
		{
			int size = next_token_idx_in_args_list - token_idx_in_args_list;
			if (tokens[token_idx_in_args_list].type == STRING)
			{
				printf("%s", tokens[token_idx_in_args_list].strvalue);
				assert(size == 1);
			}
			else
			{
				double dvalue = Evaluate1Statement(tokens, token_idx_in_args_list, next_token_idx_in_args_list);
				int ivalue = int(dvalue);
				if (double(ivalue) == dvalue)
				{
					printf("%d", ivalue);
				}
				else
				{
					printf("%f", dvalue);
				}
			}

			token_idx_in_args_list = next_token_idx_in_args_list + 1;
			next_token_idx_in_args_list = FindChar(tokens, token_idx_in_args_list + 1, last, ',', 1, true);
		}
		if (token_idx_in_args_list != end_idx_args_list)
		{
			int size = token_idx_in_args_list - end_idx_args_list;
			if (tokens[token_idx_in_args_list].type == STRING)
			{
				printf("%s", tokens[token_idx_in_args_list].strvalue);
				//assert(size == 1);
			}
			else
			{
				double dvalue = Evaluate1Statement(tokens, token_idx_in_args_list, end_idx_args_list);
				int ivalue = int(dvalue);
				if (double(ivalue) == dvalue)
				{
					printf("%d", ivalue);
				}
				else
				{
					printf("%f", dvalue);
				}
			}
		}

		printf("\n");
		first = end_idx_args_list + 1;
		result = 1; 
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

		int next_token_idx_in_args_list = FindChar(tokens, token_idx_in_args_list, last, ',', 1, true);
		while (next_token_idx_in_args_list > 0)
		{
			double currentArg = Evaluate1Statement(tokens, token_idx_in_args_list, next_token_idx_in_args_list);
			args.push_back(currentArg);
			token_idx_in_args_list = next_token_idx_in_args_list + 1;
			next_token_idx_in_args_list = FindChar(tokens, token_idx_in_args_list + 1, last, ',', 1, true);
#if defined(DEBUG)
			cout << "arg = " << currentArg << endl;
#endif
		}
		if (token_idx_in_args_list != end_idx_args_list)
		{
			double currentArg = Evaluate1Statement(tokens, token_idx_in_args_list, end_idx_args_list);
			args.push_back(currentArg);

#if defined(DEBUG)
			cout << "arg = " << currentArg << endl;
#endif
		}

#if defined(DEBUG)
		cout << "args.size() = " << args.size() << endl;
#endif
		result = CallFunction(tokens[first], args);
		first = end_idx_args_list + 1;
	}
	else if (tokens[first].type == IF)
	{
		while (tokens[first].type == IF)
		{
			if (tokens[first + 1].type == PARENTHESIS && tokens[first + 1].cvalue == '(')
			{
				int expressionFirstIdx;
				int expressionLastIdx;
				if (GetParenthesedExpression(tokens, first + 1, last, first + 1, expressionFirstIdx, expressionLastIdx))
				{
					result = Evaluate1Statement(tokens, expressionFirstIdx + 1, expressionLastIdx - 1);
					if (result != 0)
					{
						if (hasIfConditionTrue) *hasIfConditionTrue = true;
						first = expressionLastIdx;
						result = Evaluate1Statement(tokens, first, last, hasReturn, hasIfConditionTrue);
						//return result;
					}
					else
					{
						return 0.0;
					}
				}
				else
				{
#if USE_STL
					cout << "[Evaluate1Statement] : if error : missing '(' in if condition" << endl;
#else
					printf("[Evaluate1Statement] : if error : missing '(' in if condition\n");
#endif
					assert(0);
					return 0.0;
				}
			}
			else
			{
#if USE_STL
				cout << "[Evaluate1Statement] : if error : no '('" << endl;
#else
				printf("[Evaluate1Statement] : if error : no '('\n");
#endif

				assert(0);
				return 0.0;
			}
		}

		return result;
	}
	else if (tokens[first].type == ELSE)
	{
#if defined(DEBUG)
		cout << "E L S E " << endl;
#endif
		if (previousHasIfConditionTrue)
		{
			return 0.0;
		}
		else
		{
			return Evaluate1Statement(tokens, first + 1, last, hasReturn);
		}
	}
	else if (tokens[first].type == RETURN)
	{
		// TODO : search the end of the statement (;) or statements block {}
		if (hasReturn) *hasReturn = true;
		return Evaluate1Statement(tokens, first + 1, last);
	}

	if (first != last)
	{
		double nextValue = Evaluate1Statement(tokens, first + 1, last);

		if (tokens[first].type == OPERATOR)
		{
			result = Compute(result, tokens[first].strvalue[0], nextValue);
#ifdef DEBUG
			printf("Compute %f\n", result);
#endif
		}
		else if (tokens[first].type == STORE)
		{
			result = *firstToken.dvaluePtr = nextValue;
#ifdef DEBUG
			printf("Store %f\n", result);
#endif
		}
		else
		{
#ifdef DEBUG
			printf("unknown %f\n", result);
#endif
		}
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

double EvaluateNStatements(const vector<Token> & tokens, int first, int last,
	bool & hasReturn, bool & hasIfConditionTrue)
{
	double result;
	int semicolonIdx;


	// 1. no semicolon => easy to evaluate
	semicolonIdx = FindChar(tokens, first, last, ';');
	if (semicolonIdx < 0)
	{
		return Evaluate1Statement(tokens, first, last, &hasReturn, &hasIfConditionTrue);
	}

	// 2. evaluate a multiple statements expression

	// 2.1 special case : 1rst token is return
	result = Evaluate1Statement(tokens, first, semicolonIdx, &hasReturn, &hasIfConditionTrue);
	if (hasReturn)
	{
		return result;
	}

	// 2.2 : evaluate each statement
	while (semicolonIdx >= 0)
	{
		int nextSemicolonIdx = FindChar(tokens, semicolonIdx + 1, last, ';');
		if (nextSemicolonIdx >= 0)
		{
			result = Evaluate1Statement(tokens, semicolonIdx + 1, nextSemicolonIdx, &hasReturn, &hasIfConditionTrue);
			// manage the RETURN token
			if (hasReturn)
			{
				return result;
			}
			semicolonIdx = nextSemicolonIdx;
		}
		else
		{
			//  evaluate the last statement : no need to check the return
			return Evaluate1Statement(tokens, semicolonIdx + 1, last, &hasReturn, &hasIfConditionTrue);
		}
	}

	return result; // => unreachable code
}

double Evaluate1Scope(const vector<Token> & tokens, int first, int last,
	bool & hasReturn, bool & hasIfConditionTrue)
{
	double result;
	int scopeBeginIdx;
	int scopeEndIdx;
	int ifScopedIdx;
	int elseScopedIdx;

#if defined(DEBUG)
	PrintTokensList(tokens, first, last);
#endif

	int size = last - first;
	if (size >= 2 &&
		tokens[first].type == SCOPE &&
		tokens[first].cvalue == '{' &&
		tokens[last - 1].type == SCOPE &&
		tokens[last - 1].cvalue == '}')
	{
		first++;
		last--;
		size -= 2;
	}

	if (size <= 0) return 0.0;

	scopeBeginIdx = FindChar(tokens, first, last, '{');
	while (scopeBeginIdx >= 0)
	{
		assert(GetScopedExpression(tokens, first, last, scopeBeginIdx, scopeBeginIdx, scopeEndIdx, ifScopedIdx, elseScopedIdx));
#if defined(DEBUG)
		cout << "ifScopedIdx = " << ifScopedIdx << endl;
		cout << "elseScopedIdx = " << elseScopedIdx << endl;
#endif
		// 1. evaluate before the scope
		result = EvaluateNStatements(tokens, first,
			ifScopedIdx >= 0 || elseScopedIdx >= 0? std::max(ifScopedIdx, elseScopedIdx) : scopeBeginIdx,
			hasReturn, hasIfConditionTrue);
		if (hasReturn) return result;

		if (ifScopedIdx >= 0)
		{
			int ifConditionBegin;
			int ifConditionEnd;

			assert(GetParenthesedExpression(tokens, first, last, ifScopedIdx + 1, ifConditionBegin,ifConditionEnd));

			result = Evaluate1Statement(tokens, ifConditionBegin, ifConditionEnd);
			hasIfConditionTrue = result != 0.0;
			if (hasIfConditionTrue)
			{
				result = Evaluate1Scope(tokens, scopeBeginIdx, scopeEndIdx, hasReturn, hasIfConditionTrue);
				if (hasReturn) return result;
			}
		}
		else if (elseScopedIdx >= 0 && !hasIfConditionTrue)
		{
			result = Evaluate1Scope(tokens, scopeBeginIdx, scopeEndIdx, hasReturn, hasIfConditionTrue);
			if (hasReturn) return result;
		}
		else
		{
			// 2. evaluate the scope
			result = Evaluate1Scope(tokens, scopeBeginIdx, scopeEndIdx, hasReturn, hasIfConditionTrue);
			if (hasReturn) return result;
		}

		// 3. evaluate after the scope
		first = scopeEndIdx;
		scopeBeginIdx = FindChar(tokens, first, last, '{');
	}

	result = EvaluateNStatements(tokens, first, last, hasReturn, hasIfConditionTrue);
	return result;
}

double Evaluate(const vector<Token> & tokens, int first, int last)
{
	bool hasReturn = false;
	bool hasIfConditionTrue = false;

	return Evaluate1Scope(tokens, first, last, hasReturn, hasIfConditionTrue);
}

bool GetGenericExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx,
	enum TokenType token_type, char token_cvalue_begin, char token_cvalue_end)
{
#if defined(DEBUG)
	cout << "GetGenericExpression : " << endl;
	cout << "idx = " << idx << endl;
	cout << "first = " << first << endl;
	cout << "last = " << last << endl;
#endif
	if (idx < first)
	{
		return false;
	}

	if (idx >= last)
	{
		return false;
	}

	if (tokens[idx].type == token_type)
	{
		int depth = 0;
		if (tokens[idx].cvalue == token_cvalue_begin)
		{
			firstIdx = idx;
			for(;idx < last;idx++)
			{
				if (tokens[idx].type == token_type)
				{
					if (tokens[idx].cvalue == token_cvalue_begin)
					{
						depth++;
					}
					else if (tokens[idx].cvalue == token_cvalue_end)
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
		else if (tokens[idx].cvalue == token_cvalue_end)
		{
			lastIdx = idx + 1;
			for(;idx >= first;idx--)
			{
				if (tokens[idx].type == token_type)
				{
					if (tokens[idx].cvalue == token_cvalue_begin)
					{
						depth++;
						if (depth == 0)
						{
							firstIdx = idx;
							return true; // found
						}

					}
					else if (tokens[idx].cvalue == token_cvalue_end)
					{
						depth--;
					}
				}
			}
		}
	}

	return false; // not found
}

bool GetParenthesedExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx)
{
	return GetGenericExpression(tokens, first, last, idx, firstIdx,
		lastIdx, PARENTHESIS, '(', ')');
}

bool GetScopedExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx)
{
	return GetGenericExpression(tokens, first, last, idx, firstIdx,
		lastIdx, SCOPE, '{', '}');
}

bool GetScopedExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx,
	int & ifScopedIdx, int &elseScopedIdx)
{
	bool ret = GetScopedExpression(tokens, first, last, idx, firstIdx, lastIdx);
#if defined(DEBUG)
	cout << "GetScopedExpression :" << endl;
	cout << "ret = " << ret << endl;
	cout << "idx = " << idx << endl;
	cout << "first = " << first << endl;
	cout << "last = " << last << endl;
	cout << "firstIdx = " << firstIdx << endl;
	cout << "lastIdx = " << lastIdx << endl;
#endif
	if (ret)
	{
		if (firstIdx -1 >= first)
		{
			// IF () {scope}
			if (tokens[firstIdx - 1].type == PARENTHESIS && tokens[firstIdx - 1].cvalue == ')')
			{
				int ifConditionBeginIdx;
				int ifConditionEndIdx;
				if (GetParenthesedExpression(tokens, first, last, firstIdx - 1, ifConditionBeginIdx, ifConditionEndIdx))
				{
					if (ifConditionBeginIdx - 1 >= first && tokens[ifConditionBeginIdx -1].type == IF)
					{
						ifScopedIdx = ifConditionBeginIdx - 1;
					}
					else
					{
						ifScopedIdx = -1;
					}
					elseScopedIdx = -1;
					return true;
				}
			}
			// ELSE {scope}
			else if (tokens[firstIdx -1].type == ELSE)
			{
				ifScopedIdx = -1;
				elseScopedIdx = firstIdx - 1;
				return true;
			}
		}
	}
	ifScopedIdx = -1;
	elseScopedIdx = -1;
	return ret;
}

bool GetFunctionExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx)
{
	// [TODO] need to refactor with GetGenericExpression(...)
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

void Priorize(vector<Token> & tokens, int &first, int & last, char op1, char op2)
{
	//PriorizeFunctions(tokens, first, last);
	bool changed = true;
	int op1Idx = FindOperator(tokens, first, last, op1);
	int op2Idx = FindOperator(tokens, first, last, op2);
	int opIdx = (op1Idx != -1 && op2Idx != -1 ? std::min(op1Idx, op2Idx): std::max(op1Idx, op2Idx));
	while (opIdx != -1 && changed)
	{
		changed = false;
		const Token & tokenBeforeOperator = tokens[opIdx - 1];
		const Token & tokenAfterOperator = tokens[opIdx + 1];
		if ((tokenBeforeOperator.type == NUMBER || tokenBeforeOperator.type == VARIABLE_NAME)
		&& (tokenAfterOperator.type == NUMBER || tokenAfterOperator.type == VARIABLE_NAME))
		{
			tokens.insert(tokens.begin() + opIdx - 1, Token('('));
			tokens.insert(tokens.begin() + opIdx + 3, Token(')'));
			last += 2;
			opIdx += 2;
			changed = true;
		}
		else if ((tokenBeforeOperator.type == NUMBER || tokenBeforeOperator.type == VARIABLE_NAME)
			&& tokenAfterOperator.type == PARENTHESIS)
		{
			int expressionFirstIdx;
			int expressionLastIdx;

			if (GetParenthesedExpression(tokens, first, last, opIdx + 1, expressionFirstIdx, expressionLastIdx))
			{
				tokens.insert(tokens.begin() + opIdx - 1, Token('('));
				tokens.insert(tokens.begin() + expressionLastIdx + 1, Token(')'));
				last += 2;
				opIdx += 2;
				changed = true;
			}
		}
		else if (tokenBeforeOperator.type == PARENTHESIS
			&& (tokenAfterOperator.type == NUMBER || tokenAfterOperator.type == VARIABLE_NAME))
		{
			int expressionFirstIdx;
			int expressionLastIdx;

			if (GetParenthesedExpression(tokens, first, last, opIdx - 1, expressionFirstIdx, expressionLastIdx))
			{
				tokens.insert(tokens.begin() + expressionFirstIdx, Token('('));
				tokens.insert(tokens.begin() + opIdx + 3, Token(')'));
				last += 2;
				opIdx += 2;
				changed = true;
			}
		}

		

		op1Idx = FindOperator(tokens, opIdx, last, op1);
		op2Idx = FindOperator(tokens, opIdx, last, op2);
		opIdx = (op1Idx != -1 && op2Idx != -1 ? std::min(op1Idx, op2Idx): std::max(op1Idx, op2Idx));
#if defined(DEBUG)
		cout << "op1Idx = " << op1Idx << endl;
		cout <<endl;
		PrintTokensList(tokens, 0, (int)tokens.size());
#endif
	}
}

void Priorize(vector<Token> & tokens, int first, int last)
{
	PriorizeFunctions(tokens, first, last);
	Priorize(tokens, first, last, '*', '/');
	Priorize(tokens, first, last, '+', '-');
	Priorize(tokens, first, last, '=', '=');
	Priorize(tokens, first, last, '&', '&');
}

void PriorizeFunctions(vector<Token> & tokens, int & first, int & last)
{
	int idx = FindChar(tokens, first, last, 'F');
	while (idx >= first)
	{
		int expressionFirstIdx;
		int expressionLastIdx;

		if (GetFunctionExpression(tokens, first, last, idx, expressionFirstIdx, expressionLastIdx))
		{
#if defined(DEBUG)
			PrintTokensList(tokens, 0, (int)tokens.size());
#endif

#if defined(DEBUG)
			cout << "[PriorizeFunctions] expressionFirstIdx = "<< expressionFirstIdx <<endl;
			cout << "[PriorizeFunctions] expressionLastIdx = "<< expressionLastIdx <<endl;
			cout << "A";	tokens.insert(tokens.begin() + expressionFirstIdx, Token('('));
			cout << "B";	tokens.insert(tokens.begin() + expressionLastIdx + 1, Token(')'));
			cout << "C";	idx = expressionLastIdx;
#endif
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

double Evaluate(const string & str, void * handle = NULL)
{
	vector<Token> tokens;
	Tokenize(tokens, str);
	if (Check(tokens, 0, tokens.size()))
	{
		UpdateVariablesAddr(handle, tokens, 0, tokens.size());
		Priorize(tokens, 0, tokens.size());
		return Evaluate(tokens, 0, tokens.size());
	}
#if USE_STL
	cout << "error : " << str << endl;
#else
	printf("error %s\n", str.c_str());
#endif
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

	// assertion for returns check
	assert(123.0 == Evaluate("return 123"));
	assert(pi == Evaluate("return pi"));
	assert(cos(1.0) == Evaluate("return cos(1)"));
	assert(33.0 == Evaluate("return max(22,33)"));
	assert(cos(0.25 * pi) == Evaluate("return cos(0.25 * pi)"));
	assert(11.0 + 22.0 * 2.0 + 5.0 == Evaluate("return 11 + 22 * 2 + 5"));
	assert(11.0 + 22.0 * (2.0 + 5.0) == Evaluate("return (11 + 22 * (2 + 5))"));

	// assertion for multiple statements check
	assert(111.0 == Evaluate("return 111; return 222; return 333"));
	assert(222.0 == Evaluate("111; return 222;333"));
	assert(333.0 == Evaluate("111; 222; return 333"));

	// assertion for boolean operations checks
	assert(1 && 0 == Evaluate("1 && 0"));
	assert(1 || 0 == Evaluate("1 || 0"));
	assert(1 + 1 && 0 + 1 == Evaluate("1 + 1 && 0 + 1"));
	assert(0 + 1 || 0 + 1 == Evaluate("0 + 1 || 0 + 1"));
	assert(1 || 1 && 0 == Evaluate("1 || 1 && 0"));
	assert((1 == 2 && 2 == 1) == Evaluate("1 == 2 && 2 == 1"));
	assert((1 + 1 && 2 == 2) == Evaluate("1 + 1 && 2 == 2"));

	// assertion for simple/direct if checks
	assert(22 == Evaluate("if (1) return 22"));
	assert(22 == Evaluate("if (1) return 22;"));
	assert(22 == Evaluate("if (1) return 22; return 33"));
	assert(22 == Evaluate("if (1) return 22; return 33;"));
	assert(33 == Evaluate("if (0) return 22; return 33"));
	assert(33 == Evaluate("if (0) return 22; return 33;"));

	// assertion for if+simple condition checks
	assert(33 == Evaluate("if (0 && 0) return 22; return 33;"));
	assert(33 == Evaluate("if (1 && 0) return 22; return 33;"));
	assert(33 == Evaluate("if (0 && 1) return 22; return 33;"));
	assert(22 == Evaluate("if (1 && 1) return 22; return 33;"));
	assert(33 == Evaluate("if (0 || 0) return 22; return 33;"));
	assert(22 == Evaluate("if (1 || 0) return 22; return 33;"));
	assert(22 == Evaluate("if (0 || 1) return 22; return 33;"));
	assert(22 == Evaluate("if (1 || 1) return 22; return 33;"));

	// assertion for if+complex condition checks
	assert(99 == Evaluate("if (pi == 111) return 88; return 99;"));
	assert(88 == Evaluate("if (pi == pi && 1 == 1) return 88; return 99;"));
	assert(3 * pi == Evaluate("if (pi == 111 || 1 == 1) return 3 * pi; return 5 * pi;"));

	// assertion for if/else checks
	assert(111 == Evaluate("if (1) return 111; else return 222; return 333;"));
	assert(222 == Evaluate("if (0) return 111; else return 222; return 333;"));

	// assertion for double-if checks
	assert(111 == Evaluate("if (1) if (1) return 111; return 555;"));
	assert(555 == Evaluate("if (1) if (0) return 111; return 555;"));
	assert(555 == Evaluate("if (0) if (1) return 111; return 555;"));
	assert(555 == Evaluate("if (0) if (0) return 111; return 555;"));

	// assertion for triple-if checks
	assert(555 == Evaluate("if (0) if (0) if (0) return 111; return 555;"));
	assert(555 == Evaluate("if (0) if (0) if (1) return 111; return 555;"));
	assert(555 == Evaluate("if (0) if (1) if (0) return 111; return 555;"));
	assert(555 == Evaluate("if (0) if (1) if (1) return 111; return 555;"));
	assert(555 == Evaluate("if (1) if (0) if (0) return 111; return 555;"));
	assert(555 == Evaluate("if (1) if (0) if (1) return 111; return 555;"));
	assert(555 == Evaluate("if (1) if (1) if (0) return 111; return 555;"));
	assert(111 == Evaluate("if (1) if (1) if (1) return 111; return 555;"));

	// assertion for double-if-else checks
	assert(111 == Evaluate("if (1) if (1) return 111; else return 555; return 777;"));
	assert(555 == Evaluate("if (1) if (0) return 111; else return 555; return 777;"));
	assert(555 == Evaluate("if (0) if (1) return 111; else return 555; return 777;"));
	assert(555 == Evaluate("if (0) if (0) return 111; else return 555; return 777;"));

	// assertion for triple-if-else checks
	assert(555 == Evaluate("if (0) if (0) if (0) return 111; else return 555; return 777;"));
	assert(555 == Evaluate("if (0) if (0) if (1) return 111; else return 555; return 777;"));
	assert(555 == Evaluate("if (0) if (1) if (0) return 111; else return 555; return 777;"));
	assert(555 == Evaluate("if (0) if (1) if (1) return 111; else return 555; return 777;"));
	assert(555 == Evaluate("if (1) if (0) if (0) return 111; else return 555; return 777;"));
	assert(555 == Evaluate("if (1) if (0) if (1) return 111; else return 555; return 777;"));
	assert(555 == Evaluate("if (1) if (1) if (0) return 111; else return 555; return 777;"));
	assert(111 == Evaluate("if (1) if (1) if (1) return 111; else return 555; return 777;"));

	// assertion for scope checks
	assert(99 == Evaluate("{{5+5;} return 99; 9+9;} return 123;"));

	// assertion for if/else+scope checks
	assert(789 == Evaluate("if (1) {return 789;}"));
	assert(999 == Evaluate("if (0) {return 789;} else {return 999;}"));
	assert(123 == Evaluate("if (1) return 123; else {return 555;}"));
	assert(123 == Evaluate("if (1) {return 123;} else return 555;"));
	assert(555 == Evaluate("if (0) return 123; else {return 555;}"));
	assert(555 == Evaluate("if (0) {return 123;} else return 555;"));

	// assertion for multiple-"if/else+scope"
	assert(pi == Evaluate("if (1) { if (1) {return pi;} }"));
	assert(222 == Evaluate("if (1){ if (0) return 111;else {return 222;}}"));

	// assertions for the crc32
	assert(0 == ComputeCRC32("", 0));
	assert(0 == ComputeCRC32(NULL, 0));
	assert(0xcbf43926 == ComputeCRC32("123456789", 9));

	while (true)
	{
		string in;
		vector<Token> tokens;
#if USE_STL
		cout << "expr ?" << endl;
#else
		printf("expr ?\n");
#endif

		getline(cin, in);	// *fix: use "getline(cin, in)" instead of "cin >> in"
					// cuz cin split the str with space

		double result = Evaluate(in);
#if USE_STL
		cout << "result = "<<  result <<endl;
#else
		printf("result = %f\n", result);
#endif
	}

	return 0;
}
