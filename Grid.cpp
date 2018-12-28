#include "Grid.h"

DirectGrid9::DirectGrid9()
{
	numGridVertices = 0;
	numGridIndices = 0;
	AxisXColor = D3DCOLOR_RGBA(0, 255, 0, 255);
	AxisYColor = D3DCOLOR_RGBA(0, 0, 255, 255);
	AxisZColor = D3DCOLOR_RGBA(255, 0, 0, 255);
	GridColor = D3DCOLOR_RGBA(255, 255, 255, 255);

	D3DXMatrixIdentity(&matGridWorld);
}

DirectGrid9::~DirectGrid9()
{
	pDevice = NULL;
	SAFE_DELETEARRAY(pGrid);

	SAFE_RELEASE(pGridVB);
	SAFE_RELEASE(pGridIB);

	pDevice	= NULL;
}

void DirectGrid9::SetDevice(LPDIRECT3DDEVICE9 D3DDevice)
{
	pDevice = D3DDevice;
	return;
}

void DirectGrid9::AddGridX(float XInterval, int XSize, float GridFar)
{
	if (!(XSize % 2))
		XSize++;

	int numPrevVertices = 0;

	if (pGrid)
	{
		VERTEX_GRID*	tempGrid = new VERTEX_GRID[numGridVertices];
		memcpy(tempGrid, pGrid, sizeof(VERTEX_GRID) * numGridVertices);
		numPrevVertices = numGridVertices;

		SAFE_DELETEARRAY(pGrid);
		numGridVertices += (XSize*2);
		pGrid = new VERTEX_GRID[numGridVertices];
		memcpy(pGrid, tempGrid, sizeof(VERTEX_GRID) * numPrevVertices);
		SAFE_DELETEARRAY(tempGrid);
	}
	else
	{
		SAFE_DELETEARRAY(pGrid);
		numGridVertices += (XSize*2);
		pGrid = new VERTEX_GRID[numGridVertices];
	}

	int HalfSize = int(XSize / 2);

	for (int i = 0; i < XSize; i++)
	{
		pGrid[numPrevVertices + i*2].Position		= XMFLOAT3(-(GridFar/2), 0, XInterval * (i - HalfSize));
		pGrid[numPrevVertices + i*2 + 1].Position	= XMFLOAT3((GridFar/2), 0, XInterval * (i - HalfSize));
		pGrid[numPrevVertices + i*2].Color		= GridColor;
		pGrid[numPrevVertices + i*2 + 1].Color	= GridColor;

		if ((i - HalfSize) == 0)
		{
			pGrid[numPrevVertices + i*2].Color		= AxisXColor;
			pGrid[numPrevVertices + i*2 + 1].Color	= AxisXColor;
		}
	}

	return;
}

void DirectGrid9::AddGridZ(float ZInterval, int ZSize, float GridFar)
{
	if (!(ZSize % 2))
		ZSize++;

	int numPrevVertices = 0;

	if (pGrid)
	{
		VERTEX_GRID*	tempGrid = new VERTEX_GRID[numGridVertices];
		memcpy(tempGrid, pGrid, sizeof(VERTEX_GRID) * numGridVertices);
		numPrevVertices = numGridVertices;

		SAFE_DELETEARRAY(pGrid);
		numGridVertices += (ZSize*2);
		pGrid = new VERTEX_GRID[numGridVertices];
		memcpy(pGrid, tempGrid, sizeof(VERTEX_GRID) * numPrevVertices);
		SAFE_DELETEARRAY(tempGrid);
	}
	else
	{
		SAFE_DELETEARRAY(pGrid);
		numGridVertices += (ZSize*2);
		pGrid = new VERTEX_GRID[numGridVertices];
	}

	int HalfSize = int(ZSize / 2);

	for (int i = 0; i < ZSize; i++)
	{
		pGrid[numPrevVertices + i*2].Position		= XMFLOAT3(ZInterval * (i - HalfSize), 0, -(GridFar/2));
		pGrid[numPrevVertices + i*2 + 1].Position	= XMFLOAT3(ZInterval * (i - HalfSize), 0, (GridFar/2));
		pGrid[numPrevVertices + i*2].Color		= GridColor;
		pGrid[numPrevVertices + i*2 + 1].Color	= GridColor;

		if ((i - HalfSize) == 0)
		{
			pGrid[numPrevVertices + i*2].Color		= AxisZColor;
			pGrid[numPrevVertices + i*2 + 1].Color	= AxisZColor;
		}
	}

	return;
}

void DirectGrid9::AddGridXZ(float Interval, int XSize, int ZSize)
{
	AddGridX(Interval, XSize, ZSize * Interval);
	AddGridZ(Interval, ZSize, XSize * Interval);
}

void DirectGrid9::CreateGrid()
{
	// 정점 버퍼 갱신
	SAFE_RELEASE(pGridVB);

	int SizeOfVertices = sizeof(VERTEX_GRID)*numGridVertices;
	if (FAILED(pDevice->CreateVertexBuffer(SizeOfVertices, 0, D3DFVF_VERTEX_GRID, D3DPOOL_DEFAULT, &pGridVB, NULL)))
		return;

	VOID* pVertices;
	if (FAILED(pGridVB->Lock(0, SizeOfVertices, (void**)&pVertices, 0)))
		return;
	memcpy(pVertices, pGrid, SizeOfVertices);
	pGridVB->Unlock();


	// 색인 버퍼 갱신
	SAFE_RELEASE(pGridIB);

	numGridIndices = numGridVertices / 2;
	INDEX_GRID *NewIndices = new INDEX_GRID[numGridIndices];

	for (int i = 0; i < numGridIndices; i++)
	{
		NewIndices[i]._0 = i * 2;
		NewIndices[i]._1 = i * 2 + 1;
	}

	int SizeOfIndices = sizeof(INDEX_GRID)*numGridIndices;
	if (FAILED(pDevice->CreateIndexBuffer(SizeOfIndices, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pGridIB, NULL)))
		return;

	VOID* pIndices;
	if (FAILED(pGridIB->Lock(0, SizeOfIndices, (void **)&pIndices, 0)))
		return;
	memcpy(pIndices, NewIndices, SizeOfIndices);
	pGridIB->Unlock();

	SAFE_DELETEARRAY(NewIndices);
}

void DirectGrid9::DrawGrid()
{
	pDevice->SetTransform(D3DTS_WORLD, &matGridWorld);
	
	pDevice->SetRenderState( D3DRS_LIGHTING, false );
	pDevice->SetTexture(0, 0);

	pDevice->SetStreamSource(0, pGridVB, 0, sizeof(VERTEX_GRID));
	pDevice->SetFVF(D3DFVF_VERTEX_GRID);
	pDevice->SetIndices(pGridIB);

	pDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, numGridVertices, 0, numGridIndices);

	return;
}