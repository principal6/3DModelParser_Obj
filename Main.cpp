#include <windows.h>
#include "ModelOBJ.h"
#include "DirectInput.h"
#include "Camera.h"
#include "Font.h"
#include "Grid.h"
#include <crtdbg.h>

#ifdef _DEBUG	// 메모리 누수 검사
#define new new( _CLIENT_BLOCK, __FILE, __LINE__)
#endif


// 상수 선언
const int				ScreenWidth		= 800;
const int				ScreenHeight	= 600;
const char				WindowName[]	= "D3DGAME";
const char				WindowTitle[]	= "GAME";
const int				MAX_MODEL_NUM	= 10;


// D3D 변수 선언
LPDIRECT3D9             g_pD3D			= NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice	= NULL;
LPD3DXEFFECT			g_pHLSL			= NULL;

DirectCamera9			g_Camera;				// Direct Camera
DirectFont9				g_Font;					// Direct Font
DirectGrid9				g_Grid;					// Direct Grid

DirectInput*			g_pDI			= NULL;	// Direct Input
D3DXVECTOR3				MouseState;				// Direct Input - 마우스 이동, 휠
POINT					MouseScreen;			// Direct Input - 마우스 좌표
int						MouseButtonState = 0;	// Direct Input - 마우스 버튼 눌림

D3DXMATRIXA16			matModelWorld;
D3DXMATRIXA16			matView, matProj;


// 기타 변수 선언
DWORD					SecondTimer		= 0;
int						FPS				= 0;
int						FPS_Shown		= 0;

ModelOBJ				MyOBJModel[MAX_MODEL_NUM];
bool					bDrawBoundingBoxes	= false;
bool					bDrawNormalVectors	= false;
bool					bWireFrame			= false;


// 함수 원형 선언
VOID Cleanup();
HRESULT InitD3D( HWND hWnd, HINSTANCE hInst );
HRESULT InitModel();
VOID Render();
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT );


HRESULT InitD3D( HWND hWnd, HINSTANCE hInst )
{
	if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;		// 자동 깊이 공판(Depth Stencil)을 사용한다. (그래야 컴퓨터가 알아서 멀리 있는 걸 먼저 그려줌★)
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;	// 깊이 공판 형식: D3DFMT_D16
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;	// 후면 버퍼 형식: D3DFMT_UNKNOWN는 현재 바탕화면 포맷과 일치

	if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice ) ) )
		return E_FAIL;

	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

	// 셰이더 컴파일
	D3DXCreateEffectFromFile(g_pd3dDevice, "HLSLMain.fx", NULL, NULL, NULL, NULL, &g_pHLSL, NULL);
	if (!g_pHLSL)
		return E_FAIL;

	InitModel();

	g_Grid.SetDevice(g_pd3dDevice);
	g_Grid.AddGridXZ(1, 100, 100);
	g_Grid.CreateGrid();

	g_pDI = DirectInput::GetInstance();		// Direct Input 초기화
	g_pDI->InitDirectInput(hInst, hWnd);
	g_pDI->CreateKeyboardDevice(DISCL_BACKGROUND|DISCL_NONEXCLUSIVE);
	g_pDI->CreateMouseDevice(DISCL_BACKGROUND|DISCL_NONEXCLUSIVE);

	g_Camera.SetDevice(g_pd3dDevice);
	g_Camera.SetCamera_FreeLook(0.0f, 0.0f, 0.0f);

	g_Font.CreateFontA(g_pd3dDevice, "Arial", 20, false, ScreenWidth, ScreenHeight);

	return S_OK;
}

HRESULT InitModel()
{
	MyOBJModel[0].CreateModel(g_pd3dDevice, "Model\\", "TestGeos", g_pHLSL);
	MyOBJModel[0].AddInstance( XMFLOAT3(10.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, D3DX_PI/2, 0.0f), XMFLOAT3(0.5f, 0.5f, 0.5f) );

	return S_OK;
}

void DetectInput(HWND hWnd)
{
	if(GetFocus())	// Direct Input에 입력이 들어오면
	{
		MouseState = g_pDI->DIMouseHandler();

		GetCursorPos(&MouseScreen);			// 윈도우 상 마우스 좌표 얻어오기
		ScreenToClient(hWnd, &MouseScreen);

		MouseButtonState = 0;

		if (g_pDI->DIMouseButtonHandler(0))	// 마우스 왼쪽 버튼
		{
			MouseButtonState = 1;
		}

		if (g_pDI->DIMouseButtonHandler(1))	// 마우스 오른쪽 버튼
		{
			MouseButtonState = 2;
		}

		if (MouseState.x < 0)	// 마우스 왼쪽으로 이동
		{
			g_Camera.RotateCamera_LeftRight(MouseState.x, 250.0f);
		}

		if (MouseState.x > 0)	// 마우스 오른쪽으로 이동
		{
			g_Camera.RotateCamera_LeftRight(MouseState.x, 250.0f);
		}

		if (MouseState.y < 0)	// 마우스 위로 이동
		{
			g_Camera.RotateCamera_UpDown(MouseState.y, 250.0f);
		}

		if (MouseState.y > 0)	// 마우스 아래로 이동
		{
			g_Camera.RotateCamera_UpDown(MouseState.y, 250.0f);
		}

		if(g_pDI->DIKeyboardHandler(DIK_W))	// W: 앞으로 이동
		{
			g_Camera.MoveCamera_BackForth(false, 1.0f);
		}

		if(g_pDI->DIKeyboardHandler(DIK_S))	// S: 뒤로 이동
		{
			g_Camera.MoveCamera_BackForth(true, 1.0f);
		}

		if(g_pDI->DIKeyboardHandler(DIK_A))	// A: 왼쪽으로 이동
		{
			g_Camera.MoveCamera_LeftRight(true, 1.0f);
		}
		
		if(g_pDI->DIKeyboardHandler(DIK_D))	// D: 오른쪽으로 이동
		{
			g_Camera.MoveCamera_LeftRight(false, 1.0f);
		}

		if(g_pDI->DIKeyboardHandler(DIK_B))	// B: 바운딩 박스 그리기
		{
			bDrawBoundingBoxes = !bDrawBoundingBoxes;
			Sleep(140);
		}

		if(g_pDI->DIKeyboardHandler(DIK_N))	// N: 법선 벡터 그리기
		{
			bDrawNormalVectors = !bDrawNormalVectors;
			Sleep(140);
		}

		if(g_pDI->DIKeyboardHandler(DIK_R))	// R: 그리기 모드
		{
			bWireFrame = !bWireFrame;
			Sleep(140);

			if (bWireFrame == true)
			{
				g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
			}
			else
			{
				g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
			}
		}

		if(g_pDI->DIKeyboardHandler(DIK_SPACE))	// Spacebar: 카메라 위치, 방향 초기화
		{
			g_Camera.SetCamera_FreeLook(0.0f, 0.0f, 0.0f);
			g_Camera.CamYaw = 0;
			g_Camera.CamRoll = 0;
			Sleep(140);
		}

		if(g_pDI->DIKeyboardHandler(DIK_ESCAPE))	// ESC: 종료
			DestroyWindow(hWnd);
	}
}

VOID Render()
{
	// 후면 버퍼와 Z 버퍼를 청소한다.
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0, 200, 100 ), 1.0f, 0 );

	if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
	{
		// 카메라 설정
		g_Camera.UseCamera_FreeLook();
		g_Camera.SetProjection(1000.0f);

		g_Grid.DrawGrid();

		// 조명 기능을 켠다. (HLSL을 쓰지 않을 때는 반드시 켜야만 알파 블렌딩이 된다!★★)
		//g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, true);

		for (int i = 0; i < MyOBJModel[0].numInstances; i++)
		{
			//MyOBJModel[0].DrawModel(i);
			//MyOBJModel[0].DrawModel_HLSL(i, g_Camera.GetViewMatrix(), g_Camera.GetProjectionMatrix());
			//MyOBJModel[0].DrawMesh_Opaque(i);
			MyOBJModel[0].DrawMesh_HLSL_Opaque(i, g_Camera.GetViewMatrix(), g_Camera.GetProjectionMatrix());
		}

		for (int i = 0; i < MyOBJModel[0].numInstances; i++)
		{
			//MyOBJModel[0].DrawMesh_Transparent(i);
			MyOBJModel[0].DrawMesh_HLSL_Transparent(i, g_Camera.GetViewMatrix(), g_Camera.GetProjectionMatrix());

			if (bDrawBoundingBoxes == true)
				MyOBJModel[0].DrawBoundingBoxes();
			if (bDrawNormalVectors == true)
				MyOBJModel[0].DrawNormalVecters(1.0f);

		}

		if (g_pDI->OnMouseButtonDown(0) == true)
		{
			for (int i = 0; i < MyOBJModel[0].numInstances; i++)
			{
				MyOBJModel[0].CheckMouseOverPerInstance(i, MouseScreen.x, MouseScreen.y, ScreenWidth, ScreenHeight,
					g_Camera.GetViewMatrix(), g_Camera.GetProjectionMatrix());
			}
			MyOBJModel[0].CheckMouseOverFinal();
		}

		g_Font.SetFontColor(0xFFFFFFFF);
		g_Font.DrawTextA(0, 0, "FPS: ");
		g_Font.DrawTextA(40, 0, FPS_Shown);
		g_Font.DrawTextA(0, 20, MyOBJModel[0].MouseOverPerInstances[0]);
		g_Font.DrawTextA(0, 40, MyOBJModel[0].PickedPosition[0].x);
		g_Font.DrawTextA(0, 60, MyOBJModel[0].PickedPosition[0].y);
		g_Font.DrawTextA(0, 80, MyOBJModel[0].PickedPosition[0].z);

		g_Font.DrawTextA(100, 20, MyOBJModel[0].MouseOverPerInstances[1]);
		g_Font.DrawTextA(100, 40, MyOBJModel[0].PickedPosition[1].x);
		g_Font.DrawTextA(100, 60, MyOBJModel[0].PickedPosition[1].y);
		g_Font.DrawTextA(100, 80, MyOBJModel[0].PickedPosition[1].z);

		g_pd3dDevice->EndScene();
	}

	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}


LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_DESTROY:
			PostQuitMessage( 0 );
			return 0;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}


INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
	#ifdef _DEBUG	// 메모리 누수 검사
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, WindowName, NULL };
	RegisterClassEx( &wc );

	RECT WndRect = {0, 30, ScreenWidth, 30 + ScreenHeight};
	AdjustWindowRect(&WndRect, WS_OVERLAPPEDWINDOW, false);

	HWND hWnd = CreateWindow( WindowName, WindowTitle, WS_OVERLAPPEDWINDOW,
		WndRect.left, WndRect.top, WndRect.right-WndRect.left, WndRect.bottom-WndRect.top, NULL, NULL, wc.hInstance, NULL );

	if( FAILED( InitD3D( hWnd, hInst ) ) )
		return 0;

	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			FPS++;
			DetectInput(hWnd);	// Direct Input 키보드&마우스 입력 감지!
			Render();

			if (GetTickCount() >= SecondTimer + 1000)	// 초시계
			{
				SecondTimer = GetTickCount();
				FPS_Shown = FPS;
				FPS = 0;
			}
		}
	}

	Cleanup();
	UnregisterClass( WindowName, wc.hInstance );
	return 0;
}

VOID Cleanup()
{
	g_pDI->ShutdownDirectInput();
	g_pDI->DeleteInstance();

	SAFE_RELEASE(g_pd3dDevice);
	SAFE_RELEASE(g_pD3D);
}