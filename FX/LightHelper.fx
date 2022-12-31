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
	// 초기화
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 평행광은 모든 광선의 방향이 동일하다. 평면에서 빛 벡터는 광선의 반대방향.
	float3 lightVec = -L.Direction;

	// 주변광은 오브젝트의 반사율 x 빛 자체의 반사율로 만들어진다
	ambient = mat.Ambient * L.Ambient;	

	// 분산광 계수를 계산하기 위해 정점의 법선벡터 방향으로 내적(람베르트 코사인)
	float diffuseFactor = dot(lightVec, normal);

	// 쉐이더에서 if문은 성능에 큰 악영향을 미친다. 
	// 그 이유는 GPU가 32개의 쓰레드 단위로 동시 수행하기 때문.
	// GPU는 동시에 동일한 명령을 수행해야 하므로 모든 계산을 미리 수행한 후 조건에 맞는 결과만 변수에 저장한다.
	// 이를 분기 발산(branch divergence)라 한다.
	// 만약 분기 값이 모든 쓰레드에서 동일한 값을 갖는다면 모든 분기를 미리 계산하지 않는다.
	// flatten은 분기문을 펼쳐 모든 분기를 평가한 후 결과를 저장하도록 한다.
	// 분기문을 평탄화하여 if문을 들어가는데 드는 비용을 제거할 수 있으며, 
	// diffuseFactor 값이 모든 쓰레드에서 (모든 정점에서) 0 이하의 값을 갖는 경우
	// 컴파일러가 해당 코드를 실행시키지 않으므로 동적 분기에 비해 속도가 더 빠를 수 있다.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a point light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
				   out float4 ambient, out float4 diffuse, out float4 spec)
{
	// output 초기화
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Light 벡터 계산(표면에 도달하는 하나의 광선)
	float3 lightVec = L.Position - pos;
		
	// 표면의 점과 광원 사이의 거리를 계산
	float d = length(lightVec);
	
	// 점광은 감쇠를 사용하여 빛의 영향을 받는 범위를 만들어내지만 최대 적용 범위를 명시화하여
	// 셰이더가 일찍 결과를 출력하게 할 수 있다
	// 이것은 비싼 조명 계산과 동적 분기를 생략할 수 있게 해준다
	if( d > L.Range )
		return;
		
	// 정규화
	lightVec /= d; 
	
	// 주변광(동일)
	ambient = mat.Ambient * L.Ambient;	

	// 분산광 계수 계산(람베르트 코사인)
	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}

	// 빛의 범위가 존재하므로 감쇠도 고려해야한다
	float att = 1.0f / dot(L.Att, float3(1.0f, d, d*d));

	diffuse *= att;
	spec    *= att;
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a spotlight.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
				  out float4 ambient, out float4 diffuse, out float4 spec)
{
	// output 초기화
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 빛 벡터 계산
	float3 lightVec = L.Position - pos;
		
	// 빛 벡터 길이 계산
	float d = length(lightVec);
	
	// 최대 범위 계산
	if( d > L.Range )
		return;
		
	// 정규화
	lightVec /= d; 
	
	// 주변광 계산
	ambient = mat.Ambient * L.Ambient;	

	//분산광 계수 계산(람베르트 코사인)
	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}
	
	// 점적광 계수 계산
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);

	// 점적광은 거리와 점적광 계수에 영향을 받는다
	float att = spot / dot(L.Att, float3(1.0f, d, d*d));

	ambient *= spot;
	diffuse *= att;
	spec    *= att;
}

 
 