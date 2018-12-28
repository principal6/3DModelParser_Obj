#define AnimationJointTexW	800
#define AnimationWeightTexW	800
#define AnimationWeightTexH	100
#define MAX_ANIMATIONS_MD5		20			// 모델 - 애니메이션 최대 개수
#define MAX_FRAMES_MD5			100			// 모델 - 애니메이션 최대 프레임

// --- 전역 변수 --- //
float4x4	matVP;
float4x4	matWVP;

bool		UseTexture;
float4		MtrlDiffuse;

texture		DiffuseMap_Tex;
texture		AnimationJoint_Tex;
texture		AnimationWeight_Tex;

sampler2D	DiffuseSampler =
sampler_state
{
	Texture = (DiffuseMap_Tex);
};

sampler2D	AnimationJointSampler =
sampler_state
{
	Texture = (AnimationJoint_Tex);
};

sampler2D	AnimationWeightSampler =
sampler_state
{
	Texture = (AnimationWeight_Tex);
};

struct VS_INPUT_INSTANCING
{
	// 정점 정보
	float4	Position	: POSITION0;
	float3	Normal		: NORMAL0;
	float2	Texture		: TEXCOORD0;
	float	WeightStart : PSIZE0;
	float	WeightNum : PSIZE1;

	// 인스턴스 정보
	float4	matWorld0		: POSITION1;
	float4	matWorld1		: POSITION2;
	float4	matWorld2		: POSITION3;
	float4	matWorld3		: POSITION4;
	float	bAnimated : PSIZE2;
	float	AnimationID : PSIZE3;
	float	CurFrameTime : PSIZE4;
	float	Frame0 : PSIZE5;
	float	Frame1 : PSIZE6;
};

struct VS_INPUT_NoInstancing
{
	// 정점 정보
	float4	Position	: POSITION0;
	float3	Normal		: NORMAL0;
	float2	Texture		: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Position : POSITION0;
	float2 Texture	: TEXCOORD0;
	float3 Normal	: TEXCOORD1;
};

float GetMod(float Dividend, float Divisor)	// fmod()가 오차값을 발생시키기 때문에 만든 함수★★
{
	float times = floor(Dividend / Divisor);
	float multiplied = Divisor * times;
	return (Dividend - multiplied);
}

float4 GetJointPosition(in int AnimID, in int FrameID, in int JointID)
{
	float4 Output;
	Output.x = tex2Dlod(AnimationJointSampler, float4(((0.5 + JointID * 8) / AnimationJointTexW), ((0.5 + FrameID + AnimID*MAX_FRAMES_MD5) / (MAX_FRAMES_MD5 * MAX_ANIMATIONS_MD5)), 0, 0));
	Output.y = tex2Dlod(AnimationJointSampler, float4(((0.5 + JointID * 8 + 1) / AnimationJointTexW), ((0.5 + FrameID + AnimID*MAX_FRAMES_MD5) / (MAX_FRAMES_MD5 * MAX_ANIMATIONS_MD5)), 0, 0));
	Output.z = tex2Dlod(AnimationJointSampler, float4(((0.5 + JointID * 8 + 2) / AnimationJointTexW), ((0.5 + FrameID + AnimID*MAX_FRAMES_MD5) / (MAX_FRAMES_MD5 * MAX_ANIMATIONS_MD5)), 0, 0));
	Output.w = tex2Dlod(AnimationJointSampler, float4(((0.5 + JointID * 8 + 3) / AnimationJointTexW), ((0.5 + FrameID + AnimID*MAX_FRAMES_MD5) / (MAX_FRAMES_MD5 * MAX_ANIMATIONS_MD5)), 0, 0));
	return Output;
}

float4 GetJointOrientation(in int AnimID, in int FrameID, in int JointID)
{
	float4 Output;
	Output.x = tex2Dlod(AnimationJointSampler, float4(((0.5 + JointID * 8 + 4) / AnimationJointTexW), ((0.5 + FrameID + AnimID*MAX_FRAMES_MD5) / (MAX_FRAMES_MD5 * MAX_ANIMATIONS_MD5)), 0, 0));
	Output.y = tex2Dlod(AnimationJointSampler, float4(((0.5 + JointID * 8 + 5) / AnimationJointTexW), ((0.5 + FrameID + AnimID*MAX_FRAMES_MD5) / (MAX_FRAMES_MD5 * MAX_ANIMATIONS_MD5)), 0, 0));
	Output.z = tex2Dlod(AnimationJointSampler, float4(((0.5 + JointID * 8 + 6) / AnimationJointTexW), ((0.5 + FrameID + AnimID*MAX_FRAMES_MD5) / (MAX_FRAMES_MD5 * MAX_ANIMATIONS_MD5)), 0, 0));
	Output.w = tex2Dlod(AnimationJointSampler, float4(((0.5 + JointID * 8 + 7) / AnimationJointTexW), ((0.5 + FrameID + AnimID*MAX_FRAMES_MD5) / (MAX_FRAMES_MD5 * MAX_ANIMATIONS_MD5)), 0, 0));
	return Output;
}

int GetWeightJointID(in int WeightID)
{
	float Dividend = WeightID * 8;
	int WeightU = GetMod(Dividend, AnimationWeightTexW);
	int WeightV = floor(Dividend / AnimationWeightTexW);
	int Output = tex2Dlod(AnimationWeightSampler, float4(((0.5 + WeightU) / AnimationWeightTexW), ((0.5 + WeightV) / AnimationWeightTexH), 0, 0));
	return Output;
}

float GetWeightBias(in int WeightID)
{
	float Dividend = WeightID * 8;
	int WeightU = GetMod(Dividend, AnimationWeightTexW);
	int WeightV = floor(Dividend / AnimationWeightTexW);
	float Output = tex2Dlod(AnimationWeightSampler, float4(((0.5 + WeightU + 1) / AnimationWeightTexW), ((0.5 + WeightV) / AnimationWeightTexH), 0, 0));
	return Output;
}

float4 GetWeightPosition(in int WeightID)
{
	float Dividend = WeightID * 8;
	int WeightU = GetMod(Dividend, AnimationWeightTexW);
	int WeightV = floor(Dividend / AnimationWeightTexW);
	float4 Output;
	Output.x = tex2Dlod(AnimationWeightSampler, float4(((0.5 + WeightU + 2) / AnimationWeightTexW), ((0.5 + WeightV) / AnimationWeightTexH), 0, 0));
	Output.y = tex2Dlod(AnimationWeightSampler, float4(((0.5 + WeightU + 3) / AnimationWeightTexW), ((0.5 + WeightV) / AnimationWeightTexH), 0, 0));
	Output.z = tex2Dlod(AnimationWeightSampler, float4(((0.5 + WeightU + 4) / AnimationWeightTexW), ((0.5 + WeightV) / AnimationWeightTexH), 0, 0));
	Output.w = 0;
	return Output;
}

float4 GetWeightNormal(in int WeightID)
{
	float Dividend = WeightID * 8;
	int WeightU = GetMod(Dividend, AnimationWeightTexW);
	int WeightV = floor(Dividend / AnimationWeightTexW);
	float4 Output;
	Output.x = tex2Dlod(AnimationWeightSampler, float4(((0.5 + WeightU + 5) / AnimationWeightTexW), ((0.5 + WeightV) / AnimationWeightTexH), 0, 0));
	Output.y = tex2Dlod(AnimationWeightSampler, float4(((0.5 + WeightU + 6) / AnimationWeightTexW), ((0.5 + WeightV) / AnimationWeightTexH), 0, 0));
	Output.z = tex2Dlod(AnimationWeightSampler, float4(((0.5 + WeightU + 7) / AnimationWeightTexW), ((0.5 + WeightV) / AnimationWeightTexH), 0, 0));
	Output.w = 0;
	return Output;
}

float QuaternionDot(float4 Q1, float4 Q2)
{
	float Output = 0;
	Output = Q1.x * Q2.x + Q1.y * Q2.y + Q1.z * Q2.z + Q1.w * Q2.w;
	return Output;
}

float4 QuaternionNormalize(float4 Q1)
{
	float4 Output = float4(0, 0, 0, 0);
	float denominator = sqrt(Q1.x * Q1.x + Q1.y * Q1.y + Q1.z * Q1.z + Q1.w * Q1.w);

	Output.x = Q1.x / denominator;
	Output.y = Q1.y / denominator;
	Output.z = Q1.z / denominator;
	Output.w = Q1.w / denominator;
	return Output;
}

float4 QuaternionLerp(float4 Q1, float4 Q2, float t)
{
	float4 Output = float4(0, 0, 0, 0);
	Output = Q1 + (Q2 - Q1) * t;
	Output = QuaternionNormalize(Output);
	return Output;
}

float4 QuaternionSlerp(float4 Q1, float4 Q2, float t)
{
	float4 Output = float4(0, 0, 0, 0);

	Q1 = QuaternionNormalize(Q1);
	Q2 = QuaternionNormalize(Q2);

	float dot = QuaternionDot(Q1, Q2);

	if (dot > (float)0.9995)
		return QuaternionLerp(Q1, Q2, t);;

	if (dot < 0)	// 정말 중요함!★★
	{
		Q2 = -Q2;
		dot = -dot;
	}

	dot = clamp(dot, -1, 1);	// 필요한가..?
	float theta = acos(dot) * t;

	float4 Q3 = Q2 - Q1 * dot;
	Q3 = QuaternionNormalize(Q3);

	Output.x = (Q1.x * cos(theta) + Q3.x * sin(theta));
	Output.y = (Q1.y * cos(theta) + Q3.y * sin(theta));
	Output.z = (Q1.z * cos(theta) + Q3.z * sin(theta));
	Output.w = (Q1.w * cos(theta) + Q3.w * sin(theta));

	return Output;
}

float4 QuaternionInverse(float4 Q1)
{
	float4 Output = float4(0, 0, 0, 0);
	Output.x = -Q1.x;
	Output.y = -Q1.y;
	Output.z = -Q1.z;
	Output.w = Q1.w;
	return Output;
}

float4 QuaternionMultiply(float4 Q1, float4 Q2)
{
	float4 Output = float4(0, 0, 0, 0);

	Output.x = (Q2.w * Q1.x) + (Q2.x * Q1.w) + (Q2.y * Q1.z) - (Q2.z * Q1.y);
	Output.y = (Q2.w * Q1.y) - (Q2.x * Q1.z) + (Q2.y * Q1.w) + (Q2.z * Q1.x);
	Output.z = (Q2.w * Q1.z) + (Q2.x * Q1.y) - (Q2.y * Q1.x) + (Q2.z * Q1.w);
	Output.w = (Q2.w * Q1.w) - (Q2.x * Q1.x) - (Q2.y * Q1.y) - (Q2.z * Q1.z);

	return Output;
}

float4 QuaternionRotate(float4 Quaternion, float4 Position)
{
	float4 Output = float4(0, 0, 0, 0);
	float4 QuatInv = QuaternionInverse(Quaternion);

	Output = QuaternionMultiply(QuaternionMultiply(Quaternion, Position), QuatInv);

	return Output;
}

VS_OUTPUT vs_md5instancing(VS_INPUT_INSTANCING Input)
{
	VS_OUTPUT Output;

	// 인스턴스별 월드 행렬
	float4x4 matWorld = float4x4(Input.matWorld0.x, Input.matWorld0.y, Input.matWorld0.z, Input.matWorld0.w,
		Input.matWorld1.x, Input.matWorld1.y, Input.matWorld1.z, Input.matWorld1.w,
		Input.matWorld2.x, Input.matWorld2.y, Input.matWorld2.z, Input.matWorld2.w,
		Input.matWorld3.x, Input.matWorld3.y, Input.matWorld3.z, Input.matWorld3.w);

	// 애니메이션 적용
	float4 TempPosition = float4(0, 0, 0, 0);
	float4 TempNormal = float4(0, 0, 0, 0);

	int WeightStart = Input.WeightStart;
	int WeightNum = Input.WeightNum;
	int AnimID = Input.AnimationID;

	float CurFrameTime = Input.CurFrameTime;
	int Frame0 = Input.Frame0;
	int Frame1 = Input.Frame1;
	float Interpolation = CurFrameTime - Frame0;


	for (int i = 0; i < WeightNum; i++)
	{
		int WeightID = WeightStart + i;
		int JointID = GetWeightJointID(WeightID);
		float Bias = GetWeightBias(WeightID);

		float4 WeightPos = GetWeightPosition(WeightID);
		float4 WeightNorm = GetWeightNormal(WeightID);

		float4 Joint0Pos = GetJointPosition(AnimID, Frame0, JointID);
		float4 Joint0Ori = GetJointOrientation(AnimID, Frame0, JointID);
		float4 Joint1Pos = GetJointPosition(AnimID, Frame1, JointID);
		float4 Joint1Ori = GetJointOrientation(AnimID, Frame1, JointID);

		float4 InterPos = Joint0Pos + (Interpolation * (Joint1Pos - Joint0Pos));
		float4 InterOri = QuaternionSlerp(Joint0Ori, Joint1Ori, Interpolation);

		float4 RotatedPos = QuaternionRotate(Joint0Ori, WeightPos);
		TempPosition.x += (Joint0Pos.x + RotatedPos.x) * Bias;
		TempPosition.y += (Joint0Pos.y + RotatedPos.y) * Bias;
		TempPosition.z += (Joint0Pos.z + RotatedPos.z) * Bias;
		TempPosition.w = 1;

		/*
		float4 RotatedPos = QuaternionRotateF(InterOri, WeightPos);
		TempPosition.x += (InterPos.x + RotatedPos.x) * Bias;
		TempPosition.y += (InterPos.y + RotatedPos.y) * Bias;
		TempPosition.z += (InterPos.z + RotatedPos.z) * Bias;
		TempPosition.w = 1;
		*/

		float4 RotatedNorm = QuaternionRotate(Joint0Ori, WeightNorm);
		TempNormal -= RotatedNorm * Bias;
	}

	Output.Position = mul(TempPosition, matWorld);
	Output.Position = mul(Output.Position, matVP);
	Output.Normal = normalize(TempNormal.xyz);
	Output.Texture = Input.Texture;

	return(Output);
}

VS_OUTPUT vs_noinstancing(VS_INPUT_NoInstancing Input)
{
	VS_OUTPUT Output;

	Output.Position = mul(Input.Position, matWVP);
	Output.Normal = normalize(Input.Normal);
	Output.Texture = Input.Texture;

	return(Output);
}

float4 ps_main(VS_OUTPUT Input) : COLOR
{
	if (UseTexture)
	{
		float4 albedo = tex2D(DiffuseSampler, Input.Texture);
		return albedo.rgba;
	}
	else
	{
		return MtrlDiffuse;
	}
}

float4 ps_color(VS_OUTPUT Input) : COLOR
{
	float4 color = float4(1, 0, 0, 1);
	return color;
}

technique HLSLMD5Instancing
{
	pass Pass_0
	{
		VertexShader = compile vs_3_0 vs_md5instancing();
		PixelShader = compile ps_3_0 ps_main();
	}
}

technique HLSLNoInstancing
{
	pass Pass_0
	{
		VertexShader = compile vs_3_0 vs_noinstancing();
		PixelShader = compile ps_3_0 ps_main();
	}
}