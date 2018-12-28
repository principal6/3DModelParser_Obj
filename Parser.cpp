#include "Parser.h"

// 전체 전역 변수
char			TempString[MAX_PARSE_LINE];
float			ParseFloats[MAX_PARSE_LINE];

// 문자열 관련 함수 //
char* StringTrim(char* sBuf)
{
	int iLen	= 0;
	int	i		= 0;
	int iCnt	= 0;

	iLen = strlen(sBuf);

	if(iLen < 1)
		return NULL;

	// 우측의 공백 제거
	for(i = iLen-1; i >= 0 ; --i)
	{
		char* p = sBuf + i;

		if( ' ' == *p || '\t' == *p)
			continue;

		*(sBuf + i+1) = '\0';
		break;
	}
	iLen = i +1 +1;

	// 좌측의 공백제거
	char sT[MAX_PARSE_LINE] = {0};
	strncpy_s(sT, sBuf, iLen);

	for(i=0; i < iLen; ++i)
	{
		char* p = sT + i;

		if( ' ' == *p || '\t' == *p)
			continue;

		break;
	}

	strcpy_s(TempString, sT+i);	// 결과 반영
	return TempString;
}

char* StringTrim_OBJ(char* sBuf)
{
	int iLen	= 0;
	int	i		= 0;
	int iCnt	= 0;

	iLen = strlen(sBuf);

	if(iLen < 1)
		return NULL;

	// 우측의 공백 제거
	for(i = iLen-1; i >= 0 ; --i)
	{
		char* p = sBuf + i;

		if( ' ' == *p || '\t' == *p || 13 == *p || 10 == *p)
			continue;

		*(sBuf + i+1) = '\0';
		break;
	}
	iLen = i +1 +1;

	// 좌측의 공백제거
	char sT[MAX_PARSE_LINE] = {0};
	strncpy_s(sT, sBuf, iLen);

	for(i=0; i < iLen; ++i)
	{
		char* p = sT + i;

		if( ' ' == *p || '\t' == *p || 13 == *p || 10 == *p)
			continue;

		break;
	}

	strcpy_s(TempString, sT+i);	// 결과 반영
	return TempString;
}

char* SplitString(char* String, char* SplitChar, int SplitIndex)
{
	memset(TempString, 0, sizeof(TempString));

	int iLen = strlen(String);
	int previ = 0;
	int splitcount = 0;
	
	for (int i = 1; i < iLen; i++)
	{
		if (String[i] == SplitChar[0])
		{
			if (splitcount == SplitIndex)
			{
				for (int j = previ+1; j < i; j++)
				{
					TempString[j-previ-1] = String[j];
				}
				break;
			}

			splitcount++;
			previ = i;
		}
	}

	return TempString;
}

char* GetNameBetweenBraces(char* val)
{
	memset(TempString, 0, sizeof(TempString));

	int iLen = strlen(val);
	int firstspace = 0;
	int secondspace = 0;

	for (int i = 0; i < iLen; i++)
	{
		if (val[i] == '{')
		{
			firstspace = i+1;
		}
		if (val[i] == '}')
		{
			secondspace = i-1;
			break;
		}
	}

	for (int i = firstspace; i <= secondspace; i++)
	{
		TempString[i-firstspace] = val[i];
	}

	strcpy_s(TempString, StringTrim(TempString));

	return TempString;
}

char* GetNameBetweenQuotes(char* val)
{
	memset(TempString, 0, sizeof(TempString));

	int iLen = strlen(val);
	int firstspace = 0;
	int secondspace = 0;

	for (int i = 0; i < iLen; i++)
	{
		if (val[i] == '\"')
		{
			if (firstspace == 0)
			{
				firstspace = i+1;
			}
			else
			{
				secondspace = i-1;
				break;
			}
		}
	}

	for (int i = firstspace; i <= secondspace; i++)
	{
		TempString[i-firstspace] = val[i];
	}

	strcpy_s(TempString, StringTrim(TempString));

	return TempString;
}

bool FindString(char* val, char* cmp)
{
	int	vallen = strlen(val);
	int	cmplen = strlen(cmp);
	int	cmpcount = 0;

	if (vallen < cmplen)
		return false;

	for (int i = 0; i <= vallen; i++)
	{
		if (val[i] == cmp[cmpcount])
		{
			cmpcount++;
			if (cmpcount == cmplen)
				return true;
		}
	}

	return false;
}

bool FindString_MD5(char* val, char* cmp)
{
	return (0 == _strnicmp(val, cmp, strlen(cmp)) ) ? 1: 0;
}

bool FindChar(char* val, char* cmp)
{
	int iLen = strlen(val);

	if (iLen < 1)
		return false;

	for (int i = 0; i <= iLen; i++)
	{
		if (val[i] == cmp[0])
			return true;
	}

	return false;
}

void GetFloatFromLine(char* line, char* split, char* splita)
{
	memset(ParseFloats, 0, sizeof(ParseFloats));	// float 값을 저장할 변수 초기화

	char temp[MAX_PARSE_LINE] = {0};
	strcpy_s(temp, line);
	strcat_s(temp, split);							// 마지막에 나오는 값도 얻어올 수 있도록 split문자 마지막에 하나 추가
	int iLen = strlen(temp);

	int previ = 0;
	int iNum = 0;

	for (int i = 0; i < iLen; i++)
	{
		char parsetemp[MAX_PARSE_LINE] = {0};
		if ((temp[i] == split[0]) || (temp[i] == splita[0]))
		{
			if (previ >= i)		// split 문자가 연속으로 나옴. 즉, 값이 없음!
				continue;

			for (int j = previ; j < i; j++)
			{
				parsetemp[j-previ] = temp[j];
			}
			iNum++;
			ParseFloats[iNum-1] = (float)atof(parsetemp);
			previ = i+1;
		}
	}
}