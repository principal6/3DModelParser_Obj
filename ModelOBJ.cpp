#include "ModelOBJ.h"

Object_OBJ		ModelObject;
Group_OBJ		ModelGroups[MAX_OBJ_GROUPS+1];
Material_OBJ	ModelMaterials[MAX_OBJ_MATERIALS];
BoundingBox_OBJ	ModelBoundingBoxes[MAX_OBJ_GROUPS];

VERTEX_OBJ		Vertices[MAX_OBJ_VERTICES];
INDEX_OBJ		Indices[MAX_OBJ_INDICES];


bool ModelOBJ::CreateModel(LPDIRECT3DDEVICE9 D3DDevice, char* BaseDir, char* FileNameWithoutExtension)
{
	// 변수 초기화
	memset(&ModelObject, 0, sizeof(ModelObject));
	memset(ModelGroups, 0, sizeof(ModelGroups));
	memset(ModelMaterials, 0, sizeof(ModelMaterials));
	memset(ModelInstances, 0, sizeof(ModelInstances));

	numGroups		= 0;
	memset(MtlFileName, 0, sizeof(MtlFileName));
	numMaterials	= 0;
	
	numTPositions = 0;
	memset(TPositions, 0, sizeof(TPositions));
	numTNormals = 0;
	memset(TNormals, 0, sizeof(TNormals));
	numTTextures = 0;
	memset(TTextures, 0, sizeof(TTextures));

	numTVertices = 0;
	memset(Vertices, 0, sizeof(Vertices));
	numTIndices = 0;
	memset(Indices, 0, sizeof(Indices));

	numInstances	= 0;

	// 모델 파일이 있는 폴더 설정
	SetBaseDirection(BaseDir);

	// 모델 메쉬 불러오기
	char	NewFileName[MAX_NAME_LEN] = {0};
		strcpy_s(NewFileName, FileNameWithoutExtension);
		strcat_s(NewFileName, ".OBJ");
	OpenMeshFromFile(NewFileName);

	strcpy_s(NewFileName, ModelObject.MaterialFileName);
	OpenMaterialFromFile(NewFileName);

	for (int i = 0; i < numGroups; i++)
	{
		for (int j = 0; j < numMaterials; j++)
		{
			if ( FindString(ModelGroups[i].MaterialName, ModelMaterials[j].MaterialName) )
			{
				ModelGroups[i].MaterialID = j;
			}
		}
	}

	// 모델 텍스처 불러오기
	for (int i = 0; i < numGroups; i++)
	{
		SetTexture(D3DDevice, i);
	}

	// 바운딩 박스 생성하기
	CreateBoundingBoxes();

	return true;
}

void ModelOBJ::CreateBoundingBoxes()
{
	XMFLOAT3 Min = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 Max = XMFLOAT3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < numGroups; i++)
	{
		int nStartVert = ModelGroups[i].numStartVertexID;
		int nVerts = ModelGroups[i].numVertices;


		Min = Vertices[nStartVert].Position;
		Max = Vertices[nStartVert].Position;

		for (int j = nStartVert+1; j < nStartVert+nVerts; j++)
		{
			if ( Min.x > Vertices[j].Position.x)
				Min.x = Vertices[j].Position.x;
			if ( Min.y > Vertices[j].Position.y)
				Min.y = Vertices[j].Position.y;
			if ( Min.z > Vertices[j].Position.z)
				Min.z = Vertices[j].Position.z;

			if ( Max.x < Vertices[j].Position.x)
				Max.x = Vertices[j].Position.x;
			if ( Max.y < Vertices[j].Position.y)
				Max.y = Vertices[j].Position.y;
			if ( Max.z < Vertices[j].Position.z)
				Max.z = Vertices[j].Position.z;
		}

		ModelBoundingBoxes[i].Min = Min;
		ModelBoundingBoxes[i].Max = Max;
	}

	return;
}

void ModelOBJ::AddInstance(XMFLOAT3 Translation, XMFLOAT3 Rotation, XMFLOAT3 Scaling)
{
	numInstances++;

	memset(&ModelInstances[numInstances-1], 0, sizeof(ModelInstances[numInstances-1]));

	ModelInstances[numInstances-1].Translation = Translation;
	ModelInstances[numInstances-1].Rotation = Rotation;
	ModelInstances[numInstances-1].Scaling = Scaling;

	return;
}

void ModelOBJ::SetInstance(int InstanceID, XMFLOAT3 Translation, XMFLOAT3 Rotation, XMFLOAT3 Scaling)
{
	ModelInstances[InstanceID].Translation = Translation;
	ModelInstances[InstanceID].Rotation = Rotation;
	ModelInstances[InstanceID].Scaling = Scaling;

	return;
}

void ModelOBJ::SetBaseDirection(char* Dir)
{
	strcpy_s(BaseDir, Dir);
	return;
}

bool ModelOBJ::OpenMeshFromFile(char* FileName)
{
	FILE*	fp;									// 불러올 파일
	char	cBuf = 0;							// 파일에서 불러올 한 문자
	char	NewFileName[MAX_NAME_LEN] = {0};
	
	strcpy_s(NewFileName, BaseDir);
	strcat_s(NewFileName, FileName);

	if( fopen_s(&fp, NewFileName, "rb") )
		return false;

	int		StackCount = 0;
	char	StackString[MAX_PARSE_LINE] = {0};

	while(!feof(fp))
	{
		cBuf = fgetc(fp);

		switch (cBuf)
		{
			case '#':
				cBuf = fgetc(fp);
				if (cBuf == ' ')			// '# ' = 주석
				{		
					do
					{
						cBuf = fgetc(fp);
					} while (cBuf != 0x0A);	// 줄이 바뀌는 부분까지 읽기
				}
				break;

			case 'm':
				cBuf = fgetc(fp);
				if (cBuf == 't')
				{
					cBuf = fgetc(fp);
					if (cBuf == 'l')
					{
						cBuf = fgetc(fp);
						if (cBuf == 'l')
						{
							cBuf = fgetc(fp);
							if (cBuf == 'i')
							{
								cBuf = fgetc(fp);
								if (cBuf == 'b')
								{
									cBuf = fgetc(fp);
									if (cBuf == ' ')	// 'mtllib ' = mtl 파일 정보
									{
										StackCount = 0;
										memset( StackString, 0, sizeof(StackString) );
									
										do
										{
											cBuf = fgetc(fp);
											StackString[StackCount++] = cBuf;
										} while (cBuf != 0x0A && cBuf != ' ');

										strcpy_s( ModelObject.MaterialFileName, StringTrim_OBJ(StackString) );
									}
								}
							}
						}
					}
				}
				break;

			case 'o':
				cBuf = fgetc(fp);
				if (cBuf == ' ')	// 'o ' = 오브젝트 이름만 담겨 있음 ....
				{
					StackCount = 0;
					memset( StackString, 0, sizeof(StackString) );
									
					do
					{
						cBuf = fgetc(fp);
						StackString[StackCount++] = cBuf;
					} while (cBuf != 0x0A && cBuf != ' ');

					strcpy_s( ModelObject.Name, StringTrim_OBJ(StackString) );
				}
				break;

			case 'v':
				cBuf = fgetc(fp);
				switch (cBuf)
				{
					case ' ':	// 'v ' = 정점
						StackCount = 0;
						memset( StackString, 0, sizeof(StackString) );
									
						do
						{
							cBuf = fgetc(fp);
							StackString[StackCount++] = cBuf;
						} while (cBuf != 0x0A);	// 줄이 바뀌는 부분까지 읽기
						GetFloatFromLine(StackString, " ", "/");
						
						numTPositions++;
						ModelGroups[numGroups].numPositions++;
						TPositions[numTPositions-1].x = ParseFloats[0];
						TPositions[numTPositions-1].y = ParseFloats[1];
						TPositions[numTPositions-1].z = ParseFloats[2];
						break;

					case 't':	// 'vt' = 텍스처
						StackCount = 0;
						memset( StackString, 0, sizeof(StackString) );
									
						do
						{
							cBuf = fgetc(fp);
							StackString[StackCount++] = cBuf;
						} while (cBuf != 0x0A);	// 줄이 바뀌는 부분까지 읽기
						GetFloatFromLine(StackString, " ", "/");

						numTTextures++;
						ModelGroups[numGroups].numTextures++;
						TTextures[numTTextures-1].x = ParseFloats[0];
						TTextures[numTTextures-1].y = ParseFloats[1];
						break;

					case 'n':	// 'vn' = 법선
						StackCount = 0;
						memset( StackString, 0, sizeof(StackString) );
									
						do
						{
							cBuf = fgetc(fp);
							StackString[StackCount++] = cBuf;
						} while (cBuf != 0x0A);	// 줄이 바뀌는 부분까지 읽기
						GetFloatFromLine(StackString, " ", "/");

						numTNormals++;
						ModelGroups[numGroups].numNormals++;
						TNormals[numTNormals-1].x = ParseFloats[0];
						TNormals[numTNormals-1].y = ParseFloats[1];
						TNormals[numTNormals-1].z = ParseFloats[2];
						break;

				}
				break;

			case 'g':
				cBuf = fgetc(fp);
				if (cBuf == ' ')	// 'g ' = 그룹 (즉, 메쉬 서브셋)
				{
					StackCount = 0;
					memset( StackString, 0, sizeof(StackString) );
									
					do
					{
						cBuf = fgetc(fp);
						StackString[StackCount++] = cBuf;
					} while (cBuf != 0x0A && cBuf != ' ');

					numGroups++;
					strcpy_s( ModelGroups[numGroups-1].Name, StringTrim_OBJ(StackString) );
				}
				break;

			case 'u':
				cBuf = fgetc(fp);
				if (cBuf == 's')
				{
					cBuf = fgetc(fp);
					if (cBuf == 'e')
					{
						cBuf = fgetc(fp);
						if (cBuf == 'm')
						{
							cBuf = fgetc(fp);
							if (cBuf == 't')
							{
								cBuf = fgetc(fp);
								if (cBuf == 'l')
								{
									cBuf = fgetc(fp);
									if (cBuf == ' ')	// 'g' - 'usemtl ' 재질 이름
									{
										StackCount = 0;
										memset( StackString, 0, sizeof(StackString) );
									
										do
										{
											cBuf = fgetc(fp);
											StackString[StackCount++] = cBuf;
										} while (cBuf != 0x0A && cBuf != ' ');

										strcpy_s( ModelGroups[numGroups-1].MaterialName, StringTrim_OBJ(StackString) );
									}
								}
							}
						}
					}
				}
				break;

			case 'f':
				cBuf = fgetc(fp);
				if (cBuf == ' ')	// 'f ' = 면 정보
				{
					StackCount = 0;
					memset( StackString, 0, sizeof(StackString) );
									
					do
					{
						cBuf = fgetc(fp);
						StackString[StackCount++] = cBuf;
					} while (cBuf != 0x0A);	// 줄이 바뀌는 부분까지 읽기
					GetFloatFromLine(StackString, " ", "/");

					int PosID[3] = {0};
					int TexID[3] = {0};
					int NormID[3] = {0};
					int	VertID[3] = {0};

					PosID[0] = (int)ParseFloats[0]-1;	// 시작 숫자를 1이 아니라 0으로 맞추기!
					TexID[0] = (int)ParseFloats[1]-1;
					NormID[0] = (int)ParseFloats[2]-1;

					PosID[1] = (int)ParseFloats[3]-1;
					TexID[1] = (int)ParseFloats[4]-1;
					NormID[1] = (int)ParseFloats[5]-1;

					PosID[2] = (int)ParseFloats[6]-1;
					TexID[2] = (int)ParseFloats[7]-1;
					NormID[2] = (int)ParseFloats[8]-1;

					bool	Identical[3];
					int		IdenticalVertID[3] = {0};
					memset( Identical, false, sizeof(Identical) );

					if ( ModelGroups[numGroups-1].numVertices > 0 )
					{
						for (int j = 0; j < 3; j++)
						{
							for (int i = 0; i < ModelGroups[numGroups-1].numVertices; i++)
							{
								if (Vertices[ModelGroups[numGroups-1].numStartVertexID + i].PosID == PosID[j])
								{
									if (Vertices[ModelGroups[numGroups-1].numStartVertexID + i].TexID == TexID[j])
									{
										if (Vertices[ModelGroups[numGroups-1].numStartVertexID + i].NormID == NormID[j])
										{
											Identical[j] = true;
											IdenticalVertID[j] = i;
											//IdenticalVertID[j] = ModelGroups[numGroups-1].numStartVertexID + i;
										}
									}
								}
							}
						}
					}

					for (int i = 0; i < 3; i++)
					{
						if (Identical[i] == false)
						{
							numTVertices++;
							ModelGroups[numGroups-1].numVertices++;
							Vertices[ModelGroups[numGroups-1].numStartVertexID + ModelGroups[numGroups-1].numVertices - 1].Position =
								TPositions[PosID[i]];
							Vertices[ModelGroups[numGroups-1].numStartVertexID + ModelGroups[numGroups-1].numVertices - 1].Texture =
								TTextures[TexID[i]];
							Vertices[ModelGroups[numGroups-1].numStartVertexID + ModelGroups[numGroups-1].numVertices - 1].Normal =
								TNormals[NormID[i]];
							Vertices[ModelGroups[numGroups-1].numStartVertexID + ModelGroups[numGroups-1].numVertices - 1].PosID = PosID[i];
							Vertices[ModelGroups[numGroups-1].numStartVertexID + ModelGroups[numGroups-1].numVertices - 1].TexID = TexID[i];
							Vertices[ModelGroups[numGroups-1].numStartVertexID + ModelGroups[numGroups-1].numVertices - 1].NormID = NormID[i];

							ModelGroups[numGroups].numStartVertexID =
								ModelGroups[numGroups-1].numVertices + ModelGroups[numGroups-1].numStartVertexID;

							VertID[i] = ModelGroups[numGroups-1].numVertices - 1;
							//VertID[i] = ModelGroups[numGroups-1].numVertices + ModelGroups[numGroups-1].numStartVertexID - 1;
						}
						else if(Identical[i] == true)
						{
							VertID[i] = IdenticalVertID[i];
						}
					}

					numTIndices++;
					ModelGroups[numGroups-1].numIndices++;
					Indices[ModelGroups[numGroups-1].numStartIndexID + ModelGroups[numGroups-1].numIndices - 1]._0 = VertID[0];
					Indices[ModelGroups[numGroups-1].numStartIndexID + ModelGroups[numGroups-1].numIndices - 1]._1 = VertID[1];
					Indices[ModelGroups[numGroups-1].numStartIndexID + ModelGroups[numGroups-1].numIndices - 1]._2 = VertID[2];

					ModelGroups[numGroups].numStartIndexID =
						ModelGroups[numGroups-1].numIndices + ModelGroups[numGroups-1].numStartIndexID;

				}
				break;
		}
	}	// WHILE문 종료

	fclose(fp);

	return true;
}

bool ModelOBJ::OpenMaterialFromFile(char* FileName)
{
	FILE*	fp;									// 불러올 파일
	char	cBuf = 0;							// 파일에서 불러올 한 문자
	char	NewFileName[MAX_NAME_LEN] = {0};
	
	strcpy_s(NewFileName, BaseDir);
	strcat_s(NewFileName, FileName);

	if( fopen_s(&fp, NewFileName, "rb") )
		return false;

	int		StackCount = 0;
	char	StackString[MAX_PARSE_LINE] = {0};

	while(!feof(fp))
	{
		cBuf = fgetc(fp);

		switch (cBuf)
		{
			case '#':
				cBuf = fgetc(fp);
				if (cBuf == ' ')			// '# ' = 주석
				{		
					do
					{
						cBuf = fgetc(fp);
					} while (cBuf != 0x0A);	// 줄이 바뀌는 부분까지 읽기
				}
				break;

			case 'n':
				cBuf = fgetc(fp);
				if (cBuf == 'e')
				{
					cBuf = fgetc(fp);
					if (cBuf == 'w')
					{
						cBuf = fgetc(fp);
						if (cBuf == 'm')
						{
							cBuf = fgetc(fp);
							if (cBuf == 't')
							{
								cBuf = fgetc(fp);
								if (cBuf == 'l')
								{
									cBuf = fgetc(fp);
									if (cBuf == ' ')	// 'newmtl ' = 재질 이름
									{
										StackCount = 0;
										memset( StackString, 0, sizeof(StackString) );
									
										do
										{
											cBuf = fgetc(fp);
											StackString[StackCount++] = cBuf;
										} while (cBuf != 0x0A && cBuf != ' ');

										numMaterials++;
										strcpy_s( ModelMaterials[numMaterials-1].MaterialName, StringTrim_OBJ(StackString) );
									}
								}
							}
						}
					}
				}
				break;

			case 'm':
				cBuf = fgetc(fp);
				if (cBuf == 'a')
				{
					cBuf = fgetc(fp);
					if (cBuf == 'p')
					{
						cBuf = fgetc(fp);
						if (cBuf == '_')
						{
							cBuf = fgetc(fp);
							if (cBuf == 'K')
							{
								cBuf = fgetc(fp);
								if (cBuf == 'd')
								{
									cBuf = fgetc(fp);
									if (cBuf == ' ')	// 'map_Kd ' = 텍스처 파일 이름★
									{
										StackCount = 0;
										memset( StackString, 0, sizeof(StackString) );
									
										do
										{
											cBuf = fgetc(fp);
											StackString[StackCount++] = cBuf;
										} while (cBuf != 0x0A && cBuf != ' ');

										strcpy_s( ModelMaterials[numMaterials-1].TextureFileName, StringTrim_OBJ(StackString) );
									}
								}
							}
						}
					}
				}
				break;

			case 'd':
				cBuf = fgetc(fp);
				if (cBuf == ' ')	// 'd ' = 투명도★
				{
					StackCount = 0;
					memset( StackString, 0, sizeof(StackString) );

					do
					{
						cBuf = fgetc(fp);
						StackString[StackCount++] = cBuf;
					} while (cBuf != 0x0A);

					GetFloatFromLine(StackString, " ", "/");
						
					ModelMaterials[numMaterials-1].Transparency = ParseFloats[0];
				}
				break;

			case 'K':
				cBuf = fgetc(fp);
				switch (cBuf)
				{
					case 'a':	// 'Ka ' = 환경광(Ambient color)★
						cBuf = fgetc(fp);
						if (cBuf == ' ')
						{
							StackCount = 0;
							memset( StackString, 0, sizeof(StackString) );
									
							do
							{
								cBuf = fgetc(fp);
								StackString[StackCount++] = cBuf;
							} while (cBuf != 0x0A);

							GetFloatFromLine(StackString, " ", "/");
						
							ModelMaterials[numMaterials-1].Color_Ambient.x = ParseFloats[0];
							ModelMaterials[numMaterials-1].Color_Ambient.y = ParseFloats[1];
							ModelMaterials[numMaterials-1].Color_Ambient.z = ParseFloats[2];
						}
						break;

					case 'd':	// 'Kd ' = 확산광(Diffuse color)★
						cBuf = fgetc(fp);
						if (cBuf == ' ')
						{
							StackCount = 0;
							memset( StackString, 0, sizeof(StackString) );
									
							do
							{
								cBuf = fgetc(fp);
								StackString[StackCount++] = cBuf;
							} while (cBuf != 0x0A);

							GetFloatFromLine(StackString, " ", "/");
						
							ModelMaterials[numMaterials-1].Color_Diffuse.x = ParseFloats[0];
							ModelMaterials[numMaterials-1].Color_Diffuse.y = ParseFloats[1];
							ModelMaterials[numMaterials-1].Color_Diffuse.z = ParseFloats[2];
						}
						break;

					case 's':	// 'Ks ' = 반사광(Specular color)★
						cBuf = fgetc(fp);
						if (cBuf == ' ')
						{
							StackCount = 0;
							memset( StackString, 0, sizeof(StackString) );
									
							do
							{
								cBuf = fgetc(fp);
								StackString[StackCount++] = cBuf;
							} while (cBuf != 0x0A);

							GetFloatFromLine(StackString, " ", "/");
						
							ModelMaterials[numMaterials-1].Color_Specular.x = ParseFloats[0];
							ModelMaterials[numMaterials-1].Color_Specular.y = ParseFloats[1];
							ModelMaterials[numMaterials-1].Color_Specular.z = ParseFloats[2];
						}
						break;
				}
				break;
		}
	}	// WHILE문 종료

	fclose(fp);

	return true;
}

HRESULT ModelOBJ::UpdateVertices(LPDIRECT3DDEVICE9 D3DDevice, int GroupID)
{
	if( g_pModelVB != NULL )
		g_pModelVB->Release();

	if( g_pModelIB != NULL )
		g_pModelIB->Release();

	int numVertices = ModelGroups[GroupID].numVertices;
	int numIndices = ModelGroups[GroupID].numIndices;
	int	numStartVID =  ModelGroups[GroupID].numStartVertexID;
	int	numStartIID =  ModelGroups[GroupID].numStartIndexID;

	// 정점 버퍼 업데이트!
	VERTEX_OBJ *NewVertices = new VERTEX_OBJ[numVertices];

		for (int i = 0; i < numVertices; i++)
		{
			NewVertices[i].Position.x = Vertices[numStartVID + i].Position.x;
			NewVertices[i].Position.y = Vertices[numStartVID + i].Position.y;
			NewVertices[i].Position.z = Vertices[numStartVID + i].Position.z;
			NewVertices[i].Texture.x = Vertices[numStartVID + i].Texture.x;
			NewVertices[i].Texture.y = Vertices[numStartVID + i].Texture.y;
			NewVertices[i].Normal.x = Vertices[numStartVID + i].Normal.x;
			NewVertices[i].Normal.y = Vertices[numStartVID + i].Normal.y;
			NewVertices[i].Normal.z = Vertices[numStartVID + i].Normal.z;
		}

		int SizeOfVertices = sizeof(VERTEX_OBJ)*numVertices;
		if (FAILED(D3DDevice->CreateVertexBuffer(SizeOfVertices, 0, D3DFVF_VERTEX_OBJ, D3DPOOL_DEFAULT, &g_pModelVB, NULL)))
			return E_FAIL;
	
		VOID* pVertices;
		if (FAILED(g_pModelVB->Lock(0, SizeOfVertices, (void**)&pVertices, 0)))
			return E_FAIL;
		memcpy(pVertices, NewVertices, SizeOfVertices);
		g_pModelVB->Unlock();

	delete[] NewVertices;

	// 색인 버퍼 업데이트!
	INDEX_OBJ *NewIndices = new INDEX_OBJ[numIndices];

		for (int i = 0; i < numIndices; i++)
		{
			NewIndices[i]._0 = Indices[numStartIID + i]._0;
			NewIndices[i]._1 = Indices[numStartIID + i]._1;
			NewIndices[i]._2 = Indices[numStartIID + i]._2;
		}

		int SizeOfIndices = sizeof(INDEX_OBJ)*numIndices;
		if (FAILED(D3DDevice->CreateIndexBuffer(SizeOfIndices, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pModelIB, NULL)))
			return E_FAIL;

		VOID* pIndices;
		if (FAILED(g_pModelIB->Lock(0, SizeOfIndices, (void **)&pIndices, 0)))
			return E_FAIL;
		memcpy(pIndices, NewIndices, SizeOfIndices);
		g_pModelIB->Unlock();
	
	delete[] NewIndices;

	return S_OK;	// 함수 종료!
}

HRESULT ModelOBJ::SetTexture(LPDIRECT3DDEVICE9 D3DDevice, int GroupID)
{
	if ( ModelTextures[GroupID] != NULL)
		ModelTextures[GroupID]->Release();

	// 텍스처 불러오기
	char	NewFileName[MAX_NAME_LEN] = {0};
	strcpy_s(NewFileName, BaseDir);

	int	MaterialID = 0;
	for (int i = 0; i < numMaterials; i++)
	{
		if ( FindString(ModelGroups[GroupID].MaterialName, ModelMaterials[i].MaterialName) )
			MaterialID = i;
	}
	strcat_s(NewFileName, ModelMaterials[MaterialID].TextureFileName);

	D3DXCreateTextureFromFile(D3DDevice, NewFileName, &ModelTextures[GroupID]);

	return S_OK;
}

void ModelOBJ::DrawBoundingBoxes(LPDIRECT3DDEVICE9 D3DDevice)
{
	for (int i = 0; i < numGroups; i++)
	{
		if( g_pModelVB != NULL )
			g_pModelVB->Release();

		if( g_pModelIB != NULL )
			g_pModelIB->Release();

		int numVertices = 8;	// 상자니까 점 8개 필요
		VERTEX_OBJ_BB *NewVertices = new VERTEX_OBJ_BB[numVertices];

			float XLen = ModelBoundingBoxes[i].Max.x - ModelBoundingBoxes[i].Min.x;
			float YLen = ModelBoundingBoxes[i].Max.y - ModelBoundingBoxes[i].Min.y;
			float ZLen = ModelBoundingBoxes[i].Max.z - ModelBoundingBoxes[i].Min.z;

			NewVertices[0].Position = ModelBoundingBoxes[i].Min;
			NewVertices[1].Position = ModelBoundingBoxes[i].Min;
				NewVertices[1].Position.x += XLen;
			NewVertices[2].Position = ModelBoundingBoxes[i].Min;
				NewVertices[2].Position.x += XLen;
				NewVertices[2].Position.z += ZLen;
			NewVertices[3].Position = ModelBoundingBoxes[i].Min;
				NewVertices[3].Position.z += ZLen;
			NewVertices[4].Position = ModelBoundingBoxes[i].Min;
				NewVertices[4].Position.y += YLen;
			NewVertices[5].Position = ModelBoundingBoxes[i].Min;
				NewVertices[5].Position.y += YLen;
				NewVertices[5].Position.x += XLen;
			NewVertices[6].Position = ModelBoundingBoxes[i].Min;
				NewVertices[6].Position.y += YLen;
				NewVertices[6].Position.x += XLen;
				NewVertices[6].Position.z += ZLen;
			NewVertices[7].Position = ModelBoundingBoxes[i].Min;
				NewVertices[7].Position.y += YLen;
				NewVertices[7].Position.z += ZLen;

			int SizeOfVertices = sizeof(VERTEX_OBJ_BB)*numVertices;
			if (FAILED(D3DDevice->CreateVertexBuffer(SizeOfVertices, 0, D3DFVF_VERTEX_OBJ_BB, D3DPOOL_DEFAULT, &g_pModelVB, NULL)))
				return;
	
			VOID* pVertices;
			if (FAILED(g_pModelVB->Lock(0, SizeOfVertices, (void**)&pVertices, 0)))
				return;
			memcpy(pVertices, NewVertices, SizeOfVertices);
			g_pModelVB->Unlock();

		delete[] NewVertices;
		

		int numIndices = 12;	// 상자니까 모서리 12개 필요
		INDEX_OBJ_BB *NewIndices = new INDEX_OBJ_BB[numIndices];

			NewIndices[0]._0 = 0, NewIndices[0]._1 = 1;	// 바닥
			NewIndices[1]._0 = 1, NewIndices[1]._1 = 2;
			NewIndices[2]._0 = 2, NewIndices[2]._1 = 3;
			NewIndices[3]._0 = 3, NewIndices[3]._1 = 0;
			NewIndices[4]._0 = 0, NewIndices[4]._1 = 4;	// 기둥
			NewIndices[5]._0 = 1, NewIndices[5]._1 = 5;
			NewIndices[6]._0 = 2, NewIndices[6]._1 = 6;
			NewIndices[7]._0 = 3, NewIndices[7]._1 = 7;
			NewIndices[8]._0 = 4, NewIndices[8]._1 = 5;	// 천장
			NewIndices[9]._0 = 5, NewIndices[9]._1 = 6;
			NewIndices[10]._0 = 6, NewIndices[10]._1 = 7;
			NewIndices[11]._0 = 7, NewIndices[11]._1 = 4;

			int SizeOfIndices = sizeof(INDEX_OBJ_BB)*numIndices;
			if (FAILED(D3DDevice->CreateIndexBuffer(SizeOfIndices, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pModelIB, NULL)))
				return;

			VOID* pIndices;
			if (FAILED(g_pModelIB->Lock(0, SizeOfIndices, (void **)&pIndices, 0)))
				return;
			memcpy(pIndices, NewIndices, SizeOfIndices);
			g_pModelIB->Unlock();
	
		delete[] NewIndices;


		D3DDevice->SetStreamSource(0, g_pModelVB, 0, sizeof(VERTEX_OBJ_BB));
		D3DDevice->SetFVF(D3DFVF_VERTEX_OBJ_BB);
		D3DDevice->SetIndices(g_pModelIB);

		D3DDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, numVertices, 0, numIndices);
	}

	return;
}

void ModelOBJ::DrawModel(LPDIRECT3DDEVICE9 D3DDevice)
{
	// 모델의 각 메쉬를 그린다!
	for (int i = 0; i < numGroups; i++)
	{
		UpdateVertices(D3DDevice, i);

		// 재질을 설정한다.
		D3DMATERIAL9 mtrl;
		ZeroMemory( &mtrl, sizeof( D3DMATERIAL9 ) );
		mtrl.Diffuse.r = ModelMaterials[ModelGroups[i].MaterialID].Color_Diffuse.x;
		mtrl.Diffuse.g = ModelMaterials[ModelGroups[i].MaterialID].Color_Diffuse.y;
		mtrl.Diffuse.b = ModelMaterials[ModelGroups[i].MaterialID].Color_Diffuse.z;
		mtrl.Diffuse.a = ModelMaterials[ModelGroups[i].MaterialID].Transparency;
		mtrl.Ambient.r = ModelMaterials[ModelGroups[i].MaterialID].Color_Ambient.x;
		mtrl.Ambient.g = ModelMaterials[ModelGroups[i].MaterialID].Color_Ambient.y;
		mtrl.Ambient.b = ModelMaterials[ModelGroups[i].MaterialID].Color_Ambient.z;
		mtrl.Ambient.a = ModelMaterials[ModelGroups[i].MaterialID].Transparency;
		mtrl.Specular.r = ModelMaterials[ModelGroups[i].MaterialID].Color_Specular.x;
		mtrl.Specular.g = ModelMaterials[ModelGroups[i].MaterialID].Color_Specular.y;
		mtrl.Specular.b = ModelMaterials[ModelGroups[i].MaterialID].Color_Specular.z;
		mtrl.Specular.a = ModelMaterials[ModelGroups[i].MaterialID].Transparency;
		mtrl.Power = 5.0f;
		D3DDevice->SetMaterial( &mtrl );

		D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
		D3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE);

		D3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		D3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
		D3DDevice->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);

		D3DDevice->SetTexture(0, ModelTextures[i]);
		D3DDevice->SetStreamSource(0, g_pModelVB, 0, sizeof(VERTEX_OBJ));
		D3DDevice->SetFVF(D3DFVF_VERTEX_OBJ);
		D3DDevice->SetIndices(g_pModelIB);

		D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, ModelGroups[i].numVertices, 0, ModelGroups[i].numIndices);
	}

	return;
}

void ModelOBJ::Destroy()
{
	for (int i = 0; i < numMaterials; i++)
	{
		if ( ModelTextures[i] != NULL)
			ModelTextures[i]->Release();
	}

	return;
}