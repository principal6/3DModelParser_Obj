#include <iostream>

// 파싱용 상수 선언
#define MAX_PARSE_LINE		512			// 파일에서 읽어 들일 한 줄의 최대 길이
#define MAX_KEYWORD_LEN		64			// 파싱을 위한 키워드의 최대 길이
#define MAX_NAME_LEN		256			// 각종 이름들의 최대 길이

typedef char ANYNAME[MAX_NAME_LEN];		// 이름 저장용 변수 유형
typedef char KEYWORD[MAX_KEYWORD_LEN];	// 키워드 저장용 변수 유형

// 파싱용 변수 선언 (extern)
extern char		TempString[MAX_PARSE_LINE];
extern float	ParseFloats[MAX_PARSE_LINE];

// 함수 원형 선언
char*	StringTrim(char* sBuf);
char*	StringTrim_OBJ(char* sBuf);
char*	SplitString(char* String, char* SplitChar, int SplitIndex);
char*	GetNameBetweenBraces(char* val);
char*	GetNameBetweenQuotes(char* val);
bool	FindString(char* val, char* cmp);
bool	FindString_MD5(char* val, char* cmp);
bool	FindChar(char* val, char cmp[]);
void	GetFloatFromLine(char* line, char* split, char* splita);