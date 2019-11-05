#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>

/*
 * A simple C-like interpreter with :
 *	- 4 binary operations *, /, +, - and parenthesis ( and ).
 *	- 2 unary operations : neg(-) and not(!).
 *	- 2 binary boolean operations && and ||.
 *	- 1 comparaison operation ==.
 *	- 1 constante : pi.
 *	- 3 variables : abc (double), counter (int) and btest (boolean)
 *	- 2 functions : max(double, double) and cos(double).
 *	- 'return' keyword management.
 *	- 'if (<condition>) <statement>;' management.
 *	- 'if (<condition>) <statementA>; else <statementB>;' management.
 * It works fine.
 *
 * ps : I don't use yacc or lex but it can help you if you want to dev a compiler or a calculator
 *
 * [TODO] => It's only for the tutorial. It's not for advanced developers and it's obviously not yet highly optimized ^^
 *	- It uses my old code. It needs more OOP.
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

	vector(const vector<T> & other)
	{
		if (other.m_data && other.m_capacity)
		{
			m_size = other.m_size;
			m_capacity = other.m_capacity;
			m_data = new T[m_capacity];
			memcpy(m_data, other.m_data, m_size * sizeof(T));
		}
		else
		{
			m_data = NULL;
			m_size = 0;
			m_capacity = 0;
		}
	}

	virtual ~vector()
	{
		if (m_data)
		{
			delete [] m_data;
			m_data = NULL;
		}
		m_size = 0;
		m_capacity = 0;
	}

	size_t size() const { return m_size;}
	bool empty() const { return m_size == 0;}
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

// -------------------- VARIABLES TYPE (TEMP CODE) ---------------------------

enum VariableType { VOID, DOUBLE, FLOAT, INT, BOOLEAN };

double GetVariableValue(const char * variableName, void *variableAddr, VariableType variableType);
unsigned int ComputeCRC32(const void * buffer, size_t len, unsigned int crc = 0xffffffff); // used for STRING

// ----------------------- EVALUATE PROTOTYPES -------------------------------

class string;
struct ScriptEngineContext;

double Evaluate(const string & str, struct ScriptEngineContext * ctx = NULL);

// -------------------------- SCRIPT ENGINE ---------------------------------

typedef void* (*GetVariablePtrCallback) (void *handle, const char * variableName, VariableType & type);
typedef double (*GetConstanteValueCallback) (void *handle, const char * constanteName, bool & found);
typedef void* (*GetFunctionAddrCallback) (const char* functionName, size_t & nbParams);

static void* emptyGetVariablePtrCallback(void *handle, const char * variableName, VariableType & type)
{
	type = VOID;
	return NULL;
}

double emptyGetConstanteValueCallback(void *handle, const char * constanteName, bool & found)
{
	found = false;
	return 0.0;
}

void* emptyGetFunctionAddrCallback(const char* functionName, size_t & nbParams)
{
	nbParams = 0;
	return NULL;
}

struct ScriptEngineContext
{
	ScriptEngineContext()
	{
		pHandle = NULL;
		pGetVariablePtrCallback = NULL;
		pGetConstanteValueCallback = NULL;
		pGetFunctionAddrCallback = NULL;
	}

	void * pHandle;
	GetVariablePtrCallback pGetVariablePtrCallback;
	GetConstanteValueCallback pGetConstanteValueCallback;
	GetFunctionAddrCallback pGetFunctionAddrCallback;
};

class ScriptEngine
{
public:
	ScriptEngine(ScriptEngineContext * ctx = NULL) : pCtx(ctx) {}

	double Evaluate(const string & str)
	{
		return ::Evaluate(str, pCtx);
	}

private:
	ScriptEngineContext * pCtx;
};


// ------------------------------- TOKEN -------------------------------------


enum TokenType
{
	NONE, NUMBER, STRING, OPERATOR, UNARY_OPERATOR, PARENTHESIS, COMMA,
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
		Clear();
	}

	Token(char c, char c2 = '\0')
	{
		Clear();

		if (c == '(' || c == ')')
		{
			type = PARENTHESIS;
			cvalue = c;
		}
		else if (c == '+' || c == '-' || c == '*' || c == '/'
			|| (c == '&' && c2 == '&')
			|| (c == '|' && c2 == '|')
			|| (c == '=' && c2 == '=')
			|| (c == '!' && c2 == '='))
		{
			type = OPERATOR;
			cvalue = 'O';
		}
		else if (c == '!' && c2 == '\0')
		{
			type = UNARY_OPERATOR;
			cvalue = 'U';
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
	}

	Token(const char * str)
	{
		assert(str != NULL);

		Clear();
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
		else if	((c == '&' && c2 == '&')	// AND
			|| (c == '|' && c2 == '|')	// OR
			|| (c == '=' && c2 == '=')	// EQUALS
			|| (c == '!' && c2 == '=')	// NOT EQUALS
			|| (c == '!' && c2 == '\0'))	// NOT
		{
			*this = Token(c, c2);
			strvalue[0] = c;
			strvalue[1] = c2;
			strvalue[2] = '\0';
		}
		else if (c == '=')			// STORE
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
			}
			else if (strcmp(str, "return") == 0)
			{
				type = RETURN;
				cvalue = 'R';
			}
			else if (strcmp(str, "else") == 0)
			{
				type = ELSE;
				cvalue = 'E';
			}
			else
			{
				type = NAME; // NAME =  VARIABLE_NAME or FUNCTION_NAME
				cvalue = 'N';
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
		if (type == VARIABLE_NAME && ptrvalue != NULL)
		{
			return GetVariableValue(strvalue, ptrvalue, variableType);
		}
		else
		{
			return dvalue;
		}
	}

	char strvalue[NAME_NB_CHARS_MAX + 1];
	char cvalue;
	double dvalue;
	void* ptrvalue;
	bool isConstante;
	VariableType variableType;
	void* functionAddr;
	size_t nbParams;

private:
	void Clear()
	{
		type = NONE;
		strvalue[0] ='\0';
		cvalue = '?';
		dvalue = 0.0;
		ptrvalue = NULL;
		isConstante = false;
		variableType = VOID;
		functionAddr = NULL;
		nbParams = 0;
	}
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
			Token t(c);
			if (c == '-' && (tokens.empty() || tokens[tokens.size() - 1].cvalue != 'N'))
			{
				// convert to unary operator
				t.type = UNARY_OPERATOR;
				t.cvalue = 'U';
			}
			tokens.push_back(t);
		}
		else if ((c == '&' && c2 == '&') || (c == '|' && c2 == '|')
			|| (c == '=' && c2 == '=') || (c == '!' && c2 == '='))
		{
			tokens.push_back(Token(c, c2));
			s++;
		}
		else if (c == '!') // check the boolean NOT (UNARY OPERATOR)
		{
			static const Token NOT('!');
			tokens.push_back(NOT);
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
	if (!tokens.empty())
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
					currentToken.functionAddr = NULL;
					//currentToken.functionAddr = GetFunctionAddr(currentToken.strvalue, currentToken.nbParams);
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

		// Step 3.1
		// '++' is forbiden
		// '--' is forbiden
		bool hasIncrementError = false;
		bool hasDecrementError = false;
		for (size_t i = 0; i < tokens.size() - 1; i++)
		{
			const Token & cur = tokens[i];
			const Token & next = tokens[i + 1];

			if ((cur.type == OPERATOR && cur.strvalue[0] == '+')
				&& (next.type == OPERATOR && next.strvalue[0] == '+'))
			{
				hasIncrementError = true;
			}
			else if (((cur.type == OPERATOR || cur.type == UNARY_OPERATOR) && cur.strvalue[0] == '-')
				&& ((next.type == OPERATOR || next.type == UNARY_OPERATOR) && next.strvalue[0] == '-'))
			{
				hasDecrementError = true;
			}
		}

		if (hasIncrementError)
		{
			printf("error : '++' is forbiden\n");
		}
		if (hasDecrementError)
		{
			printf("error : '--' is forbiden\n");
		}
		if (hasIncrementError || hasDecrementError)
		{
			return;
		}


		// Step 3.2
		// Replace NEG(-) by '0 - '
		// Replace NOT(!) by '0 == '
		bool changed = true; // to avoid infinite loop with '5!'
		bool restart = true;
		size_t restartIdx = 0;
		while (restart && changed)
		{
			restart = false;
			changed = false;
			for (size_t i = restartIdx; i < tokens.size() - 1; i++)
			{
				Token & cur = tokens[i];
				Token & next = tokens[i + 1];
				if (cur.type == UNARY_OPERATOR && (cur.strvalue[0] == '-'  || cur.strvalue[0] == '!'))
				{
					static const Token ZERO("0");
					static const Token SUB('-');
					static const Token EQUALS('=', '=');
					static const Token PARENTHESIS_OPEN('(');
					static const Token PARENTHESIS_CLOSE(')');

					const Token & REPLACE = cur.strvalue[0] == '-' ? SUB : EQUALS;

					if (next.cvalue == 'N')
					{
						changed = true;
						cur = REPLACE;
						tokens.insert(tokens.begin() + i, /* Token("(") */ PARENTHESIS_OPEN);
						tokens.insert(tokens.begin() + i + 1, /* Token("0")*/ ZERO);
						tokens.insert(tokens.begin() + i + 4, /* Token(")") */ PARENTHESIS_CLOSE);
					}
					else if (next.cvalue == '(')
					{
						int firstIdx;
						int lastIdx;

						if (GetParenthesedExpression(tokens, 0, tokens.size(), i + 1,
							firstIdx, lastIdx))
						{
							changed = true;
							cur = REPLACE;
							tokens.insert(tokens.begin() + i, /* Token("(") */ PARENTHESIS_OPEN);
							tokens.insert(tokens.begin() + i + 1, /* Token("0")*/ ZERO);
							tokens.insert(tokens.begin() + lastIdx + 2, /* Token(")") */ PARENTHESIS_CLOSE);
						}
					}
					else if (next.type == FUNCTION_NAME)
					{
						int firstIdx;
						int lastIdx;

						if (GetFunctionExpression(tokens, 0, tokens.size(), i + 1,
							firstIdx, lastIdx))
						{
							changed = true;
							cur = REPLACE;
							tokens.insert(tokens.begin() + i, /* Token("(") */ PARENTHESIS_OPEN);
							tokens.insert(tokens.begin() + i + 1, /* Token("0")*/ ZERO);
							tokens.insert(tokens.begin() + lastIdx + 2, /* Token(")") */ PARENTHESIS_CLOSE);
							i--;
						}
					}
					else if (next.type == UNARY_OPERATOR)
					{
						restart = true;
						restartIdx = (i == 0 ? 0: i - 1); // [fix] for '!!!!5' or more
					}
					else if (restart) // [optimisation] restart asap to avoid too much loops
					{
						break;
					}
				}
			}
		} // end of the while

		const Token & last = tokens[tokens.size() - 1];
		if (last.type == UNARY_OPERATOR)
		{
			printf("error : UNARY OPERATOR for the last token is forbiden : %s\n", last.strvalue);
			return;
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
	const Token & token = tokens[idx];

	return token.type == NUMBER || token.type == VARIABLE_NAME || token.type == STRING;
}

bool Check2(const vector<Token> & tokens, int idx)
{
	const Token & token0 = tokens[idx];
	const Token & token1 = tokens[idx + 1];

	return	(
			(token0.type == RETURN || token0.type == ELSE)
			&& (token1.type == NUMBER || token1.type == VARIABLE_NAME || token1.type == STRING)
		)
		||
		(
			(token0.type == UNARY_OPERATOR && token0.strvalue[0] == '!')
			&& (token1.type == NUMBER || token1.type == VARIABLE_NAME)
		)
		||
			(token0.type == SCOPE && token0.cvalue == '{'
			&& token1.type == SCOPE && token1.cvalue == '}');
}

bool Check3(const vector<Token> & tokens, int idx)
{
	const char * validcombo[] = {"(N)", "(S)", "NON", "F[]", "RN;", "RS;","ERN",  "{N}", "{S}", "{;}", NULL};

	const Token & token0 = tokens[idx];
	const Token & token1 = tokens[idx + 1];
	const Token & token2 = tokens[idx + 2];

	for (int i = 0; validcombo[i]; i++)
	{
		if (token0.cvalue == validcombo[i][0]
			&& token1.cvalue == validcombo[i][1]
			&& token2.cvalue == validcombo[i][2])
		{
			return true;
		}
	}

	// special case : 'Variable = Number' or 'Variable = Variable' or 'Variable = ('
	if (token0.type == VARIABLE_NAME
		&& token1.cvalue == '='
		&& token2.cvalue == 'N')
	{
		return true;
	}

	// special case : !! Number
	if ((token0.type == UNARY_OPERATOR && token0.strvalue[0] == '!')
		&& (token1.type == UNARY_OPERATOR && token1.strvalue[0] == '!')
		&& token2.cvalue == 'N')
	{
		return true;
	}

	return false;
}

bool CheckCombo(const vector<Token> & tokens, int idx0, int idx1)
{
	const char * validcombo[] = {"((", "))",
		"(N", "N)", "NO", "ON", "O(", ")O",
		"OU", "UN", "UF", "U(", "(U", "RU", "EU", "=U",
		"F[", "[F", ",F", "],", ")]", "]]", "[(", "([", "])", "[]", ",(", "),",
		"[N", "N]", "N,", ",N", "OF", "]O",
		"I(", ")I", "RN", "R(", ")R", "RF", ";R",
		";E", "EN", "ER", "E(", "EF", "E;",
		"N;", ";N", ");", ";(", ";I", ";F", "];",
		"{N", "{F", "{(", "{{", "{R", "{I", "){", "{;", "};", ";;",
		"{}", ";}", "}}", "}N", "}F", "}R", "}I", "}E", "E{",
		"=N", "=(", "=F",
		"S,", ",S", "(S", "S)","[S", "S]", "S;", ";S", "RS", NULL};

	const Token & token0 = tokens[idx0];
	const Token & token1 = tokens[idx1];

	for (int i = 0; validcombo[i]; i++)
	{
		if (token0.cvalue == validcombo[i][0]
			&& token1.cvalue == validcombo[i][1])
		{
			return true;
		}
	}

	// special case : only 'Variable ='  is accepted
	if (token0.type == VARIABLE_NAME
		&& token1.cvalue == '=')
	{
		return true;
	}

	// special case : !!
	if ((token0.type == UNARY_OPERATOR && token0.strvalue[0] == '!')
		&& (token1.type == UNARY_OPERATOR && token1.strvalue[0] == '!'))
	{
		return true;
	}

#if USE_STL
	cout << "[CheckCombo] wrong combo " << token0.cvalue << token1.cvalue << endl;
#else
	printf("[CheckCombo] wrong combo %c%c\n", token0.cvalue, token1.cvalue);
#endif
	return false;
}

bool CheckParenthesis(const vector<Token> & tokens, int first, int last)
{
	int depth = 0;

	// check classic parenthesis
	for (int i = first; i < last; i++)
	{
		const Token & token = tokens[i];

		if (token.cvalue == '(')
		{
			depth++;
		}

		if (token.cvalue == ')')
		{
			depth--;
			if (depth < 0)
				return false;
		}
	}

	if (depth != 0)
	{
		return false;
	}

	// check function start / end
	for (int i = first; i < last; i++)
	{
		const Token & token = tokens[i];

		if (token.cvalue == '[')
		{
			depth++;
		}

		if (token.cvalue == ']')
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
	bool result = true;

	for (int i = first; i < last; i++)
	{
		const Token & currentToken = tokens[i];
		if (currentToken.type == VARIABLE_NAME)
		{
			bool found = false;

			if (currentToken.ptrvalue)
			{
				found = true;
			}
			else if (currentToken.isConstante)
			{
				found = true;
			}

			if (!found)
			{
#if USE_STL
				cout << "error : variable not found : " << currentToken.strvalue << endl;
#else
				printf("error : variable not found : %s\n", currentToken.strvalue);
#endif
				result = false;
			}
		}
	}
	return result;
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
			// check function addr
			if (currentToken.functionAddr == NULL)
			{
#if USE_STL
				cout << "error : function not found : " << currentToken.strvalue << endl;
#else
				printf("error : function not found : %s\n", currentToken.strvalue);
#endif
				return false;
			}

			// check function params (count)
			if (currentToken.nbParams != currentToken.dvalue && currentToken.nbParams != (size_t)-1)
			{
#if USE_STL
				cout << "error : invalid params in function : " << currentToken.strvalue << endl;
#else
				printf("error : invalid params in function : %s\n", currentToken.strvalue);
#endif
				return false;
			}

			return true;
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
		case '!': result = double(a != b); break;
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
		const Token & token0 = tokens[first];
		const Token & token1 = tokens[first + 1];

		if (token0.type == OPERATOR)
		{
			double a = val;
			double b = token1.GetDoubleValue();
			result = Compute(a, token0.strvalue[0], b);
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
		const Token & token0 = tokens[first];
		const Token & token1 = tokens[first + 1];
		const Token & token2 = tokens[first + 2];

		// NON
		if (token1.type == OPERATOR)
		{
			double a = token0.GetDoubleValue();
			double b = token2.GetDoubleValue();
			result = Compute(a, token1.strvalue[0], b);
			/*switch(tokens[first + 1].strvalue[0])
			{
				case '+': result = a + b; break;
				case '-': result = a - b; break;
				case '*': result = a * b; break;
				case '/': result = a / b; break;
			}*/
			return true;
		}
		// (N) or (S)
		if (token1.type == NUMBER || token1.type == VARIABLE_NAME || token1.type == STRING)
		{
			result = token1.GetDoubleValue();
			return true;
		}
	}
	return false;
}

unsigned int ComputeCRC32(const void * buffer, size_t len, unsigned int crc /*= 0xffffffff*/)
{
	static unsigned int crc32_table[256];
	static bool initialized = false;
	unsigned char *ptr;

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

	ptr = (unsigned char *)buffer;
	while(len--)
	{
		crc = (crc >> 8) ^ crc32_table[(crc & 0xff) ^ *ptr++];
	}

	return ~crc;
}

// -------------------------------- MISC -------------------------------------

typedef bool (*TokenEqualsCallback)(const Token & a, const Token & b);

bool TokenCharEquals(const Token & a, const Token & b)
{
	return a.cvalue == b.strvalue[0] || a.cvalue == b.cvalue;
}

bool TokenOperatorEquals(const Token & a, const Token & b)
{
	return b.type == OPERATOR && TokenCharEquals(a, b);
}

bool TokenTypeEquals(const Token & a, const Token & b)
{
	return a.type == b.type;
}

int FindGeneric(const vector<Token> & tokens, int first, int last, const Token & token, TokenEqualsCallback equals,
	int dir = 1, bool checkDepth = false)
{
	int depth = 0;

	if (dir > 0)
	{
		for (int idx = first; idx < last; idx++)
		{
			const Token & currentToken = tokens[idx];

			if (checkDepth)
			{
				if (currentToken.cvalue == '(' || currentToken.cvalue == '[')
				{
					depth++;
				}

				if (currentToken.cvalue == ')' || currentToken.cvalue == ']')
				{
					depth--;
				}
			}

			if (depth == 0 && equals(token, currentToken))
			{
				return idx;
			}
		}
	}
	else if (dir < 0)
	{
		for (int idx = last-1; idx >= first; idx--)
		{
			const Token & currentToken = tokens[idx];

			if (checkDepth)
			{
				if (currentToken.cvalue == '(' || currentToken.cvalue == '[')
				{
					depth++;
				}

				if (currentToken.cvalue == ')' || currentToken.cvalue == ']')
				{
					depth--;
				}
			}

			if (depth == 0 && equals(token, currentToken))
			{
				return idx;
			}
		}
	}

	return -1;
}

int FindChar(const vector<Token> & tokens, int first, int last, char c, int dir = 1, bool checkDepth = false)
{
	static Token token;
	token.cvalue = c;

	return FindGeneric(tokens, first, last, token, TokenCharEquals, dir, checkDepth);
}

int FindOperator(const vector<Token> & tokens, int first, int last, char c, int dir = 1, bool checkDepth = false)
{
	static Token token('+');
	token.cvalue = c;

	return FindGeneric(tokens, first, last, token, TokenOperatorEquals, dir, checkDepth);
}

int FindToken(const vector<Token> & tokens, int first, int last, TokenType type, int dir = 1, bool checkDepth = false)
{
	static Token token;
	token.type = type;

	return FindGeneric(tokens, first, last, token, TokenTypeEquals, dir, checkDepth);
}


// ------------------------------ VARIABLES ----------------------------------

bool SetVariableValue(const char * variableName, void * variableAddress, double value, VariableType variableType)
{
	if (variableAddress)
	{
		assert(variableName != NULL);
		assert(variableType != VOID);

		switch(variableType)
		{
			case DOUBLE: *((double*)variableAddress) = value; return true;
			case FLOAT: *((float*)variableAddress) = value; return true;
			case INT: *((int*)variableAddress) = value; return true;
			case BOOLEAN: *((bool*)variableAddress) = (value != 0.0); return true;
			default : assert(0);
		}
	}
	return false;
}

double GetVariableValue(const char * variableName, void *variableAddr, VariableType variableType)
{
	assert(variableName != NULL);
	assert(variableAddr != NULL);

	switch(variableType)
	{
		case DOUBLE: return *((double*)variableAddr);
		case FLOAT: return *((float*)variableAddr);
		case INT: return *((int*)variableAddr);
		case BOOLEAN: return *((bool*)variableAddr);
		default : assert(0); return 0.0;
	}
}

void UpdateVariablesAddr(struct ScriptEngineContext * ctx, vector<Token> & tokens, int first, int last)
{
	void* handle = NULL;
	GetConstanteValueCallback pGetConstanteValueCallback = NULL;
	GetVariablePtrCallback pGetVariablePtrCallback = NULL;

	if (ctx != NULL)
	{
		handle = ctx->pHandle;
		pGetConstanteValueCallback = ctx->pGetConstanteValueCallback;
		pGetVariablePtrCallback = ctx->pGetVariablePtrCallback;
	}

	if (pGetConstanteValueCallback == NULL)
	{
		pGetConstanteValueCallback = emptyGetConstanteValueCallback;
	}

	if (pGetVariablePtrCallback == NULL)
	{
		pGetVariablePtrCallback = emptyGetVariablePtrCallback;
	}

	for (int i = first; i < last; i++)
	{
		Token & currentToken = tokens[i];
		if (currentToken.type == VARIABLE_NAME)
		{
			// apply relocation : symbole -> addr

			// default values
			currentToken.isConstante = false;
			currentToken.dvalue = 0.0;
			currentToken.ptrvalue = NULL;
			currentToken.variableType = VOID;

			// Get Constante Value
			currentToken.dvalue = pGetConstanteValueCallback(handle, currentToken.strvalue, currentToken.isConstante);

			// Get Variable Ptr
			if (!currentToken.isConstante)
			{
				currentToken.ptrvalue = pGetVariablePtrCallback(handle, currentToken.strvalue, currentToken.variableType);
			}

			// check if the variable or the constante is found
			if (!currentToken.isConstante && !currentToken.ptrvalue)
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

void UpdateFunctionsAddr(struct ScriptEngineContext * ctx, vector<Token> & tokens, int first, int last)
{
	GetFunctionAddrCallback pGetFunctionAddrCallback = NULL;

	if (ctx != NULL)
	{
		pGetFunctionAddrCallback = ctx->pGetFunctionAddrCallback;
	}

	if (pGetFunctionAddrCallback == NULL)
	{
		pGetFunctionAddrCallback = emptyGetFunctionAddrCallback;
	}

	for (int i = first; i < last; i++)
	{
		Token & currentToken = tokens[i];
		if (currentToken.type == FUNCTION_NAME)
		{
			currentToken.functionAddr = pGetFunctionAddrCallback(currentToken.strvalue, currentToken.nbParams);
		}
	}
}

double CallFunction(const Token& function, vector<double> & args)
{
	assert(function.nbParams == args.size());
	double result = 0.0;

	if (function.functionAddr)
	{
typedef double (*function0_t) ();
typedef double (*function1_t) (double);
typedef double (*function2_t) (double, double);
typedef double (*function3_t) (double, double, double);
typedef double (*function4_t) (double, double, double, double);
typedef double (*function5_t) (double, double, double, double, double);
typedef double (*function6_t) (double, double, double, double, double, double);

		switch(function.nbParams)
		{
			case 0:	result = ((function0_t)function.functionAddr)();break;
			case 1:	result = ((function1_t)function.functionAddr)(args[0]);break;
			case 2:	result = ((function2_t)function.functionAddr)(args[0], args[1]);break;
			case 3:	result = ((function3_t)function.functionAddr)(args[0], args[1], args[2]);break;
			case 4:	result = ((function4_t)function.functionAddr)(args[0], args[1], args[2], args[3]);break;
			case 5:	result = ((function5_t)function.functionAddr)(args[0], args[1], args[2], args[3], args[4]);break;
			case 6:	result = ((function6_t)function.functionAddr)(args[0], args[1], args[2], args[3], args[4], args[5]);break;
			default:
#if USE_STL
				cout << "error : Call Function : too much params ("<< function.nbParams <<") : " << function.strvalue <<endl;
#else
				printf("error : Call Function : too much params (%d): %s\n", int(function.nbParams), function.strvalue);
#endif
				assert(0);
		}
	}
	else
	{
#if USE_STL
		cout << "error : Call Function : function not found : " << function.strvalue <<endl;
#else
		printf("error : Call Function : function not found : %s\n", function.strvalue);
#endif
		assert(0);
	}

	return result;
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
		SetVariableValue(variable.strvalue, variable.ptrvalue, result, variable.variableType);

#ifdef DEBUG
		printf("_STORE %f\n", result);
#endif
		return variable.GetDoubleValue(); // useful if variable type is bool or int
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
			SetVariableValue(firstToken.strvalue, firstToken.ptrvalue, nextValue, firstToken.variableType);
			result = firstToken.GetDoubleValue();
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
	enum TokenType token_type1, enum TokenType token_type2, char token_cvalue_begin, char token_cvalue_end)
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

	const Token & currentToken = tokens[idx];
	if (currentToken.type == token_type1 || currentToken.type == token_type2)
	{
		int depth = 0;
		if (currentToken.cvalue == token_cvalue_begin)
		{
			firstIdx = idx;
			for(;idx < last;idx++)
			{
				const Token & currentToken = tokens[idx];
				if (currentToken.type == token_type1 || currentToken.type == token_type2)
				{
					if (currentToken.cvalue == token_cvalue_begin)
					{
						depth++;
					}
					else if (currentToken.cvalue == token_cvalue_end)
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
		else if (currentToken.cvalue == token_cvalue_end)
		{
			lastIdx = idx + 1;
			for(;idx >= first;idx--)
			{
				const Token & currentToken = tokens[idx];
				if (currentToken.type == token_type1 || currentToken.type == token_type2)
				{
					if (currentToken.cvalue == token_cvalue_begin)
					{
						depth++;
						if (depth == 0)
						{
							firstIdx = idx;
							return true; // found
						}

					}
					else if (currentToken.cvalue == token_cvalue_end)
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
		lastIdx, PARENTHESIS, PARENTHESIS, '(', ')');
}

bool GetScopedExpression(const vector<Token> & tokens, int first, int last, int idx, int &firstIdx, int &lastIdx)
{
	return GetGenericExpression(tokens, first, last, idx, firstIdx,
		lastIdx, SCOPE, SCOPE, '{', '}');
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
	if (tokens[idx].type == FUNCTION_NAME)
	{
		if (GetGenericExpression(tokens, first, last, idx + 1, firstIdx, lastIdx,
			FUNCTION_ARGS_BEGIN, FUNCTION_ARGS_END, '[', ']'))
		{
			firstIdx--;
			return firstIdx>= first && tokens[firstIdx].type == FUNCTION_NAME;
		};
	}
	else if (tokens[idx].type == FUNCTION_ARGS_END)
	{
		if (GetGenericExpression(tokens, first, last, idx, firstIdx, lastIdx,
			FUNCTION_ARGS_BEGIN, FUNCTION_ARGS_END, '[', ']'))
		{
			firstIdx--;
			return firstIdx >= first && tokens[firstIdx].type == FUNCTION_NAME;
		};
	}

	return false;
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
	Priorize(tokens, first, last, '=', '!'); // '==' and '!='
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

double Evaluate(const string & str, struct ScriptEngineContext * ctx)
{
	vector<Token> tokens;
	Tokenize(tokens, str);
	UpdateVariablesAddr(ctx, tokens, 0, tokens.size());
	UpdateFunctionsAddr(ctx, tokens, 0, tokens.size());
	if (Check(tokens, 0, tokens.size()))
	{
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

vector<Token> Compile(const string & str, struct ScriptEngineContext * ctx)
{
	vector<Token> tokens;
	Tokenize(tokens, str);
	UpdateVariablesAddr(ctx, tokens, 0, tokens.size());
	UpdateFunctionsAddr(ctx, tokens, 0, tokens.size());
	if (Check(tokens, 0, tokens.size()))
	{
		Priorize(tokens, 0, tokens.size());
		return tokens;
	}
#if USE_STL
	cout << "error : " << str << endl;
#else
	printf("error %s\n", str.c_str());
#endif
	return vector<Token>();
}

double Execute(const vector<Token> & tokens)
{
	return Evaluate(tokens, 0, tokens.size());
}

// -------------------------------- TESTER -----------------------------------

void* myGetVariablePtr(void *handle, const char * variableName, VariableType & type)
{
	static double abc = 0.0;
	static int counter = 0;
	static bool btest = false;

	static struct VariableInfo {const char * name; void* addr; VariableType type;} table [] =
	{
		{"abc", (void*)&abc, DOUBLE},
		{"counter", (void*)&counter, INT},
		{"btest", (void*)&btest, BOOLEAN},
		{NULL, NULL, VOID}
	};


	for (size_t i = 0; table[i].name; i++)
	{
		VariableInfo & info = table[i];
		if (strcmp(variableName, info.name) == 0)
		{
			type = info.type;
			return info.addr;
		}
	}

	type = VOID;
	return NULL;
}

double myGetConstanteValue(void *handle, const char * constanteName, bool & found)
{
	static struct ConstanteInfo {const char * name; double value;} table [] =
	{
		{"pi", pi},
		{NULL, 0.0}
	};

	for (size_t i = 0; table[i].name; i++)
	{
		struct ConstanteInfo & info = table[i];

		if (strcmp(constanteName, info.name) == 0)
		{
			found = true;
			return info.value;
		}

	}

	found = false;
	return 0.0;
}

double myEmptyFunction()
{
	return 1.0;
}

double myMin(double a, double b)
{
	return a < b? a : b;
}

double myMax(double a, double b)
{
	return a > b? a : b;
}

double myCos(double rad)
{
	return cos(rad);
}

void* myGetFunctionAddr(const char* functionName, size_t & nbParams)
{
	assert(functionName != NULL);
	static struct FunctionInfo {const char*name; size_t nbParams; void * addr;} functionTable[] =
	{
		{"print", -1, (void*)&myEmptyFunction},
		{"min", 2, (void*)&myMin},
		{"max", 2, (void*)&myMax},
		{"cos", 1, (void*)&myCos},
		{NULL, 0, NULL}
	};

	for (size_t i = 0; functionTable[i].name; i++)
	{
		struct FunctionInfo & info = functionTable[i];

		if (strcmp(functionName, info.name) == 0)
		{
			nbParams = info.nbParams;
			return info.addr;
		}
	}

	return NULL;
}

// --------------------------------- MAIN ------------------------------------

int main(int argc, char ** argv)
{
	struct ScriptEngineContext ctx;

	// init the context
	ctx.pHandle = NULL;
	ctx.pGetVariablePtrCallback = myGetVariablePtr;
	ctx.pGetConstanteValueCallback = myGetConstanteValue;
	ctx.pGetFunctionAddrCallback = myGetFunctionAddr;

	struct ScriptEngine script(&ctx);

	// assertion for constants check
	assert(42 == script.Evaluate("42"));
	assert(2.5 == script.Evaluate("5/2"));

	// assertion of binary operations check
	assert(99 + 42 == script.Evaluate("99 + 42"));
	assert(99 * 42 == script.Evaluate("99 * 42"));
	assert(11 + 22 * 33 == script.Evaluate("11 + 22 * 33"));
	assert(36.0 + 50.0 * 100.0 / 2.0 * 3.0 == script.Evaluate("36 + 50 * 100 / 2 * 3"));
	assert(99.0 * 56.0 + 25.0 * 37.0 / 3.0 * 5.0 == script.Evaluate("99 * 56 + 25 * 37 / 3 * 5"));

	// assertion for variables check
	assert(pi == script.Evaluate("pi"));
	assert(2.0 * pi == script.Evaluate("2 * pi"));

	// assertion for functions check (simple)
	assert(55.0 == script.Evaluate("max(55,22)"));
	assert(55.0 == script.Evaluate("max(22,55)"));
	assert(1.0 == script.Evaluate("cos(0)"));
	assert(cos(pi) == script.Evaluate("cos(pi)"));
	assert(cos(2 * pi) == script.Evaluate("cos(2 * pi)"));

	// assertion for functions check (multiple functions or "function inception")
	assert(cos(cos(0.5 * pi)) == script.Evaluate("cos(cos(0.5 * pi))"));
	assert(cos(0) + cos(0) + cos(cos(0.5*pi)) == script.Evaluate("cos(0) + cos(0) + cos(cos(0.5*pi))"));
	assert(99.0 == script.Evaluate("max(22,max(99,55))"));
	assert(99.0 == script.Evaluate("max(max(99,55), 22)"));
	assert(44.0 == script.Evaluate("max(max(11,22), max(33,44))"));
	assert(44.0 == script.Evaluate("max(max(33,44), max(11,22))"));
	assert(45.0 == script.Evaluate("max(max(11,22) + 1, max(33,44) + 1)"));
	assert(45.0 == script.Evaluate("max(max(33,44) + 1, max(11,22) + 1)"));

	// assertion for returns check
	assert(123.0 == script.Evaluate("return 123"));
	assert(pi == script.Evaluate("return pi"));
	assert(cos(1.0) == script.Evaluate("return cos(1)"));
	assert(33.0 == script.Evaluate("return max(22,33)"));
	assert(cos(0.25 * pi) == script.Evaluate("return cos(0.25 * pi)"));
	assert(11.0 + 22.0 * 2.0 + 5.0 == script.Evaluate("return 11 + 22 * 2 + 5"));
	assert(11.0 + 22.0 * (2.0 + 5.0) == script.Evaluate("return (11 + 22 * (2 + 5))"));

	// assertion for multiple statements check
	assert(111.0 == script.Evaluate("return 111; return 222; return 333"));
	assert(222.0 == script.Evaluate("111; return 222;333"));
	assert(333.0 == script.Evaluate("111; 222; return 333"));

	// assertion for equality checks
	assert(1.0 == script.Evaluate("2 == 2"));
	assert(1.0 == script.Evaluate("pi == pi"));
	assert(1.0 == script.Evaluate("1 != 2"));
	assert(1.0 == script.Evaluate("pi != 999"));
	assert(0.0 == script.Evaluate("2 != 2"));
	assert(0.0 == script.Evaluate("999 != 999"));
	assert(0.0 == script.Evaluate("pi != pi"));

	// assertion for boolean operations checks
	assert(1 && 0 == script.Evaluate("1 && 0"));
	assert(1 || 0 == script.Evaluate("1 || 0"));
	assert(1 + 1 && 0 + 1 == script.Evaluate("1 + 1 && 0 + 1"));
	assert(0 + 1 || 0 + 1 == script.Evaluate("0 + 1 || 0 + 1"));
	assert(1 || 1 && 0 == script.Evaluate("1 || 1 && 0"));
	assert((1 == 2 && 2 == 1) == script.Evaluate("1 == 2 && 2 == 1"));
	assert((1 + 1 && 2 == 2) == script.Evaluate("1 + 1 && 2 == 2"));

	// assertion for unary operator checks
	assert(-5.0 == script.Evaluate("-5"));
	assert(-10.0 == script.Evaluate("-5-5"));
	assert(25.0 == script.Evaluate("-5*-5"));
	assert(25.0 == script.Evaluate("-5*(-5)"));
	assert(25.0 == script.Evaluate("(-5)*(-5)"));
	assert(25.0 == script.Evaluate("(-5)*-5"));
	assert(-25.0 == script.Evaluate("-5*-(-5)"));
	assert(25.0 == script.Evaluate("-5*-(-(-5))"));
	assert(-5.0 == script.Evaluate("max(-5, -8)"));
	assert(-5.0 == script.Evaluate("max((-5), (-8))"));
	assert(-5.0 == script.Evaluate("max(-5, (-8))"));
	assert(-5.0 == script.Evaluate("max((-5), -8)"));
	assert(-9.0 == script.Evaluate("-max(5, 9)"));
	assert(5.0 == script.Evaluate("-max(-5, -8)"));
	assert(5.0 == script.Evaluate("-max((-5), (-8))"));
	assert(5.0 == script.Evaluate("-max(-5, (-8))"));
	assert(5.0 == script.Evaluate("-max((-5), -8)"));
	assert(1.0 == script.Evaluate("!0"));
	assert(0.0 == script.Evaluate("!1"));
	assert(0.0 == script.Evaluate("!5"));
	assert(1.0 == script.Evaluate("!!5"));
	assert(0.0 == script.Evaluate("!!!5"));
	assert(1.0 == script.Evaluate("!!!!5"));
	assert(0.0 == script.Evaluate("!!!!!5"));
	assert(1.0 == script.Evaluate("!!!!!!5"));
	assert(0.0 == script.Evaluate("!!!!!!!5"));
	assert(1.0 == script.Evaluate("!!!!!!!!5"));
	assert(0.0 == script.Evaluate("!2 == 2"));
	assert(1.0 == script.Evaluate("!2 != 2"));
	assert((!!2==2) == script.Evaluate("!!2==2"));
	assert((!!pi == pi) == script.Evaluate("!!pi == pi"));

	// assertion for simple/direct if checks
	assert(22 == script.Evaluate("if (1) return 22"));
	assert(22 == script.Evaluate("if (1) return 22;"));
	assert(22 == script.Evaluate("if (1) return 22; return 33"));
	assert(22 == script.Evaluate("if (1) return 22; return 33;"));
	assert(33 == script.Evaluate("if (0) return 22; return 33"));
	assert(33 == script.Evaluate("if (0) return 22; return 33;"));

	// assertion for if+simple condition checks
	assert(33 == script.Evaluate("if (0 && 0) return 22; return 33;"));
	assert(33 == script.Evaluate("if (1 && 0) return 22; return 33;"));
	assert(33 == script.Evaluate("if (0 && 1) return 22; return 33;"));
	assert(22 == script.Evaluate("if (1 && 1) return 22; return 33;"));
	assert(33 == script.Evaluate("if (0 || 0) return 22; return 33;"));
	assert(22 == script.Evaluate("if (1 || 0) return 22; return 33;"));
	assert(22 == script.Evaluate("if (0 || 1) return 22; return 33;"));
	assert(22 == script.Evaluate("if (1 || 1) return 22; return 33;"));

	// assertion for if+complex condition checks
	assert(99 == script.Evaluate("if (pi == 111) return 88; return 99;"));
	assert(88 == script.Evaluate("if (pi == pi && 1 == 1) return 88; return 99;"));
	assert(3 * pi == script.Evaluate("if (pi == 111 || 1 == 1) return 3 * pi; return 5 * pi;"));

	// assertion for if/else checks
	assert(111 == script.Evaluate("if (1) return 111; else return 222; return 333;"));
	assert(222 == script.Evaluate("if (0) return 111; else return 222; return 333;"));

	// assertion for double-if checks
	assert(111 == script.Evaluate("if (1) if (1) return 111; return 555;"));
	assert(555 == script.Evaluate("if (1) if (0) return 111; return 555;"));
	assert(555 == script.Evaluate("if (0) if (1) return 111; return 555;"));
	assert(555 == script.Evaluate("if (0) if (0) return 111; return 555;"));

	// assertion for triple-if checks
	assert(555 == script.Evaluate("if (0) if (0) if (0) return 111; return 555;"));
	assert(555 == script.Evaluate("if (0) if (0) if (1) return 111; return 555;"));
	assert(555 == script.Evaluate("if (0) if (1) if (0) return 111; return 555;"));
	assert(555 == script.Evaluate("if (0) if (1) if (1) return 111; return 555;"));
	assert(555 == script.Evaluate("if (1) if (0) if (0) return 111; return 555;"));
	assert(555 == script.Evaluate("if (1) if (0) if (1) return 111; return 555;"));
	assert(555 == script.Evaluate("if (1) if (1) if (0) return 111; return 555;"));
	assert(111 == script.Evaluate("if (1) if (1) if (1) return 111; return 555;"));

	// assertion for double-if-else checks
	assert(111 == script.Evaluate("if (1) if (1) return 111; else return 555; return 777;"));
	assert(555 == script.Evaluate("if (1) if (0) return 111; else return 555; return 777;"));
	assert(555 == script.Evaluate("if (0) if (1) return 111; else return 555; return 777;"));
	assert(555 == script.Evaluate("if (0) if (0) return 111; else return 555; return 777;"));

	// assertion for triple-if-else checks
	assert(555 == script.Evaluate("if (0) if (0) if (0) return 111; else return 555; return 777;"));
	assert(555 == script.Evaluate("if (0) if (0) if (1) return 111; else return 555; return 777;"));
	assert(555 == script.Evaluate("if (0) if (1) if (0) return 111; else return 555; return 777;"));
	assert(555 == script.Evaluate("if (0) if (1) if (1) return 111; else return 555; return 777;"));
	assert(555 == script.Evaluate("if (1) if (0) if (0) return 111; else return 555; return 777;"));
	assert(555 == script.Evaluate("if (1) if (0) if (1) return 111; else return 555; return 777;"));
	assert(555 == script.Evaluate("if (1) if (1) if (0) return 111; else return 555; return 777;"));
	assert(111 == script.Evaluate("if (1) if (1) if (1) return 111; else return 555; return 777;"));

	// assertion for scope checks
	assert(99 == script.Evaluate("{{5+5;} return 99; 9+9;} return 123;"));

	// assertion for if/else+scope checks
	assert(789 == script.Evaluate("if (1) {return 789;}"));
	assert(999 == script.Evaluate("if (0) {return 789;} else {return 999;}"));
	assert(123 == script.Evaluate("if (1) return 123; else {return 555;}"));
	assert(123 == script.Evaluate("if (1) {return 123;} else return 555;"));
	assert(555 == script.Evaluate("if (0) return 123; else {return 555;}"));
	assert(555 == script.Evaluate("if (0) {return 123;} else return 555;"));

	// assertion for multiple-"if/else+scope"
	assert(pi == script.Evaluate("if (1) { if (1) {return pi;} }"));
	assert(222 == script.Evaluate("if (1){ if (0) return 111;else {return 222;}}"));

	// assertions for the crc32
	assert(0 == ComputeCRC32("", 0));
	assert(0 == ComputeCRC32(NULL, 0));
	assert(0xcbf43926 == ComputeCRC32("123456789", 9));

	// assertions for the strings (easy)
	assert(0.0 == script.Evaluate("\"\"")); // test empty string ""
	assert(0xcbf43926 == script.Evaluate("\"123456789\"")); // test "123456789"

	// assertions for the strings + code (easy)
	assert(0.0 == script.Evaluate("return \"\"")); // test return empty string ""
	assert(0.0 == script.Evaluate("(\"\")")); // test '("")'
	assert(0xcbf43926 == script.Evaluate("(\"123456789\")")); // test '("123456789")'
	assert(0xcbf43926 == script.Evaluate("return \"123456789\"")); // test 'return "123456789"'
	assert(0xcbf43926 == script.Evaluate("max(\"\", \"123456789\")")); // test function + string
	assert(0xcbf43926 == script.Evaluate("return max(\"\", \"123456789\")")); // test return + function + string

	// assertions for store + constante : the constante will not be modified (no warning message)
	assert(pi == script.Evaluate("pi = 555"));
	assert(pi == script.Evaluate("pi = 555 + 999 * 777"));
	assert(pi == script.Evaluate("pi = max(555 + 999 * 777, 999)"));
	assert(pi == script.Evaluate("pi = cos(0.0)"));

	// check variable modification
	assert(777.0 == script.Evaluate("if (abc = 777) return abc;"));
	assert(777.0 == script.Evaluate("return abc"));
	assert(777.0 == script.Evaluate("return abc;"));

	// check compile (more complexe test)
	vector<Token> code = Compile("max(max(abc,44) + 1, max(11,22) + 1)", &ctx);
	assert(10.0 == script.Evaluate("abc = 10"));
	assert(10.0 == script.Evaluate("abc"));
	assert(45.0 == Execute(code));
	assert(99.0 == script.Evaluate("abc = 99"));
	assert(99.0 == script.Evaluate("abc"));
	assert(100.0 == Execute(code));

	// reset abc
	assert(0.0 == script.Evaluate("abc = 0"));

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

		double result = Evaluate(in, &ctx);
#if USE_STL
		cout << "result = "<<  result <<endl;
#else
		printf("result = %f\n", result);
#endif
	}

	return 0;
}
