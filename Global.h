#include <d3dx9.h>
#include <xnamath.h>
#include <iostream>

extern LPDIRECT3DVERTEXBUFFER9	g_pModelVB;
extern LPDIRECT3DINDEXBUFFER9	g_pModelIB;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)	if(p) {p->Release(); p=NULL;}
#endif