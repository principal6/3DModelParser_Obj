#include "ModelOBJ.h"

ModelOBJ::ModelOBJ()
{
	// 변수 초기화
	memset(&ModelObject, 0, sizeof(ModelObject));
	memset(ModelGroups, 0, sizeof(ModelGroups));
	memset(ModelMaterials, 0, sizeof(ModelMaterials));

	memset(ModelInstances, 0, sizeof(ModelInstances));
	memset(MouseOverPerInstances, 0, sizeof(MouseOverPerInstances));
	memset(DistanceCmp, 0, sizeof(DistanceCmp));
	memset(PickedPosition, 0, sizeof(PickedPosition));
	
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

	SAFE_RELEASE(pModelVB);
	SAFE_RELEASE(pModelIB);

	pDevice	= NULL;
	pHLSL	= NULL;
}

bool ModelOBJ::CreateModel(LPDIRECT3DDEVICE9 D3DDevice, char* BaseDir, char* FileNameWithoutExtension, LPD3DXEFFECT HLSL)
{
	// 포인터 지정
	pDevice = D3DDevice;
	pHLSL	= HLSL;

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
		SetTexture(i);
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

	SetInstance(numInstances-1, Translation, Rotation, Scaling);

	return;
}

void ModelOBJ::SetInstance(int InstanceID, XMFLOAT3 Translation, XMFLOAT3 Rotation, XMFLOAT3 Scaling)
{
	ModelInstances[InstanceID].Translation = Translation;
	ModelInstances[InstanceID].Rotation = Rotation;
	ModelInstances[InstanceID].Scaling = Scaling;

	// 월드 행렬(위치, 회전, 크기 설정)
	D3DXMatrixIdentity( &matModelWorld[InstanceID] );

		D3DXMATRIXA16 matTrans;
		D3DXMATRIXA16 matRotX;
		D3DXMATRIXA16 matRotY;
		D3DXMATRIXA16 matRotZ;
		D3DXMATRIXA16 matSize;

		D3DXMatrixTranslation(&matTrans, Translation.x, Translation.y, Translation.z);
		D3DXMatrixRotationX(&matRotX, Rotation.x);
		D3DXMatrixRotationY(&matRotY, Rotation.y);				// Y축을 기준으로 회전 (즉, X&Z가 회전함)
		D3DXMatrixRotationZ(&matRotZ, Rotation.z);
		D3DXMatrixScaling(&matSize, Scaling.x, Scaling.y, Scaling.z);

	matModelWorld[InstanceID] = matModelWorld[InstanceID] * matRotX * matRotY * matRotZ * matSize * matTrans;

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

HRESULT ModelOBJ::UpdateVertices(int GroupID)
{
	SAFE_RELEASE(pModelVB);
	SAFE_RELEASE(pModelIB);

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
		if (FAILED(pDevice->CreateVertexBuffer(SizeOfVertices, 0, D3DFVF_VERTEX_OBJ, D3DPOOL_DEFAULT, &pModelVB, NULL)))
			return E_FAIL;
	
		VOID* pVertices;
		if (FAILED(pModelVB->Lock(0, SizeOfVertices, (void**)&pVertices, 0)))
			return E_FAIL;
		memcpy(pVertices, NewVertices, SizeOfVertices);
		pModelVB->Unlock();

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
		if (FAILED(pDevice->CreateIndexBuffer(SizeOfIndices, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pModelIB, NULL)))
			return E_FAIL;

		VOID* pIndices;
		if (FAILED(pModelIB->Lock(0, SizeOfIndices, (void **)&pIndices, 0)))
			return E_FAIL;
		memcpy(pIndices, NewIndices, SizeOfIndices);
		pModelIB->Unlock();
	
	delete[] NewIndices;

	return S_OK;	// 함수 종료!
}

HRESULT ModelOBJ::SetTexture(int GroupID)
{
	SAFE_RELEASE(ModelTextures[GroupID]);

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

	D3DXCreateTextureFromFile(pDevice, NewFileName, &ModelTextures[GroupID]);

	return S_OK;
}

void ModelOBJ::DrawBoundingBoxes()
{
	for (int i = 0; i < numGroups; i++)
	{
		SAFE_RELEASE(pModelVB);
		SAFE_RELEASE(pModelIB);

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

			for (int i = 0; i < 8; i++)
			{
				NewVertices[i].Color = D3DCOLOR_RGBA(255, 255, 0, 255);
			}

			int SizeOfVertices = sizeof(VERTEX_OBJ_BB)*numVertices;
			if (FAILED(pDevice->CreateVertexBuffer(SizeOfVertices, 0, D3DFVF_VERTEX_OBJ_BB, D3DPOOL_DEFAULT, &pModelVB, NULL)))
				return;
	
			VOID* pVertices;
			if (FAILED(pModelVB->Lock(0, SizeOfVertices, (void**)&pVertices, 0)))
				return;
			memcpy(pVertices, NewVertices, SizeOfVertices);
			pModelVB->Unlock();

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
			if (FAILED(pDevice->CreateIndexBuffer(SizeOfIndices, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pModelIB, NULL)))
				return;

			VOID* pIndices;
			if (FAILED(pModelIB->Lock(0, SizeOfIndices, (void **)&pIndices, 0)))
				return;
			memcpy(pIndices, NewIndices, SizeOfIndices);
			pModelIB->Unlock();
	
		delete[] NewIndices;

		pDevice->SetTexture(0, 0);
		pDevice->SetStreamSource(0, pModelVB, 0, sizeof(VERTEX_OBJ_BB));
		pDevice->SetFVF(D3DFVF_VERTEX_OBJ_BB);
		pDevice->SetIndices(pModelIB);

		pDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, numVertices, 0, numIndices);
	}

	return;
}

HRESULT ModelOBJ::DrawNormalVecters(float LenFactor)
{
	for (int i = 0; i < numGroups; i++)
	{
		SAFE_RELEASE(pModelVB);
		SAFE_RELEASE(pModelIB);

		int numVertices = ModelGroups[i].numVertices * 2;
		int numIndices = numVertices / 2;
		int	numStartVID =  ModelGroups[i].numStartVertexID;
		int	numStartIID =  ModelGroups[i].numStartIndexID;

		// 정점 버퍼 업데이트!
		VERTEX_OBJ_NORMAL *NewVertices = new VERTEX_OBJ_NORMAL[numVertices];

			for (int j = 0; j < numVertices; j+=2)
			{
				NewVertices[j].Normal.x = Vertices[numStartVID + j/2].Position.x;
				NewVertices[j].Normal.y = Vertices[numStartVID + j/2].Position.y;
				NewVertices[j].Normal.z = Vertices[numStartVID + j/2].Position.z;
				NewVertices[j+1].Normal.x = NewVertices[j].Normal.x + Vertices[numStartVID + j/2].Normal.x * LenFactor;
				NewVertices[j+1].Normal.y = NewVertices[j].Normal.y + Vertices[numStartVID + j/2].Normal.y * LenFactor;
				NewVertices[j+1].Normal.z = NewVertices[j].Normal.z + Vertices[numStartVID + j/2].Normal.z * LenFactor;

				NewVertices[j].Color = D3DCOLOR_RGBA(255, 0, 255, 255);
				NewVertices[j+1].Color = D3DCOLOR_RGBA(255, 0, 255, 255);
			}

			int SizeOfVertices = sizeof(VERTEX_OBJ_NORMAL)*numVertices;
			if (FAILED(pDevice->CreateVertexBuffer(SizeOfVertices, 0, D3DFVF_VERTEX_OBJ_NORMAL, D3DPOOL_DEFAULT, &pModelVB, NULL)))
				return E_FAIL;
	
			VOID* pVertices;
			if (FAILED(pModelVB->Lock(0, SizeOfVertices, (void**)&pVertices, 0)))
				return E_FAIL;
			memcpy(pVertices, NewVertices, SizeOfVertices);
			pModelVB->Unlock();

		delete[] NewVertices;


		// 색인 버퍼 업데이트!
		INDEX_OBJ_NORMAL *NewIndices = new INDEX_OBJ_NORMAL[numIndices];
		int k = 0;

			for (int j = 0; j < numIndices; j++)
			{
				NewIndices[j]._0 = k++;
				NewIndices[j]._1 = k++;
			}

			int SizeOfIndices = sizeof(INDEX_OBJ_NORMAL)*numIndices;
			if (FAILED(pDevice->CreateIndexBuffer(SizeOfIndices, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pModelIB, NULL)))
				return E_FAIL;

			VOID* pIndices;
			if (FAILED(pModelIB->Lock(0, SizeOfIndices, (void **)&pIndices, 0)))
				return E_FAIL;
			memcpy(pIndices, NewIndices, SizeOfIndices);
			pModelIB->Unlock();
	
		delete[] NewIndices;

		pDevice->SetTexture(0, 0);
		pDevice->SetStreamSource(0, pModelVB, 0, sizeof(VERTEX_OBJ_NORMAL));
		pDevice->SetFVF(D3DFVF_VERTEX_OBJ_NORMAL);
		pDevice->SetIndices(pModelIB);

		pDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, numVertices, 0, numIndices);

	}

	return S_OK;	// 함수 종료!
}

void ModelOBJ::DrawModel(int InstanceID)
{
	pDevice->SetTransform(D3DTS_WORLD, &matModelWorld[InstanceID]);

	// 모델의 각 메쉬를 그린다!
	for (int i = 0; i < numGroups; i++)
	{
		UpdateVertices(i);

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
		pDevice->SetMaterial( &mtrl );

		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

		pDevice->SetTexture(0, ModelTextures[i]);

		pDevice->SetStreamSource(0, pModelVB, 0, sizeof(VERTEX_OBJ));
		pDevice->SetFVF(D3DFVF_VERTEX_OBJ);
		pDevice->SetIndices(pModelIB);

		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, ModelGroups[i].numVertices, 0, ModelGroups[i].numIndices);
	}

	return;
}

void ModelOBJ::DrawModel_HLSL(int InstanceID, D3DXMATRIX matView, D3DXMATRIX matProj)
{
	// 모델의 각 메쉬를 그린다!
	for (int i = 0; i < numGroups; i++)
	{
		UpdateVertices(i);

		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

		pDevice->SetStreamSource(0, pModelVB, 0, sizeof(VERTEX_OBJ));
		pDevice->SetFVF(D3DFVF_VERTEX_OBJ);
		pDevice->SetIndices(pModelIB);

		pHLSL->SetTechnique("HLSLNoInstancing");

		D3DXMATRIX matWVP = matModelWorld[InstanceID] * matView * matProj;
		pHLSL->SetMatrix("matWVP", &matWVP);

		UINT numPasses = 0;
		pHLSL->Begin(&numPasses, NULL);

		for (UINT j = 0; j < numPasses; ++j)
		{
			pHLSL->BeginPass(j);

			if (ModelTextures[i])
			{
				pHLSL->SetBool("UseTexture", true);
				pHLSL->SetTexture("DiffuseMap_Tex", ModelTextures[i]);
			}
			else
			{		
				pHLSL->SetBool("UseTexture", false);
				float MtrlDiffuse[4];
					MtrlDiffuse[0] = ModelMaterials[ModelGroups[i].MaterialID].Color_Diffuse.x;
					MtrlDiffuse[1] = ModelMaterials[ModelGroups[i].MaterialID].Color_Diffuse.y;
					MtrlDiffuse[2] = ModelMaterials[ModelGroups[i].MaterialID].Color_Diffuse.z;
					MtrlDiffuse[3] = ModelMaterials[ModelGroups[i].MaterialID].Transparency;
				pHLSL->SetFloatArray("MtrlDiffuse", MtrlDiffuse, 4);
			}

			pHLSL->CommitChanges();

			pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, ModelGroups[i].numVertices, 0, ModelGroups[i].numIndices);

			pHLSL->EndPass();
		}

		pHLSL->End();
	}

	return;
}

void ModelOBJ::DrawMesh_Opaque(int InstanceID)
{
	pDevice->SetTransform(D3DTS_WORLD, &matModelWorld[InstanceID]);

	// 모델의 각 메쉬를 그린다!
	for (int i = 0; i < numGroups; i++)
	{
		UpdateVertices(i);

		if (ModelMaterials[ModelGroups[i].MaterialID].Transparency != 1)	// 알파값이 1이 아니면 종료
			continue;

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
		pDevice->SetMaterial( &mtrl );

		pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, false );

		pDevice->SetTexture(0, ModelTextures[i]);

		pDevice->SetStreamSource(0, pModelVB, 0, sizeof(VERTEX_OBJ));
		pDevice->SetFVF(D3DFVF_VERTEX_OBJ);
		pDevice->SetIndices(pModelIB);

		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, ModelGroups[i].numVertices, 0, ModelGroups[i].numIndices);
	}

	return;
}

void ModelOBJ::DrawMesh_Transparent(int InstanceID)
{
	pDevice->SetTransform(D3DTS_WORLD, &matModelWorld[InstanceID]);

	// 모델의 각 메쉬를 그린다!
	for (int i = 0; i < numGroups; i++)
	{
		UpdateVertices(i);

		if (ModelMaterials[ModelGroups[i].MaterialID].Transparency == 1)	// 알파값이 1이면 종료
			continue;

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
		pDevice->SetMaterial( &mtrl );

		pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
		
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

		pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
		pDevice->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);

		pDevice->SetTexture(0, ModelTextures[i]);
		pDevice->SetStreamSource(0, pModelVB, 0, sizeof(VERTEX_OBJ));
		pDevice->SetFVF(D3DFVF_VERTEX_OBJ);
		pDevice->SetIndices(pModelIB);

		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, ModelGroups[i].numVertices, 0, ModelGroups[i].numIndices);
	}

	return;
}


void ModelOBJ::SetHLSLTexture(int GroupID)
{
	if (ModelTextures[GroupID])
	{
		pHLSL->SetBool("UseTexture", true);
		pHLSL->SetTexture("DiffuseMap_Tex", ModelTextures[GroupID]);
	}
	else
	{
		pHLSL->SetBool("UseTexture", false);
		float MtrlDiffuse[4];
		MtrlDiffuse[0] = ModelMaterials[ModelGroups[GroupID].MaterialID].Color_Diffuse.x;
		MtrlDiffuse[1] = ModelMaterials[ModelGroups[GroupID].MaterialID].Color_Diffuse.y;
		MtrlDiffuse[2] = ModelMaterials[ModelGroups[GroupID].MaterialID].Color_Diffuse.z;
		MtrlDiffuse[3] = ModelMaterials[ModelGroups[GroupID].MaterialID].Transparency;
		pHLSL->SetFloatArray("MtrlDiffuse", MtrlDiffuse, 4);
	}

	return;
}

void ModelOBJ::DrawMesh_HLSL_Opaque(int InstanceID, D3DXMATRIX matView, D3DXMATRIX matProj)
{
	pDevice->SetTransform(D3DTS_WORLD, &matModelWorld[InstanceID]);

	// 모델의 각 메쉬를 그린다!
	for (int i = 0; i < numGroups; i++)
	{
		UpdateVertices(i);

		if (ModelMaterials[ModelGroups[i].MaterialID].Transparency != 1)	// 알파값이 1이 아니면 종료
			continue;

		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

		pDevice->SetStreamSource(0, pModelVB, 0, sizeof(VERTEX_OBJ));
		pDevice->SetFVF(D3DFVF_VERTEX_OBJ);
		pDevice->SetIndices(pModelIB);

		pHLSL->SetTechnique("HLSLNoInstancing");

		D3DXMATRIX matWVP = matModelWorld[InstanceID] * matView * matProj;
		pHLSL->SetMatrix("matWVP", &matWVP);

		UINT numPasses = 0;
		pHLSL->Begin(&numPasses, NULL);

		for (UINT j = 0; j < numPasses; ++j)
		{
			pHLSL->BeginPass(j);

			SetHLSLTexture(i);

			pHLSL->CommitChanges();

			pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, ModelGroups[i].numVertices, 0, ModelGroups[i].numIndices);

			pHLSL->EndPass();
		}

		pHLSL->End();

	}

	return;
}

void ModelOBJ::DrawMesh_HLSL_Transparent(int InstanceID, D3DXMATRIX matView, D3DXMATRIX matProj)
{
	pDevice->SetTransform(D3DTS_WORLD, &matModelWorld[InstanceID]);

	// 모델의 각 메쉬를 그린다!
	for (int i = 0; i < numGroups; i++)
	{
		UpdateVertices(i);

		if (ModelMaterials[ModelGroups[i].MaterialID].Transparency == 1)	// 알파값이 1이면 종료
			continue;

		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

		pDevice->SetStreamSource(0, pModelVB, 0, sizeof(VERTEX_OBJ));
		pDevice->SetFVF(D3DFVF_VERTEX_OBJ);
		pDevice->SetIndices(pModelIB);

		pHLSL->SetTechnique("HLSLNoInstancing");

		D3DXMATRIX matWVP = matModelWorld[InstanceID] * matView * matProj;
		pHLSL->SetMatrix("matWVP", &matWVP);

		UINT numPasses = 0;
		pHLSL->Begin(&numPasses, NULL);

		for (UINT j = 0; j < numPasses; ++j)
		{
			pHLSL->BeginPass(j);

			SetHLSLTexture(i);

			pHLSL->CommitChanges();

			pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, ModelGroups[i].numVertices, 0, ModelGroups[i].numIndices);

			pHLSL->EndPass();
		}

		pHLSL->End();

	}

	return;
}

PickingRay ModelOBJ::GetPickingRay(int MouseX, int MouseY, int ScreenWidth, int ScreenHeight, D3DXMATRIX matView, D3DXMATRIX matProj)
{
	if (MouseX < 0 || MouseY < 0 || MouseX > ScreenWidth || MouseY > ScreenHeight)
		return PickingRay(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(FLT_MAX, 0, 0));

	D3DVIEWPORT9 vp;
	D3DXMATRIX InvView;

	D3DXVECTOR3 MouseViewPortXY, PickingRayDir, PickingRayPos;

	pDevice->GetViewport(&vp);
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

bool ModelOBJ::CheckMouseOverPerInstance(int InstanceID, int MouseX, int MouseY, int ScreenWidth, int ScreenHeight,
	D3DXMATRIX matView, D3DXMATRIX matProj)
{
	PickingRay PR = GetPickingRay(MouseX, MouseY, ScreenWidth, ScreenHeight, matView, matProj);

	if (PR.Dir.x == FLT_MAX)
		return false;

	DistanceCmp[InstanceID]				= FLT_MAX;
	MouseOverPerInstances[InstanceID]	= false;
	PickedPosition[InstanceID]			= XMFLOAT3(0, 0, 0);

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

			// 인스턴스
			D3DXMATRIX matInstTrans;
			D3DXMATRIX matInstRotX;
			D3DXMATRIX matInstRotY;
			D3DXMATRIX matInstRotZ;
			D3DXMATRIX matInstScal;
			D3DXMATRIX matInstWorld;

			D3DXMatrixTranslation(&matInstTrans,
				ModelInstances[InstanceID].Translation.x, ModelInstances[InstanceID].Translation.y, ModelInstances[InstanceID].Translation.z);

			D3DXMatrixRotationX(&matInstRotX, ModelInstances[InstanceID].Rotation.x);
			D3DXMatrixRotationY(&matInstRotY, ModelInstances[InstanceID].Rotation.y);
			D3DXMatrixRotationZ(&matInstRotZ, ModelInstances[InstanceID].Rotation.z);

			D3DXMatrixScaling(&matInstScal,
				ModelInstances[InstanceID].Scaling.x, ModelInstances[InstanceID].Scaling.y, ModelInstances[InstanceID].Scaling.z);
				
			matInstWorld = matInstRotX * matInstRotY * matInstRotZ * matInstScal * matInstTrans;

			D3DXVec3TransformCoord(&p0, &p0, &matInstWorld);
			D3DXVec3TransformCoord(&p1, &p1, &matInstWorld);
			D3DXVec3TransformCoord(&p2, &p2, &matInstWorld);

			float pU, pV, pDist;

			if (D3DXIntersectTri(&p0, &p1, &p2, &PR.Pos, &PR.Dir, &pU, &pV, &pDist))
			{
				if (pDist < DistanceCmp[InstanceID])
				{
					DistanceCmp[InstanceID] = pDist;
					D3DXVECTOR3 TempPosition = p0 + (p1*pU - p0*pU) + (p2*pV - p0*pV);
					D3DXVec3TransformCoord(&TempPosition, &TempPosition, &matInstWorld);

					PickedPosition[InstanceID].x = TempPosition.x;
					PickedPosition[InstanceID].y = TempPosition.y;
					PickedPosition[InstanceID].z = TempPosition.z;
				}

				MouseOverPerInstances[InstanceID] = true;
			}
		}
	}

	switch (MouseOverPerInstances[InstanceID])
	{
		case true:
			return true;
		case false:
			return false;
	}

	return false;
}

void ModelOBJ::CheckMouseOverFinal()
{
	float	ClosestDistance		= FLT_MAX;
	int		ClosestInstanceID	= -1;

	for (int i = 0; i < numInstances; i++)
	{
		if (MouseOverPerInstances[i] == true)
		{
			if ( DistanceCmp[i] < ClosestDistance )
			{
				ClosestDistance = DistanceCmp[i];
				ClosestInstanceID = i;
			}
		}
	}

	for (int i = 0; i < numInstances; i++)
	{
		MouseOverPerInstances[i] = false;
	}

	if (ClosestInstanceID != -1)
		MouseOverPerInstances[ClosestInstanceID] = true;
}