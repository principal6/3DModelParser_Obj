#include "Global.h"

class DirectCamera9
{
public:
	DirectCamera9();
	~DirectCamera9();

	int				CameraType;
	float			CamYaw;
	float			CamRoll;

	D3DXVECTOR3		CamPosition;
	D3DXVECTOR3		CamDefaultPosition;

	D3DXMATRIX GetViewMatrix();
	D3DXMATRIX GetProjectionMatrix();

	void SetDevice(LPDIRECT3DDEVICE9 D3DDevice);
	VOID SetCamera_FirstPerson(float CameraY);
	VOID SetCamera_FreeLook(float CameraX, float CameraY, float CameraZ);
	VOID SetCamera_ThirdPerson(float DistanceHigh, float DistanceFar, float TargetY);
	VOID SetCamera_Static(float CamPX, float CamPY, float CamPZ, float CamTX, float CamTY, float CamTZ);
	VOID UseCamera_FirstPerson();
	VOID UseCamera_FreeLook();
	VOID UseCamera_ThirdPerson(D3DXVECTOR3 SpritePosition);
	VOID UseCamera_Static();
	VOID SetProjection(float ZFar);

	VOID ZoomCamera(bool ZoomIn, float MoveDistance);

	VOID MoveCamera_BackForth(bool MoveBack, float MoveDistance);
	VOID MoveCamera_LeftRight(bool MoveLeft, float MoveDistance);

	VOID RotateCamera_LeftRight(float MouseMovedX, float SpeedFactor);
	VOID RotateCamera_LeftRightRegular(bool RotateLeft, float RotateAngle);
	VOID RotateCamera_UpDown(float MouseMovedY, float SpeedFactor);
	VOID RotateCamera_UpDownRegular(bool RotateUp, float RotateAngle);

	VOID CreateViewFrustum();
	bool IsSphereInFrustum( D3DXVECTOR3* pPosition, float radius);

private:
	LPDIRECT3DDEVICE9		pDevice;

	D3DXPLANE		ViewFrustum[6];
	D3DXMATRIX		matView, matProj;
};