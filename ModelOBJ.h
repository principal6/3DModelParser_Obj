#include "Global.h"
#include "Parser.h"

#define MAX_OBJ_GROUPS			10			// �� - �ִ� �׷� ����
#define MAX_OBJ_MATERIALS		10			// �� - �ִ� ���� ����

#define MAX_OBJ_POSITIONS		76240		// �� - �ִ� ��ġ ��ǥ ����
#define MAX_OBJ_TEXTURES		76351		// �� - �ִ� �ؽ�ó ��ǥ ����
#define MAX_OBJ_NORMALS			93540		// �� - �ִ� ���� ��ǥ ����

#define MAX_OBJ_VERTICES		30000		// �� - �ִ� ���� ����
#define MAX_OBJ_INDICES			20000		// �� - �ִ� ���� ����

#define MAX_OBJ_INSTANCES		100			// �� - �ִ� �ν��Ͻ� ����


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

struct VERTEX_OBJ_BB
{
	XMFLOAT3 Position;
};

struct VERTEX_OBJ_NORMAL
{
	XMFLOAT3 Normal;
};


#define D3DFVF_VERTEX_OBJ (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define D3DFVF_VERTEX_OBJ_BB (D3DFVF_XYZ)
#define D3DFVF_VERTEX_OBJ_NORMAL (D3DFVF_XYZ)

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

	int						numGroups;
	ANYNAME					MtlFileName;

	int						numMaterials;

	int						numTPositions;
	XMFLOAT3				TPositions[MAX_OBJ_POSITIONS];
	int						numTNormals;
	XMFLOAT3				TNormals[MAX_OBJ_NORMALS];
	int						numTTextures;
	XMFLOAT2				TTextures[MAX_OBJ_TEXTURES];
	int						numTVertices;
	int						numTIndices;

	int						numInstances;
	Instance_OBJ			ModelInstances[MAX_OBJ_INSTANCES];
	bool					MouseOver;

	char					BaseDir[MAX_NAME_LEN];
	void ModelOBJ::SetBaseDirection(char* Dir);

	LPDIRECT3DTEXTURE9		ModelTextures[MAX_OBJ_MATERIALS];
	bool ModelOBJ::CreateModel(LPDIRECT3DDEVICE9 D3DDevice, char* BaseDir, char* FileNameWithoutExtension);
		bool ModelOBJ::OpenMeshFromFile(char* FileName);
		bool ModelOBJ::OpenMaterialFromFile(char* FileName);
	void ModelOBJ::CreateBoundingBoxes();

	void ModelOBJ::AddInstance(XMFLOAT3 Translation, XMFLOAT3 Rotation, XMFLOAT3 Scaling);
	void ModelOBJ::SetInstance(int InstanceID, XMFLOAT3 Translation, XMFLOAT3 Rotation, XMFLOAT3 Scaling);

	void ModelOBJ::DrawModel(LPDIRECT3DDEVICE9 D3DDevice);
	void ModelOBJ::DrawMesh_Opaque(LPDIRECT3DDEVICE9 D3DDevice);
	void ModelOBJ::DrawMesh_Transparent(LPDIRECT3DDEVICE9 D3DDevice);
	void ModelOBJ::DrawBoundingBoxes(LPDIRECT3DDEVICE9 D3DDevice);
	HRESULT ModelOBJ::DrawNormalVecters(LPDIRECT3DDEVICE9 D3DDevice, float LenFactor);
	bool ModelOBJ::CheckMouseOver(PickingRay PR, int MouseX, int MouseY);

	HRESULT ModelOBJ::SetTexture(LPDIRECT3DDEVICE9 D3DDevice, int GroupID);
	HRESULT ModelOBJ::UpdateVertices(LPDIRECT3DDEVICE9 D3DDevice, int GroupID);

private:
	Object_OBJ		ModelObject;
	Group_OBJ		ModelGroups[MAX_OBJ_GROUPS+1];
	Material_OBJ	ModelMaterials[MAX_OBJ_MATERIALS];
	BoundingBox_OBJ	ModelBoundingBoxes[MAX_OBJ_GROUPS];

	VERTEX_OBJ		Vertices[MAX_OBJ_VERTICES];
	INDEX_OBJ		Indices[MAX_OBJ_INDICES];
};