#include "Global.h"
#include "Parser.h"

#define MAX_OBJ_GROUPS			20			// �� - �ִ� �׷� ����
#define MAX_OBJ_MATERIALS		20			// �� - �ִ� ���� ����

#define MAX_OBJ_POSITIONS		100000		// �� - �ִ� ��ġ ��ǥ ����
#define MAX_OBJ_TEXTURES		100000		// �� - �ִ� �ؽ�ó ��ǥ ����
#define MAX_OBJ_NORMALS			120000		// �� - �ִ� ���� ��ǥ ����

#define MAX_OBJ_VERTICES		30000		// �� - �ִ� ���� ����
#define MAX_OBJ_INDICES			20000		// �� - �ִ� ���� ����

#define MAX_OBJ_INSTANCES		1000		// �� - �ִ� �ν��Ͻ� ����


// ���� ����ü ���� - ��ġ(Position), �ؽ�ó ��ǥ(Texture Coordinates), ���� ����(Normal) //
struct VERTEX_OBJ	// ��ġ -> ���� -> �ؽ�ó ������ �ݵ�� ���Ѿ� ��!�ڡڡ�
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 Texture;
	int	PosID;
	int	NormID;
	int	TexID;
};

struct INSTANCE_DATA_OBJ	// HLSL�� �ν��Ͻ� ������ (�׷��� ���� float��)
{
	XMFLOAT4	matModelWorld[4];
};

struct VERTEX_OBJ_BB
{
	XMFLOAT3	Position;
	D3DCOLOR	Color;
};

struct VERTEX_OBJ_NORMAL
{
	XMFLOAT3	Normal;
	D3DCOLOR	Color;
};

#define D3DFVF_VERTEX_OBJ (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define D3DFVF_VERTEX_OBJ_BB (D3DFVF_XYZ | D3DFVF_DIFFUSE)
#define D3DFVF_VERTEX_OBJ_NORMAL (D3DFVF_XYZ | D3DFVF_DIFFUSE)

struct INDEX_OBJ
{
	WORD _0, _1, _2;
};

struct INDEX_OBJ_BB
{
	WORD _0, _1;
};

struct INDEX_OBJ_NORMAL
{
	WORD _0, _1;
};

struct Object_OBJ
{
	ANYNAME		Name;
	ANYNAME		MaterialFileName;
};

struct Group_OBJ
{
	int			GroupID;
	ANYNAME		Name;
	ANYNAME		MaterialName;
	int			MaterialID;

	int			numPositions;
	int			numNormals;
	int			numTextures;

	int			numVertices;
	int			numStartVertexID;
	int			numIndices;
	int			numStartIndexID;
};

struct Material_OBJ
{
	ANYNAME		MaterialName;
	ANYNAME		TextureFileName;
	XMFLOAT3	Color_Ambient;
	XMFLOAT3	Color_Diffuse;
	XMFLOAT3	Color_Specular;
	float		Transparency;
};

struct Instance_OBJ
{
	XMFLOAT3	Translation;
	XMFLOAT3	Rotation;
	XMFLOAT3	Scaling;

	int			CurAnimID;
	float		CurAnimTime;
};

struct BoundingBox_OBJ
{
	XMFLOAT3	Min;
	XMFLOAT3	Max;
};

// OBJ �� Ŭ����
class ModelOBJ {
public:
	ModelOBJ();
	~ModelOBJ();

	bool	CreateModel(LPDIRECT3DDEVICE9 D3DDevice, char* BaseDir, char* FileNameWithoutExtension, LPD3DXEFFECT HLSL = NULL);

	void	AddInstance(XMFLOAT3 Translation, XMFLOAT3 Rotation, XMFLOAT3 Scaling);
	void	AddInstanceEnd();
	void	SetInstance(int InstanceID, XMFLOAT3 Translation, XMFLOAT3 Rotation, XMFLOAT3 Scaling);
	
	void	DrawModel(int InstanceID);
	void	DrawModel_HLSL(int InstanceID, D3DXMATRIX matView, D3DXMATRIX matProj);
	void	DrawMesh_Opaque(int InstanceID);
	void	DrawMesh_Transparent(int InstanceID);
	void	DrawMesh_HLSL_Opaque(int InstanceID, D3DXMATRIX matView, D3DXMATRIX matProj);
	void	DrawMesh_HLSL_Transparent(int InstanceID, D3DXMATRIX matView, D3DXMATRIX matProj);
	void	DrawMesh_HLSLINST_Opaque(D3DXMATRIX matView, D3DXMATRIX matProj);
	void	DrawMesh_HLSLINST_Transparent(D3DXMATRIX matView, D3DXMATRIX matProj);

	void	DrawBoundingBoxes();
	void	DrawNormalVecters(float LenFactor);
	void	DrawBoundingBoxes_HLSLINST(D3DXMATRIX matView, D3DXMATRIX matProj);
	void	DrawNormalVecters_HLSLINST(float LenFactor, D3DXMATRIX matView, D3DXMATRIX matProj);

	PickingRay	GetPickingRay(int MouseX, int MouseY, int ScreenWidth, int ScreenHeight, D3DXMATRIX matView, D3DXMATRIX matProj);
	bool		CheckMouseOverPerInstance(int InstanceID, int MouseX, int MouseY, int ScreenWidth, int ScreenHeight, D3DXMATRIX matView, D3DXMATRIX matProj);
	void		CheckMouseOverFinal();
	
	bool			UseHLSLInstancing;
	int				numInstances;
	Instance_OBJ*	ModelInstances;
	bool*			MouseOverPerInstances;
	float*			DistanceCmp;
	XMFLOAT3*		PickedPosition;
	D3DXMATRIX*		matModelWorld;

private:
	// CreateModel ���� �Լ�
	void		SetBaseDirection(char* Dir);
	bool		OpenMeshFromFile(char* FileName);
	bool		OpenMaterialFromFile(char* FileName);
	HRESULT		CreateTexture(int GroupID);
	void		CreateBoundingBoxes();

	// Instance ���� �Լ�
	void		CreateInstanceBuffer();
	void		UpdateInstanceBuffer();

	// DrawCall ���� �Լ�
	HRESULT		UpdateModel(int GroupID);
	void		SetHLSLTexture(int GroupID);

	char				BaseDir[MAX_NAME_LEN];

	Object_OBJ			ModelObject;
	Group_OBJ*			ModelGroups;
	Material_OBJ*		ModelMaterials;
	BoundingBox_OBJ*	ModelBoundingBoxes;
	int					numGroups;
	int					numMaterials;

	XMFLOAT3*		TPositions;
	XMFLOAT3*		TNormals;
	XMFLOAT2*		TTextures;
	int				numTPositions;
	int				numTNormals;
	int				numTTextures;

	VERTEX_OBJ*		Vertices;
	INDEX_OBJ*		Indices;
	int				numTVertices;
	int				numTIndices;

	LPDIRECT3DDEVICE9		pDevice;
	LPD3DXEFFECT			pHLSL;

	LPDIRECT3DTEXTURE9		ModelTextures[MAX_OBJ_MATERIALS];
	LPDIRECT3DVERTEXBUFFER9	pModelVB;
	LPDIRECT3DINDEXBUFFER9	pModelIB;
	LPDIRECT3DVERTEXBUFFER9	pInstanceVB;

	LPDIRECT3DVERTEXBUFFER9	pBBVB;
	LPDIRECT3DINDEXBUFFER9	pBBIB;

	LPDIRECT3DVERTEXBUFFER9	pNVVB;
	LPDIRECT3DINDEXBUFFER9	pNVIB;

	LPDIRECT3DVERTEXDECLARATION9	pVBDeclaration;
};