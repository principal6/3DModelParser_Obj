#include <windows.h>
#include "ModelOBJ.h"
#include "DirectInput.h"
#include "Camera.h"


// ��� ����
const int				MAX_MODEL_NUM	= 10;


// D3D ���� ����
LPDIRECT3D9             g_pD3D			= NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice	= NULL;

LPDIRECT3DVERTEXBUFFER9	g_pModelVB		= NULL;
LPDIRECT3DINDEXBUFFER9	g_pModelIB		= NULL;

DirectCamera9			g_Camera;				// Direct Camera

DirectInput*			g_pDI			= NULL;	// Direct Input
D3DXVECTOR3				MouseState;				// Direct Input - ���콺 �̵�, ��
POINT					MouseScreen;			// Direct Input - ���콺 ��ǥ
int						MouseButtonState = 0;	// Direct Input - ���콺 ��ư ����

D3DLIGHT9				Light_Directional;

D3DXMATRIXA16			matModelWorld;
D3DXMATRIXA16			matView, matProj;


// ��Ÿ ���� ����
ModelOBJ				MyOBJModel[MAX_MODEL_NUM];
DWORD					SecondTimer		= 0;
int						FPS				= 0;
const int				ScreenWidth		= 1024;
const int				ScreenHeight	= 768;
bool					bBoundingBoxed	= false;


// �Լ� ���� ����
VOID Cleanup();
HRESULT InitD3D(HWND hWnd, HINSTANCE hInst);
HRESULT InitModel();
VOID SetupCameraMatrices();
VOID SetupModelMatrix(float Tx, float Ty, float Tz, float Rx, float Ry, float Rz, float Sx, float Sy, float Sz);
VOID SetupLights();
VOID Render();
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT );


HRESULT InitD3D(HWND hWnd, HINSTANCE hInst)
{
	if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;		// �ڵ� ���� ����(Depth Stencil)�� ����Ѵ�. (�׷��� ��ǻ�Ͱ� �˾Ƽ� �ָ� �ִ� �� ���� �׷��ܡ�)
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;	// ���� ���� ����: D3DFMT_D16
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;	// �ĸ� ���� ����: D3DFMT_UNKNOWN�� ���� ����ȭ�� ���˰� ��ġ
	//d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;	// �̰� ������ 60FPS�� �Ѿ �׷�����

	if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice ) ) )
		return E_FAIL;

	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	//g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

	InitModel();

	g_pDI = DirectInput::GetInstance();		// Direct Input �ʱ�ȭ
	g_pDI->InitDirectInput(hInst, hWnd);
	g_pDI->CreateKeyboardDevice(DISCL_BACKGROUND|DISCL_NONEXCLUSIVE);
	g_pDI->CreateMouseDevice(DISCL_BACKGROUND|DISCL_NONEXCLUSIVE);

	g_Camera.SetCamera_FreeLook(0.0f, 2.0f, 0.0f);

	return S_OK;
}

HRESULT InitModel()
{
	MyOBJModel[0].CreateModel(g_pd3dDevice, "Model\\", "TestMap");
	MyOBJModel[0].AddInstance( XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) );
	MyOBJModel[0].AddInstance( XMFLOAT3(0.0f, 0.0f, 50.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.5f, 0.5f, 0.5f) );

	return S_OK;
}

VOID SetupCameraMatrices()
{
	// �� ���(ī�޶� ����)
	g_Camera.UseCamera_FreeLook( g_pd3dDevice, &matView );
	
	// ���� ���(���ٰ� ����)
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 1000.0f );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}

VOID SetupModelMatrix(XMFLOAT3 Translation, XMFLOAT3 Rotation, XMFLOAT3 Scaling)
{
	// ���� ���(��ġ, ȸ��, ũ�� ����)
	D3DXMatrixIdentity( &matModelWorld );

		D3DXMATRIXA16 matTrans;
		D3DXMATRIXA16 matRotX;
		D3DXMATRIXA16 matRotY;
		D3DXMATRIXA16 matRotZ;
		D3DXMATRIXA16 matSize;

		D3DXMatrixTranslation(&matTrans, Translation.x, Translation.y, Translation.z);
		D3DXMatrixRotationX(&matRotX, Rotation.x);
		D3DXMatrixRotationY(&matRotY, Rotation.y);				// Y���� �������� ȸ�� (��, X&Z�� ȸ����)
		D3DXMatrixRotationZ(&matRotZ, Rotation.z);
		D3DXMatrixScaling(&matSize, Scaling.x, Scaling.y, Scaling.z);

	matModelWorld = matModelWorld * matTrans * matRotX * matRotY * matRotZ * matSize;
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matModelWorld);
}

VOID SetupLights()
{
	// �Ͼ���� ���⼺ ����(Directional Light)�� �����Ѵ�.
	D3DXVECTOR3 vecDir;
	ZeroMemory( &Light_Directional, sizeof( D3DLIGHT9 ) );
	Light_Directional.Type = D3DLIGHT_DIRECTIONAL;
	Light_Directional.Diffuse.r = 1.0f;
	Light_Directional.Diffuse.g = 1.0f;
	Light_Directional.Diffuse.b = 1.0f;
	vecDir = D3DXVECTOR3( 0.0f, -1.0f, 0.0f );
	D3DXVec3Normalize( ( D3DXVECTOR3* )&Light_Directional.Direction, &vecDir );

	g_pd3dDevice->SetLight( 0, &Light_Directional );
	g_pd3dDevice->LightEnable( 0, TRUE );

	// ȯ�汤�� ����Ѵ�.
	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffA0A0A0 );

	// ���� ����� �Ҵ�.
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
}

void DetectInput(HWND hWnd)
{
	if(GetFocus())	// Direct Input�� �Է��� ������
	{
		MouseState = g_pDI->DIMouseHandler();

		GetCursorPos(&MouseScreen);			// ������ �� ���콺 ��ǥ ������
		ScreenToClient(hWnd, &MouseScreen);

		MouseButtonState = 0;

		if (g_pDI->DIMouseButtonHandler(0))	// ���콺 ���� ��ư
		{
			MouseButtonState = 1;
		}

		if (g_pDI->DIMouseButtonHandler(1))	// ���콺 ������ ��ư
		{
			MouseButtonState = 2;
		}

		if (MouseState.x < 0)	// ���콺 �������� �̵�
		{
			g_Camera.RotateCamera_LeftRight(MouseState.x, 250.0f);
		}

		if (MouseState.x > 0)	// ���콺 ���������� �̵�
		{
			g_Camera.RotateCamera_LeftRight(MouseState.x, 250.0f);
		}

		if (MouseState.y < 0)	// ���콺 ���� �̵�
		{
			g_Camera.RotateCamera_UpDown(MouseState.y, 250.0f);
		}

		if (MouseState.y > 0)	// ���콺 �Ʒ��� �̵�
		{
			g_Camera.RotateCamera_UpDown(MouseState.y, 250.0f);
		}

		if(g_pDI->DIKeyboardHandler(DIK_W))	// W: ������ �̵�
		{
			g_Camera.MoveCamera_BackForth(false, 1.0f);
		}

		if(g_pDI->DIKeyboardHandler(DIK_S))	// S: �ڷ� �̵�
		{
			g_Camera.MoveCamera_BackForth(true, 1.0f);
		}

		if(g_pDI->DIKeyboardHandler(DIK_A))	// A: �������� �̵�
		{
			g_Camera.MoveCamera_LeftRight(true, 1.0f);
		}
		
		if(g_pDI->DIKeyboardHandler(DIK_D))	// D: ���������� �̵�
		{
			g_Camera.MoveCamera_LeftRight(false, 1.0f);
		}

		if(g_pDI->DIKeyboardHandler(DIK_B))	// B: �ٿ�� �ڽ� �׸���
		{
			bBoundingBoxed = !bBoundingBoxed;
		}

		if(g_pDI->DIKeyboardHandler(DIK_ESCAPE))	// ESC: ����
			DestroyWindow(hWnd);
	}
}

VOID Render()
{
	// �ĸ� ���ۿ� Z ���۸� û���Ѵ�.
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0, 200, 100 ), 1.0f, 0 );

	if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
	{
		SetupCameraMatrices();
		SetupLights();

		for (int i = 0; i < MyOBJModel[0].numInstances; i++)
		{
			//MyOBJModel[0].ModelInstances[i].Rotation.x += 0.01f;
			//MyOBJModel[0].ModelInstances[i].Rotation.y += 0.01f;
			SetupModelMatrix(MyOBJModel[0].ModelInstances[i].Translation,
				MyOBJModel[0].ModelInstances[i].Rotation,
				MyOBJModel[0].ModelInstances[i].Scaling);
			if (bBoundingBoxed == true)
				MyOBJModel[0].DrawBoundingBoxes(g_pd3dDevice);
			MyOBJModel[0].DrawMesh_Opaque(g_pd3dDevice);
		}

		for (int i = 0; i < MyOBJModel[0].numInstances; i++)
		{
			//MyOBJModel[0].ModelInstances[i].Rotation.x += 0.01f;
			//MyOBJModel[0].ModelInstances[i].Rotation.y += 0.01f;
			SetupModelMatrix(MyOBJModel[0].ModelInstances[i].Translation,
				MyOBJModel[0].ModelInstances[i].Rotation,
				MyOBJModel[0].ModelInstances[i].Scaling);
			if (bBoundingBoxed == true)
				MyOBJModel[0].DrawBoundingBoxes(g_pd3dDevice);
			MyOBJModel[0].DrawMesh_Transparent(g_pd3dDevice);
		}

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
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "D3DGAME", NULL };
	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow( "D3DGAME", "GAME", WS_OVERLAPPEDWINDOW, 0, 0, ScreenWidth, ScreenHeight, NULL, NULL, wc.hInstance, NULL );

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
			DetectInput(hWnd);	// Direct Input Ű����&���콺 �Է� ����!
			Render();

			if (GetTickCount() >= SecondTimer + 1000)	// �ʽð�
			{
				SecondTimer = GetTickCount();

				char temp[20];
				_itoa_s(FPS, temp, 10);
				SetWindowText(hWnd, temp);

				FPS = 0;
			}
		}
	}

	Cleanup();
	UnregisterClass( "D3DGAME", wc.hInstance );
	return 0;
}

VOID Cleanup()
{
	for (int i = 0; i < MAX_MODEL_NUM; i++)
	{
		MyOBJModel[i].Destroy();
	}

	g_pDI->ShutdownDirectInput();
	g_pDI->DeleteInstance();

	SAFE_RELEASE(g_pModelVB);
	SAFE_RELEASE(g_pModelIB);
	SAFE_RELEASE(g_pd3dDevice);
	SAFE_RELEASE(g_pD3D);
}