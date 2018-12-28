#include "Global.h"

struct VERTEX_GRID
{
	XMFLOAT3	Position;
	D3DCOLOR	Color;
};

struct INDEX_GRID
{
	WORD _0, _1;
};

#define D3DFVF_VERTEX_GRID (D3DFVF_XYZ | D3DFVF_DIFFUSE)

class DirectGrid9
{
public:
	DirectGrid9();
	~DirectGrid9();

	void SetDevice(LPDIRECT3DDEVICE9 D3DDevice);

	void AddGridX(float XInterval, int XSize, float GridFar);
	void AddGridZ(float ZInterval, int ZSize, float GridFar);
	void AddGridXZ(float Interval, int XSize, int ZSize);
	void CreateGrid();

	void DrawGrid();

private:
	LPDIRECT3DDEVICE9		pDevice;
	LPDIRECT3DVERTEXBUFFER9	pGridVB;
	LPDIRECT3DINDEXBUFFER9	pGridIB;

	D3DXMATRIX			matGridWorld;

	int					numGridVertices;
	int					numGridIndices;
	D3DCOLOR			AxisXColor;
	D3DCOLOR			AxisYColor;
	D3DCOLOR			AxisZColor;
	D3DCOLOR			GridColor;
	VERTEX_GRID			*pGrid;
};