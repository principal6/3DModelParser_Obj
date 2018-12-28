#ifndef GLOBAL_H
#define GLOBAL_H

	#include <d3dx9.h>
	#include <xnamath.h>
	#include <iostream>

	#ifndef SAFE_RELEASE
	#define SAFE_RELEASE(p)	if(p) {p->Release(); p=NULL;}
	#endif

	#ifndef SAFE_DELETE
	#define SAFE_DELETE(p)	if(p) {delete p; p=NULL;}
	#endif

	#ifndef SAFE_DELETEARRAY
	#define SAFE_DELETEARRAY(p)	if(p) {delete[] p; p=NULL;}
	#endif


	struct PickingRay
	{
		PickingRay(){};
		PickingRay(D3DXVECTOR3 VPos, D3DXVECTOR3 VDir): Pos(VPos), Dir(VDir) {};
		D3DXVECTOR3 Pos;
		D3DXVECTOR3 Dir;
	};

#endif