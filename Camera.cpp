#include "Camera.h"

float			CamMoveLR;
float			CamMoveBF;
float			CamPitch;
float			CamRoll;
float			CamRotLR;
float			CamRotUD;

D3DXVECTOR3		CamTarget;
D3DXVECTOR3		CamUp;
D3DXVECTOR3		CamForward;
D3DXVECTOR3		CamDefaultForward;
D3DXVECTOR3		CamRight;
D3DXVECTOR3		CamDefaultRight;


VOID DirectCamera9::SetCamera_FirstPerson(float CameraY)
{
	CamDefaultForward	= D3DXVECTOR3(1.0f, CameraY, 0.0f);
	CamDefaultRight		= D3DXVECTOR3(0.0f, CameraY, 1.0f);

	CamMoveLR = 0.0f;
	CamMoveBF = 0.0f;
	CamYaw = 0.0f;
	CamPitch = 0.0f;
	CamRoll = 0.0f;
	CamRotLR = 0.0f;
	CamRotUD = 0.0f;
}

VOID DirectCamera9::SetCamera_FreeLook(float CameraX, float CameraY, float CameraZ)
{
	CamPosition			= D3DXVECTOR3(CameraX, CameraY, CameraZ);
	CamDefaultForward	= D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	CamDefaultRight		= D3DXVECTOR3(0.0f, 0.0f, 1.0f);

	CamMoveLR = 0.0f;
	CamMoveBF = 0.0f;
	CamYaw = 0.0f;
	CamPitch = 0.0f;
	CamRoll = 0.0f;
	CamRotLR = 0.0f;
	CamRotUD = 0.0f;
}

VOID DirectCamera9::SetCamera_ThirdPerson(float DistanceHigh, float DistanceFar)
{
	CamDefaultPosition	= D3DXVECTOR3(-DistanceFar, DistanceHigh, 0.0f);
	CamUp				= D3DXVECTOR3(0.0f, 1.0f, 0.0f);

	CamMoveLR = 0.0f;
	CamMoveBF = 0.0f;
	CamYaw = 0.0f;
	CamPitch = 0.0f;
	CamRoll = 0.0f;
	CamRotLR = 0.0f;
	CamRotUD = 0.0f;
}

VOID DirectCamera9::UseCamera_FirstPerson(LPDIRECT3DDEVICE9 D3DDevice, D3DXMATRIXA16* matView)
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
	D3DXMatrixLookAtLH( matView, &CamPosition, &CamTarget, &CamUp );
	D3DDevice->SetTransform( D3DTS_VIEW, matView );
}

VOID DirectCamera9::UseCamera_FreeLook(LPDIRECT3DDEVICE9 D3DDevice, D3DXMATRIXA16* matView)
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
	D3DXMatrixLookAtLH( matView, &CamPosition, &CamTarget, &CamUp );
	D3DDevice->SetTransform( D3DTS_VIEW, matView );
}

VOID DirectCamera9::UseCamera_ThirdPerson(LPDIRECT3DDEVICE9 D3DDevice, D3DXMATRIXA16* matView, D3DXVECTOR3 SpritePosition)
{
	CameraType = 2;

	// (옵션3) 3인칭 중앙 시점
	D3DXMATRIX matCameraRotation;

	CamTarget = SpritePosition;

	D3DXMatrixRotationYawPitchRoll(&matCameraRotation, CamYaw, 0.0f, CamRoll);
	D3DXVec3TransformCoord(&CamPosition, &CamDefaultPosition, &matCameraRotation);
	CamPosition += CamTarget;

	D3DXMatrixLookAtLH( matView, &CamPosition, &CamTarget, &CamUp );
	D3DDevice->SetTransform( D3DTS_VIEW, matView );
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