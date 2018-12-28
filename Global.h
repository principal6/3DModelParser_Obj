#ifndef GLOBAL_H
#define GLOBAL_H

	#include <d3dx9.h>
	#include <xnamath.h>
	#include <iostream>

	extern LPDIRECT3DVERTEXBUFFER9	g_pModelVB;
	extern LPDIRECT3DINDEXBUFFER9	g_pModelIB;

	#ifndef SAFE_RELEASE
	#define SAFE_RELEASE(p)	if(p) {p->Release(); p=NULL;}
	#endif

	struct PickingRay
	{
		PickingRay(){};
		PickingRay(D3DXVECTOR3 VPos, D3DXVECTOR3 VDir): Pos(VPos), Dir(VDir) {};
		D3DXVECTOR3 Pos;
		D3DXVECTOR3 Dir;
	};

#endif