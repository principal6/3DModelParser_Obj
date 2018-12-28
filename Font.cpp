#include "Font.h"

DirectFont9::DirectFont9()
{
	for (int i = 0; i < MAX_FONT_NUM; i++)
	{
		g_pMainFont[i] = NULL;
	}

	numFontCount = 0;
	CurrentFontID = 0;

	g_ScreenWidth = 0;
	g_ScreenHeight = 0;

	g_FontColor = 0xFF000000;
}

DirectFont9::~DirectFont9()
{
	for (int i = 0; i < MAX_FONT_NUM; i++)
	{
		SAFE_RELEASE(g_pMainFont[i]);
	}
}

VOID DirectFont9::CreateFont(LPDIRECT3DDEVICE9 D3DDevice, char* FontName, int FontSize, bool IsBold, int ScreenWidth, int ScreenHeight)
{
	g_ScreenWidth = ScreenWidth;
	g_ScreenHeight = ScreenHeight;

	if (IsBold)
	{
		D3DXCreateFont(D3DDevice, FontSize, 0, FW_BOLD, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			ANTIALIASED_QUALITY, DEFAULT_PITCH|FF_DONTCARE, FontName, &g_pMainFont[numFontCount]);
	}
	else
	{
		D3DXCreateFont(D3DDevice, FontSize, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			ANTIALIASED_QUALITY, DEFAULT_PITCH|FF_DONTCARE, FontName, &g_pMainFont[numFontCount]);
	}

	numFontCount++;
}

VOID DirectFont9::UseFont(int FontID)
{
	CurrentFontID = FontID;
}

VOID DirectFont9::SetFontColor(DWORD Color)
{
	g_FontColor = Color;
}

VOID DirectFont9::DrawText(int X, int Y, char* val)
{
	RECT Rect_Font;
	SetRect(&Rect_Font, X, Y, g_ScreenWidth, g_ScreenHeight);
	g_pMainFont[CurrentFontID]->DrawText(NULL, val, -1, &Rect_Font, DT_LEFT|DT_NOCLIP, g_FontColor);

	return;
}

VOID DirectFont9::DrawText(int X, int Y, LPCWSTR val)
{
	RECT Rect_Font;
	SetRect(&Rect_Font, X, Y, g_ScreenWidth, g_ScreenHeight);
	g_pMainFont[CurrentFontID]->DrawTextW(NULL, val, -1, &Rect_Font, DT_LEFT|DT_NOCLIP, g_FontColor);

	return;
}

VOID DirectFont9::DrawText(int X, int Y, int val)
{
	char Temp[256] = {0};
	sprintf_s(Temp, "%d", val);

	RECT Rect_Font;
	SetRect(&Rect_Font, X, Y, g_ScreenWidth, g_ScreenHeight);
	g_pMainFont[CurrentFontID]->DrawTextA(NULL, Temp, -1, &Rect_Font, DT_LEFT|DT_NOCLIP, g_FontColor);

	return;
}

VOID DirectFont9::DrawText(int X, int Y, float val)
{
	char Temp[256] = {0};
	sprintf_s(Temp, "%f", val);

	RECT Rect_Font;
	SetRect(&Rect_Font, X, Y, g_ScreenWidth, g_ScreenHeight);
	g_pMainFont[CurrentFontID]->DrawTextA(NULL, Temp, -1, &Rect_Font, DT_LEFT|DT_NOCLIP, g_FontColor);

	return;
}