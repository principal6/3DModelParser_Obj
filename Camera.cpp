#include "Camera.h"

float			CamDefaultTargetY;

float			CamMoveLR;
float			CamMoveBF;
float			CamPitch;
float			CamRotLR;
float			CamRotUD;

D3DXVECTOR3		CamTarget;
D3DXVECTOR3		CamUp;
D3DXVECTOR3		CamForward;
D3DXVECTOR3		CamDefaultForward;
D3DXVECTOR3		CamRight;
D3DXVECTOR3		CamDefaultRight;


DirectCamera9::DirectCamera9()
{
	CamMoveLR = 0.0f;
	CamMoveBF = 0.0f;
	CamYaw = 0.0f;
	CamPitch = 0.0f;
	CamRoll = 0.0f;
	CamRotLR = 0.0f;
	CamRotUD = 0.0f;

	CamDefaultTargetY = 0.0f;
}

DirectCamera9::~DirectCamera9()
{
	pDevice = NULL;
}

D3DXMATRIX DirectCamera9::GetViewMatrix()
{
	return matView;
}

D3DXMATRIX DirectCamera9::GetProjectionMatrix()
{
	return matProj;
}

VOID DirectCamera9::CreateViewFrustum()
{
	D3DXMATRIX matVP;
	D3DXMatrixMultiply( &matVP, &matView, &matProj);

	// Left plane
	ViewFrustum[0].a = matVP._14 + matVP._11;
	ViewFrustum[0].b = matVP._24 + matVP._21;
	ViewFrustum[0].c = matVP._34 + matVP._31;
	ViewFrustum[0].d = matVP._44 + matVP._41;
 
	// Right plane
	ViewFrustum[1].a = matVP._14 - matVP._11;
	ViewFrustum[1].b = matVP._24 - matVP._21;
	ViewFrustum[1].c = matVP._34 - matVP._31;
	ViewFrustum[1].d = matVP._44 - matVP._41;
 
	// Top plane
	ViewFrustum[2].a = matVP._14 - matVP._12;
	ViewFrustum[2].b = matVP._24 - matVP._22;
	ViewFrustum[2].c = matVP._34 - matVP._32;
	ViewFrustum[2].d = matVP._44 - matVP._42;
 
	// Bottom plane
	ViewFrustum[3].a = matVP._14 + matVP._12;
	ViewFrustum[3].b = matVP._24 + matVP._22;
	ViewFrustum[3].c = matVP._34 + matVP._32;
	ViewFrustum[3].d = matVP._44 + matVP._42;
 
	// Near plane
	ViewFrustum[4].a = matVP._13;
	ViewFrustum[4].b = matVP._23;
	ViewFrustum[4].c = matVP._33;
	ViewFrustum[4].d = matVP._43;
 
	// Far plane
	ViewFrustum[5].a = matVP._14 - matVP._13;
	ViewFrustum[5].b = matVP._24 - matVP._23;
	ViewFrustum[5].c = matVP._34 - matVP._33;
	ViewFrustum[5].d = matVP._44 - matVP._43;
 
	// Normalize planes
	for ( int i = 0; i < 6; i++ )
	{
		D3DXPlaneNormalize( &ViewFrustum[i], &ViewFrustum[i] );
	}
}

bool DirectCamera9::IsSphereInFrustum( D3DXVECTOR3* pPosition, float radius)
{
	for ( int i = 0; i < 6; i++ )
	{
		if ( D3DXPlaneDotCoord( &ViewFrustum[i], pPosition ) + radius < 0 )
		{
			// Outside the frustum, reject it!
			return false;
		}
	}
	return true;
}

void DirectCamera9::SetDevice(LPDIRECT3DDEVICE9 D3DDevice)
{
	pDevice = D3DDevice;
	return;
}

VOID DirectCamera9::SetCamera_FirstPerson(float CameraY)
{
	CamDefaultForward	= D3DXVECTOR3(1.0f, CameraY, 0.0f);
	CamDefaultRight		= D3DXVECTOR3(0.0f, CameraY, 1.0f);
}

VOID DirectCamera9::SetCamera_FreeLook(float CameraX, float CameraY, float CameraZ)
{
	CamPosition			= D3DXVECTOR3(CameraX, CameraY, CameraZ);
	CamDefaultForward	= D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	CamDefaultRight		= D3DXVECTOR3(0.0f, 0.0f, 1.0f);
}

VOID DirectCamera9::SetCamera_ThirdPerson(float DistanceHigh, float DistanceFar, float TargetY)
{
	CamDefaultPosition	= D3DXVECTOR3(-DistanceFar, DistanceHigh, 0.0f);
	CamUp				= D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	CamDefaultTargetY	= TargetY;
}

VOID DirectCamera9::SetCamera_Static(float CamPX, float CamPY, float CamPZ, float CamTX, float CamTY, float CamTZ)
{
	CamPosition	= D3DXVECTOR3(CamPX, CamPY, CamPZ);
	CamTarget	= D3DXVECTOR3(CamTX, CamTY, CamTZ);
	CamUp		= D3DXVECTOR3(0.0f, 1.0f, 0.0f);
}

VOID DirectCamera9::UseCamera_FirstPerson()
{
	CameraType = 0;

	// (옵션1) 1인칭 시점 카메라(First-Person Camera)
	D3DXMATRIX matCameraRotation;
		D3DXMatrixRotationYawPitchRoll(&matCameraRotation, CamYaw, 0.0f, CamRoll);
		D3DXVec3TransformCoord(&CamTarget, &CamDefaultForward, &matCameraRotation);
		D3DXVec3Normalize(&CamTarget, &CamTarget);

	D3DXMATRIX RotateYTempMatrix;
		D3DXMatrixRotationY(&RotateYTempMatrix, CamYaw);
	D3DXVec3TransformCoord(&CamRight, &CamDefaultRight, &RotateYTempMatrix);
	D3DXVec3TransformCoord(&CamUp, &CamUp, &RotateYTempMatrix);
	D3DXVec3TransformCoord(&CamForward, &CamDefaultForward, &RotateYTempMatrix);

	CamPosition += CamMoveLR * CamRight;
	CamPosition += CamMoveBF * CamForward;

	CamMoveLR = 0.0f;
	CamMoveBF = 0.0f;

	CamTarget = CamPosition + CamTarget;
	D3DXMatrixLookAtLH( &matView, &CamPosition, &CamTarget, &CamUp );
	pDevice->SetTransform( D3DTS_VIEW, &matView );
}

VOID DirectCamera9::UseCamera_FreeLook()
{
	CameraType = 1;

	// (옵션2) 자유 시점 카메라(Free-Look Camera)
	D3DXMATRIX matCameraRotation;
		D3DXMatrixRotationYawPitchRoll(&matCameraRotation, CamYaw, 0.0f, CamRoll);
		D3DXVec3TransformCoord(&CamTarget, &CamDefaultForward, &matCameraRotation);
		D3DXVec3Normalize(&CamTarget, &CamTarget);
	D3DXVec3TransformCoord(&CamRight, &CamDefaultRight, &matCameraRotation);
	D3DXVec3TransformCoord(&CamForward, &CamDefaultForward, &matCameraRotation);
	D3DXVec3Cross(&CamUp, &CamRight, &CamForward);
	
	CamPosition += CamMoveLR * CamRight;
	CamPosition += CamMoveBF * CamForward;

	CamMoveLR = 0.0f;
	CamMoveBF = 0.0f;

	CamTarget = CamPosition + CamTarget;
	D3DXMatrixLookAtLH( &matView, &CamPosition, &CamTarget, &CamUp );
	pDevice->SetTransform( D3DTS_VIEW, &matView );
}

VOID DirectCamera9::UseCamera_ThirdPerson(D3DXVECTOR3 SpritePosition)
{
	CameraType = 2;

	// (옵션3) 3인칭 중앙 시점
	D3DXMATRIX matCameraRotation;

	CamTarget = SpritePosition;
	CamTarget.y += CamDefaultTargetY;

	D3DXMatrixRotationYawPitchRoll(&matCameraRotation, CamYaw, 0.0f, CamRoll);
	D3DXVec3TransformCoord(&CamPosition, &CamDefaultPosition, &matCameraRotation);
	CamPosition += CamTarget;

	D3DXMatrixLookAtLH( &matView, &CamPosition, &CamTarget, &CamUp );
	pDevice->SetTransform( D3DTS_VIEW, &matView );
}

VOID DirectCamera9::UseCamera_Static()
{
	CameraType = 3;

	// (옵션4) 정적 카메라(Static Camera)
	D3DXMatrixLookAtLH( &matView, &CamPosition, &CamTarget, &CamUp );
	pDevice->SetTransform( D3DTS_VIEW, &matView );
}

VOID DirectCamera9::SetProjection(float ZFar)
{
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, ZFar );
	pDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}

VOID DirectCamera9::MoveCamera_BackForth(bool MoveBack, float MoveDistance)
{
	if (CameraType == 0 || CameraType == 1)
	{
		switch (MoveBack)
		{
			case true:
				CamMoveBF -= MoveDistance;
				break;

			case false:
				CamMoveBF += MoveDistance;
				break;
		}
	}
}

VOID DirectCamera9::MoveCamera_LeftRight(bool MoveLeft, float MoveDistance)
{
	if (CameraType == 0 || CameraType == 1)
	{
		switch (MoveLeft)
		{
			case true:
				CamMoveLR += MoveDistance;
				break;

			case false:
				CamMoveLR -= MoveDistance;
				break;
		}
	}
}


VOID DirectCamera9::ZoomCamera(bool ZoomIn, float MoveDistance)
{
	if (CameraType == 2)
	{
		switch (ZoomIn)
		{
			case true:
				CamDefaultPosition.x += MoveDistance;
				CamDefaultPosition.y -= MoveDistance;
				break;

			case false:
				CamDefaultPosition.x -= MoveDistance;
				CamDefaultPosition.y += MoveDistance;
				break;
		}
	}
}

VOID DirectCamera9::RotateCamera_LeftRight(float MouseMovedX, float SpeedFactor)
{
	CamYaw += (MouseMovedX / SpeedFactor);
}

VOID DirectCamera9::RotateCamera_UpDown(float MouseMovedY, float SpeedFactor)
{
	CamRoll -= (MouseMovedY / SpeedFactor);
}

VOID DirectCamera9::RotateCamera_LeftRightRegular(bool RotateLeft, float RotateAngle)
{
	switch (RotateLeft)
	{
		case true:
			CamYaw -= RotateAngle;
			break;

		case false:
			CamYaw += RotateAngle;
			break;
	}
}

VOID DirectCamera9::RotateCamera_UpDownRegular(bool RotateUp, float RotateAngle)
{
	switch (RotateUp)
	{
		case true:
			CamRoll += RotateAngle;
			break;

		case false:
			CamRoll -= RotateAngle;
			break;
	}
}