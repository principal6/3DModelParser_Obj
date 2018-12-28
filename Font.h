#include "Global.h"

#define	MAX_FONT_NUM 10

class DirectFont9
{
public:
	DirectFont9();
	~DirectFont9();

	VOID CreateFont(LPDIRECT3DDEVICE9 D3DDevice, char* FontName, int FontSize, bool IsBold, int ScreenWidth, int ScreenHeight);
	VOID UseFont(int FontID);
	VOID SetFontColor(DWORD Color);
	VOID DrawText(int X, int Y, char* val);
	VOID DrawText(int X, int Y, LPCWSTR val);
	VOID DrawText(int X, int Y, int val);
	VOID DrawText(int X, int Y, float val);

private:
	LPD3DXFONT	g_pMainFont[MAX_FONT_NUM];

	int	numFontCount;
	int	CurrentFontID;

	int	g_ScreenWidth;
	int	g_ScreenHeight;

	DWORD g_FontColor;
};