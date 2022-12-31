//***************************************************************************************
// LightHelper.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Structures and functions for lighting calculations.
//***************************************************************************************

struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

struct PointLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Att;
	float pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Direction;
	float Spot;

	float3 Att;
	float pad;
};

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular; // w = SpecPower
	float4 Reflect;
};

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a directional light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeDirectionalLight(Material mat, DirectionalLight L,
	float3 normal, float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	// �ʱ�ȭ
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// ���౤�� ��� ������ ������ �����ϴ�. ��鿡�� �� ���ʹ� ������ �ݴ����.
	float3 lightVec = -L.Direction;

	// �ֺ����� ������Ʈ�� �ݻ��� x �� ��ü�� �ݻ����� ���������
	ambient = mat.Ambient * L.Ambient;

	// �л걤 ����� ����ϱ� ���� ������ �������� �������� ���� (������Ʈ �ڻ���)
	float diffuseFactor = dot(lightVec, normal);

	float3 v = reflect(-lightVec, normal);
	float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
	//ī�� �������� diffuse Light�� Specular Light�� �̻��Լ��� ��ȯ�Ͽ� ����Ѵ�
	[flatten]
	if (specFactor <= 0)
	{
		specFactor = 0;
	}
	else if (specFactor <= 0.5)
	{
		specFactor = 0.5;
	}
	else if (specFactor <= 1.0)
	{
		specFactor = 0.8;
	}
	spec = specFactor * mat.Specular * L.Specular;

	[flatten]
	if (diffuseFactor > 1.0)
	{
		diffuseFactor = 0.0;
	}
	else if (diffuseFactor > 0.5)
	{
		diffuseFactor = 1.0;
	}
	else if (diffuseFactor > 0.0)
	{
		diffuseFactor = 0.6;
	}
	else
	{
		diffuseFactor = 0.4;
	}
	diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a point light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 spec)
{
	// output �ʱ�ȭ
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Light ���� ���(ǥ�鿡 �����ϴ� �ϳ��� ����)
	float3 lightVec = L.Position - pos;

	// ǥ���� ���� ���� ������ �Ÿ��� ���
	float d = length(lightVec);

	// ������ ���踦 ����Ͽ� ���� ������ �޴� ������ �������� �ִ� ���� ������ ���ȭ�Ͽ�
	// ���̴��� ���� ����� ����ϰ� �� �� �ִ�
	// �̰��� ��� ���� ���� ���� �б⸦ ������ �� �ְ� ���ش�
	if (d > L.Range)
		return;

	// ����ȭ
	lightVec /= d;

	// �ֺ���(����)
	ambient = mat.Ambient * L.Ambient;

	// �л걤 ��� ���(������Ʈ �ڻ���)
	float diffuseFactor = dot(lightVec, normal);
	float3 v = reflect(-lightVec, normal);
	float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
	//ī�� �������� diffuse Light�� Specular Light�� �̻��Լ��� ��ȯ�Ͽ� ����Ѵ�
	[flatten]
	if (specFactor <= 0)
	{
		specFactor = 0;
	}
	else if (specFactor <= 0.5)
	{
		specFactor = 0.5;
	}
	else if (specFactor <= 1.0)
	{
		specFactor = 0.8;
	}
	spec = specFactor * mat.Specular * L.Specular;

	[flatten]
	if (diffuseFactor > 1.0)
	{
		diffuseFactor = 0.0;
	}
	else if (diffuseFactor > 0.5)
	{
		diffuseFactor = 1.0;
	}
	else if (diffuseFactor > 0.0)
	{
		diffuseFactor = 0.6;
	}
	else
	{
		diffuseFactor = 0.4;
	}
	diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;

	// ���� ������ �����ϹǷ� ���赵 ����ؾ��Ѵ�
	float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));

	diffuse *= att;
	spec *= att;
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a spotlight.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 spec)
{
	// output �ʱ�ȭ
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// �� ���� ���
	float3 lightVec = L.Position - pos;

	// �� ���� ���� ���
	float d = length(lightVec);

	// �ִ� ���� ���
	if (d > L.Range)
		return;

	// ����ȭ
	lightVec /= d;

	// �ֺ��� ���
	ambient = mat.Ambient * L.Ambient;

	//�л걤 ��� ���(������Ʈ �ڻ���)
	float diffuseFactor = dot(lightVec, normal);
	float3 v = reflect(-lightVec, normal);
	float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
	//ī�� �������� diffuse Light�� Specular Light�� �̻��Լ��� ��ȯ�Ͽ� ����Ѵ�
	[flatten]
	if (specFactor <= 0)
	{
		specFactor = 0;
	}
	else if (specFactor <= 0.5)
	{
		specFactor = 0.5;
	}
	else if (specFactor <= 1.0)
	{
		specFactor = 0.8;
	}
	spec = specFactor * mat.Specular * L.Specular;

	[flatten]
	if (diffuseFactor > 1.0)
	{
		diffuseFactor = 0.0;
	}
	else if (diffuseFactor > 0.5)
	{
		diffuseFactor = 1.0;
	}
	else if (diffuseFactor > 0.0)
	{
		diffuseFactor = 0.6;
	}
	else
	{
		diffuseFactor = 0.4;
	}
	diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;

	// ������ ��� ���
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);

	// �������� �Ÿ��� ������ ����� ������ �޴´�
	float att = spot / dot(L.Att, float3(1.0f, d, d * d));

	ambient *= spot;
	diffuse *= att;
	spec *= att;
}



cbuffer cbPerFrame
{
	DirectionalLight gDirLight;
	PointLight gPointLight;
	SpotLight gSpotLight;
	float3 gEyePosW;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	Material gMaterial;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION;
	float3 NormalW : NORMAL;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Transform to world space space.
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
	pin.NormalW = normalize(pin.NormalW);

	float3 toEyeW = normalize(gEyePosW - pin.PosW);

	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sum the light contribution from each light source.
	float4 A, D, S;

	ComputeDirectionalLight(gMaterial, gDirLight, pin.NormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec += S;

	ComputePointLight(gMaterial, gPointLight, pin.PosW, pin.NormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec += S;

	ComputeSpotLight(gMaterial, gSpotLight, pin.PosW, pin.NormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec += S;

	float4 litColor = ambient + diffuse + spec;

	// Common to take alpha from diffuse material.
	litColor.a = gMaterial.Diffuse.a;

	return litColor;
}

technique11 LightTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}