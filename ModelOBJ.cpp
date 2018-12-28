#include "ModelOBJ.h"

ModelOBJ::ModelOBJ()
{
	// ���� �ʱ�ȭ
	memset(&ModelObject, 0, sizeof(ModelObject));
	memset(ModelGroups, 0, sizeof(ModelGroups));
	memset(ModelMaterials, 0, sizeof(ModelMaterials));
	memset(ModelInstances, 0, sizeof(ModelInstances));
	memset(MouseOverPerInstances, 0, sizeof(MouseOverPerInstances));
	
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
}

ModelOBJ::~ModelOBJ()
{
	for (int i = 0; i < numMaterials; i++)
	{
		SAFE_RELEASE(ModelTextures[i]);
	}
}

bool ModelOBJ::CreateModel(LPDIRECT3DDEVICE9 D3DDevice, char* BaseDir, char* FileNameWithoutExtension)
{
	// �� ������ �ִ� ���� ����
	SetBaseDirection(BaseDir);

	// �� �޽� �ҷ�����
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

	// �� �ؽ�ó �ҷ�����
	for (int i = 0; i < numGroups; i++)
	{
		SetTexture(D3DDevice, i);
	}

	// �ٿ�� �ڽ� �����ϱ�
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
	FILE*	fp;									// �ҷ��� ����
	char	cBuf = 0;							// ���Ͽ��� �ҷ��� �� ����
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
				if (cBuf == ' ')			// '# ' = �ּ�
				{		
					do
					{
						cBuf = fgetc(fp);
					} while (cBuf != 0x0A);	// ���� �ٲ�� �κб��� �б�
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
									if (cBuf == ' ')	// 'mtllib ' = mtl ���� ����
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
				if (cBuf == ' ')	// 'o ' = ������Ʈ �̸��� ��� ���� ....
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
					case ' ':	// 'v ' = ����
						StackCount = 0;
						memset( StackString, 0, sizeof(StackString) );
									
						do
						{
							cBuf = fgetc(fp);
							StackString[StackCount++] = cBuf;
						} while (cBuf != 0x0A);	// ���� �ٲ�� �κб��� �б�
						GetFloatFromLine(StackString, " ", "/");
						
						numTPositions++;
						ModelGroups[numGroups].numPositions++;
						TPositions[numTPositions-1].x = ParseFloats[0];
						TPositions[numTPositions-1].y = ParseFloats[1];
						TPositions[numTPositions-1].z = ParseFloats[2];
						break;

					case 't':	// 'vt' = �ؽ�ó
						StackCount = 0;
						memset( StackString, 0, sizeof(StackString) );
									
						do
						{
							cBuf = fgetc(fp);
							StackString[StackCount++] = cBuf;
						} while (cBuf != 0x0A);	// ���� �ٲ�� �κб��� �б�
						GetFloatFromLine(StackString, " ", "/");

						numTTextures++;
						ModelGroups[numGroups].numTextures++;
						TTextures[numTTextures-1].x = ParseFloats[0];
						TTextures[numTTextures-1].y = ParseFloats[1];
						break;

					case 'n':	// 'vn' = ����
						StackCount = 0;
						memset( StackString, 0, sizeof(StackString) );
									
						do
						{
							cBuf = fgetc(fp);
							StackString[StackCount++] = cBuf;
						} while (cBuf != 0x0A);	// ���� �ٲ�� �κб��� �б�
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
				if (cBuf == ' ')	// 'g ' = �׷� (��, �޽� �����)
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
									if (cBuf == ' ')	// 'g' - 'usemtl ' ���� �̸�
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
				if (cBuf == ' ')	// 'f ' = �� ����
				{
					StackCount = 0;
					memset( StackString, 0, sizeof(StackString) );
									
					do
					{
						cBuf = fgetc(fp);
						StackString[StackCount++] = cBuf;
					} while (cBuf != 0x0A);	// ���� �ٲ�� �κб��� �б�
					GetFloatFromLine(StackString, " ", "/");

					int PosID[3] = {0};
					int TexID[3] = {0};
					int NormID[3] = {0};
					int	VertID[3] = {0};

					PosID[0] = (int)ParseFloats[0]-1;	// ���� ���ڸ� 1�� �ƴ϶� 0���� ���߱�!
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
	}	// WHILE�� ����

	fclose(fp);

	return true;
}

bool ModelOBJ::OpenMaterialFromFile(char* FileName)
{
	FILE*	fp;									// �ҷ��� ����
	char	cBuf = 0;							// ���Ͽ��� �ҷ��� �� ����
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
				if (cBuf == ' ')			// '# ' = �ּ�
				{		
					do
					{
						cBuf = fgetc(fp);
					} while (cBuf != 0x0A);	// ���� �ٲ�� �κб��� �б�
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
									if (cBuf == ' ')	// 'newmtl ' = ���� �̸�
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
									if (cBuf == ' ')	// 'map_Kd ' = �ؽ�ó ���� �̸���
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
				if (cBuf == ' ')	// 'd ' = ������
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
					case 'a':	// 'Ka ' = ȯ�汤(Ambient color)��
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

					case 'd':	// 'Kd ' = Ȯ�걤(Diffuse color)��
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

					case 's':	// 'Ks ' = �ݻ籤(Specular color)��
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
	}	// WHILE�� ����

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

	// ���� ���� ������Ʈ!
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

	// ���� ���� ������Ʈ!
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

	return S_OK;	// �Լ� ����!
}

HRESULT ModelOBJ::SetTexture(LPDIRECT3DDEVICE9 D3DDevice, int GroupID)
{
	if ( ModelTextures[GroupID] != NULL)
		ModelTextures[GroupID]->Release();

	// �ؽ�ó �ҷ�����
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

		int numVertices = 8;	// ���ڴϱ� �� 8�� �ʿ�
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
		

		int numIndices = 12;	// ���ڴϱ� �𼭸� 12�� �ʿ�
		INDEX_OBJ_BB *NewIndices = new INDEX_OBJ_BB[numIndices];

			NewIndices[0]._0 = 0, NewIndices[0]._1 = 1;	// �ٴ�
			NewIndices[1]._0 = 1, NewIndices[1]._1 = 2;
			NewIndices[2]._0 = 2, NewIndices[2]._1 = 3;
			NewIndices[3]._0 = 3, NewIndices[3]._1 = 0;
			NewIndices[4]._0 = 0, NewIndices[4]._1 = 4;	// ���
			NewIndices[5]._0 = 1, NewIndices[5]._1 = 5;
			NewIndices[6]._0 = 2, NewIndices[6]._1 = 6;
			NewIndices[7]._0 = 3, NewIndices[7]._1 = 7;
			NewIndices[8]._0 = 4, NewIndices[8]._1 = 5;	// õ��
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

HRESULT ModelOBJ::DrawNormalVecters(LPDIRECT3DDEVICE9 D3DDevice, float LenFactor)
{
	for (int i = 0; i < numGroups; i++)
	{

		if( g_pModelVB != NULL )
			g_pModelVB->Release();

		if( g_pModelIB != NULL )
			g_pModelIB->Release();

		int numVertices = ModelGroups[i].numVertices * 2;
		int numIndices = numVertices / 2;
		int	numStartVID =  ModelGroups[i].numStartVertexID;
		int	numStartIID =  ModelGroups[i].numStartIndexID;

		// ���� ���� ������Ʈ!
		VERTEX_OBJ_NORMAL *NewVertices = new VERTEX_OBJ_NORMAL[numVertices];

			for (int i = 0; i < numVertices; i+=2)
			{
				NewVertices[i].Normal.x = Vertices[numStartVID + i/2].Position.x;
				NewVertices[i].Normal.y = Vertices[numStartVID + i/2].Position.y;
				NewVertices[i].Normal.z = Vertices[numStartVID + i/2].Position.z;
				NewVertices[i+1].Normal.x = NewVertices[i].Normal.x + Vertices[numStartVID + i/2].Normal.x * LenFactor;
				NewVertices[i+1].Normal.y = NewVertices[i].Normal.y + Vertices[numStartVID + i/2].Normal.y * LenFactor;
				NewVertices[i+1].Normal.z = NewVertices[i].Normal.z + Vertices[numStartVID + i/2].Normal.z * LenFactor;
			}

			int SizeOfVertices = sizeof(VERTEX_OBJ_NORMAL)*numVertices;
			if (FAILED(D3DDevice->CreateVertexBuffer(SizeOfVertices, 0, D3DFVF_VERTEX_OBJ_NORMAL, D3DPOOL_DEFAULT, &g_pModelVB, NULL)))
				return E_FAIL;
	
			VOID* pVertices;
			if (FAILED(g_pModelVB->Lock(0, SizeOfVertices, (void**)&pVertices, 0)))
				return E_FAIL;
			memcpy(pVertices, NewVertices, SizeOfVertices);
			g_pModelVB->Unlock();

		delete[] NewVertices;


		// ���� ���� ������Ʈ!
		INDEX_OBJ_NORMAL *NewIndices = new INDEX_OBJ_NORMAL[numIndices];
		int k = 0;

			for (int i = 0; i < numIndices; i++)
			{
				NewIndices[i]._0 = k++;
				NewIndices[i]._1 = k++;
			}

			int SizeOfIndices = sizeof(INDEX_OBJ_NORMAL)*numIndices;
			if (FAILED(D3DDevice->CreateIndexBuffer(SizeOfIndices, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pModelIB, NULL)))
				return E_FAIL;

			VOID* pIndices;
			if (FAILED(g_pModelIB->Lock(0, SizeOfIndices, (void **)&pIndices, 0)))
				return E_FAIL;
			memcpy(pIndices, NewIndices, SizeOfIndices);
			g_pModelIB->Unlock();
	
		delete[] NewIndices;

		D3DDevice->SetStreamSource(0, g_pModelVB, 0, sizeof(VERTEX_OBJ_NORMAL));
		D3DDevice->SetFVF(D3DFVF_VERTEX_OBJ_NORMAL);
		D3DDevice->SetIndices(g_pModelIB);

		D3DDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, numVertices, 0, numIndices);

	}

	return S_OK;	// �Լ� ����!
}

void ModelOBJ::DrawModel(LPDIRECT3DDEVICE9 D3DDevice)
{
	// ���� �� �޽��� �׸���!
	for (int i = 0; i < numGroups; i++)
	{
		UpdateVertices(D3DDevice, i);

		// ������ �����Ѵ�.
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

		D3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, true );

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

void ModelOBJ::DrawMesh_Opaque(LPDIRECT3DDEVICE9 D3DDevice)
{
	// ���� �� �޽��� �׸���!
	for (int i = 0; i < numGroups; i++)
	{
		UpdateVertices(D3DDevice, i);

		if (ModelMaterials[ModelGroups[i].MaterialID].Transparency != 1)	// ���İ��� 1�� �ƴϸ� ����
			continue;

		// ������ �����Ѵ�.
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

		D3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, false );

		D3DDevice->SetTexture(0, ModelTextures[i]);

		D3DDevice->SetStreamSource(0, g_pModelVB, 0, sizeof(VERTEX_OBJ));
		D3DDevice->SetFVF(D3DFVF_VERTEX_OBJ);
		D3DDevice->SetIndices(g_pModelIB);

		D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, ModelGroups[i].numVertices, 0, ModelGroups[i].numIndices);
	}

	return;
}

void ModelOBJ::DrawMesh_Transparent(LPDIRECT3DDEVICE9 D3DDevice)
{
	// ���� �� �޽��� �׸���!
	for (int i = 0; i < numGroups; i++)
	{
		UpdateVertices(D3DDevice, i);

		if (ModelMaterials[ModelGroups[i].MaterialID].Transparency == 1)	// ���İ��� 1�̸� ����
			continue;

		// ������ �����Ѵ�.
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

		D3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, true );

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

PickingRay GetPickingRay(LPDIRECT3DDEVICE9 D3DDevice, int MouseX, int MouseY,
	int ScreenWidth, int ScreenHeight, D3DXMATRIX matView, D3DXMATRIX matProj)
{
	if (MouseX < 0 || MouseY < 0 || MouseX > ScreenWidth || MouseY > ScreenHeight)
		return PickingRay(D3DXVECTOR3(0,0,0), D3DXVECTOR3(9999.0f,0,0));

	D3DVIEWPORT9 vp;
	D3DXMATRIX InvView;

	D3DXVECTOR3 MouseViewPortXY, PickingRayDir, PickingRayPos;

	D3DDevice->GetViewport(&vp);
	D3DXMatrixInverse(&InvView, NULL, &matView);

	MouseViewPortXY.x = (( (((MouseX-vp.X)*2.0f/vp.Width ) - 1.0f)) - matProj._31 ) / matProj._11;
	MouseViewPortXY.y = ((- (((MouseY-vp.Y)*2.0f/vp.Height) - 1.0f)) - matProj._32 ) / matProj._22;
	MouseViewPortXY.z = 1.0f;

	PickingRayDir.x = MouseViewPortXY.x*InvView._11 + MouseViewPortXY.y*InvView._21 + MouseViewPortXY.z*InvView._31;
	PickingRayDir.y = MouseViewPortXY.x*InvView._12 + MouseViewPortXY.y*InvView._22 + MouseViewPortXY.z*InvView._32;
	PickingRayDir.z = MouseViewPortXY.x*InvView._13 + MouseViewPortXY.y*InvView._23 + MouseViewPortXY.z*InvView._33;
	D3DXVec3Normalize(&PickingRayDir, &PickingRayDir);
	PickingRayPos.x = InvView._41;
	PickingRayPos.y = InvView._42;
	PickingRayPos.z = InvView._43;

	return PickingRay(PickingRayPos, PickingRayDir);
}

bool ModelOBJ::CheckMouseOver(LPDIRECT3DDEVICE9 D3DDevice, int MouseX, int MouseY,
	int ScreenWidth, int ScreenHeight, D3DXMATRIX matView, D3DXMATRIX matProj)
{
	PickingRay PR = GetPickingRay(D3DDevice, MouseX, MouseY, ScreenWidth, ScreenHeight, matView, matProj);

	if (PR.Dir.x == 9999.0f)
		return false;

	for (int m = 0; m < numInstances; m++)
	{
		MouseOverPerInstances[m] = false;

		for (int j = 0; j < numGroups; j++)
		{
			int numIndices = ModelGroups[j].numIndices;
			int IndexStart = ModelGroups[j].numStartIndexID;

			for (int i = 0; i < numIndices; i++)
			{
				int numPrevVertices = 0;

				if ( j > 0 )
				{
					for (int k = 0; k < j; k++)
					{
						numPrevVertices += ModelGroups[k].numVertices;
					}
				}

				int ID0 = Indices[IndexStart + i]._0 + numPrevVertices;
				int ID1 = Indices[IndexStart + i]._1 + numPrevVertices;
				int ID2 = Indices[IndexStart + i]._2 + numPrevVertices;

				D3DXVECTOR3 p0, p1, p2;
				p0 = D3DXVECTOR3(Vertices[ID0].Position.x, Vertices[ID0].Position.y, Vertices[ID0].Position.z);
				p1 = D3DXVECTOR3(Vertices[ID1].Position.x, Vertices[ID1].Position.y, Vertices[ID1].Position.z);
				p2 = D3DXVECTOR3(Vertices[ID2].Position.x, Vertices[ID2].Position.y, Vertices[ID2].Position.z);

				// �ν��Ͻ�
				D3DXMATRIX matInstTrans;
				D3DXMATRIX matInstRotX;
				D3DXMATRIX matInstRotY;
				D3DXMATRIX matInstRotZ;
				D3DXMATRIX matInstScal;
				D3DXMATRIX matInstWorld;

				D3DXMatrixTranslation(&matInstTrans,
					ModelInstances[m].Translation.x, ModelInstances[m].Translation.y, ModelInstances[m].Translation.z);

				D3DXMatrixRotationX(&matInstRotX, ModelInstances[m].Rotation.x);
				D3DXMatrixRotationY(&matInstRotY, ModelInstances[m].Rotation.y);
				D3DXMatrixRotationZ(&matInstRotZ, ModelInstances[m].Rotation.z);

				D3DXMatrixScaling(&matInstScal,
					ModelInstances[m].Scaling.x, ModelInstances[m].Scaling.y, ModelInstances[m].Scaling.z);
				
				matInstWorld = matInstTrans * matInstRotX * matInstRotY * matInstRotZ * matInstScal;

				D3DXVec3TransformCoord(&p0, &p0, &matInstWorld);
				D3DXVec3TransformCoord(&p1, &p1, &matInstWorld);
				D3DXVec3TransformCoord(&p2, &p2, &matInstWorld);

				float pU, pV, pDist;

				if (D3DXIntersectTri(&p0, &p1, &p2, &PR.Pos, &PR.Dir, &pU, &pV, &pDist))
				{
					MouseOverPerInstances[m] = true;
					return true;
				}
			}
		}
	}

	return false;
}