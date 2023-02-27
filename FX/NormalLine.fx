
cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float4 Color     : COLOR;
};

struct VertexOut
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float4 Color     : COLOR;
};

struct GeoOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float4 Color     : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	vout.PosL = vin.PosL;
	vout.NormalL = vin.NormalL;
	vout.Color = vin.Color;

	return vout;
}

// IF we subdivide twice, we can output 32 vertices.
[maxvertexcount(2)]
void GS(point VertexOut gin[1], inout LineStream<GeoOut> lineStream)
{
	GeoOut gout[2];
	gout[0].PosW = mul(float4(gin[0].PosL, 1.0f), gWorld).xyz;
	gout[0].NormalW = mul(gin[0].NormalL, (float3x3)gWorldInvTranspose);
	gout[0].PosH = mul(float4(gin[0].PosL, 1.0f), gWorldViewProj);
	gout[0].Color = gin[0].Color;
	lineStream.Append(gout[0]);

	float3 pos = gin[0].PosL + gin[0].NormalL * 2.0f;
	gout[1].PosW = mul(float4(pos, 1.0f), gWorld).xyz;
	gout[1].NormalW = mul(gin[0].NormalL, (float3x3)gWorldInvTranspose);
	gout[1].PosH = mul(float4(pos, 1.0f), gWorldViewProj);
	gout[1].Color = gin[0].Color;
	lineStream.Append(gout[1]);

	lineStream.RestartStrip();
}

[maxvertexcount(2)]
void GS_TRI(triangle VertexOut gin[3], inout LineStream<GeoOut> lineStream)
{
	float3 normal = cross(gin[1].PosL - gin[0].PosL, gin[2].PosL - gin[0].PosL);
	normal = normalize(normal);

	float3 lineStart = (gin[0].PosL + gin[1].PosL + gin[2].PosL)/3.0f;
	float3 lineEnd = lineStart + normal * 2.0f;

	GeoOut gout[2];
	gout[0].PosW = mul(float4(lineStart, 1.0f), gWorld).xyz;
	gout[0].NormalW = mul(normal, (float3x3)gWorldInvTranspose);
	gout[0].PosH = mul(float4(lineStart, 1.0f), gWorldViewProj);
	gout[0].Color = gin[0].Color;
	lineStream.Append(gout[0]);

	gout[1].PosW = mul(float4(lineEnd, 1.0f), gWorld).xyz;
	gout[1].NormalW = mul(normal, (float3x3)gWorldInvTranspose);
	gout[1].PosH = mul(float4(lineEnd, 1.0f), gWorldViewProj);
	gout[1].Color = gin[0].Color;
	lineStream.Append(gout[1]);

	lineStream.RestartStrip();
}

float4 PS(GeoOut pin) : SV_Target
{
	return pin.Color;
}

technique11 ColorTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}

technique11 TriNormalTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(CompileShader(gs_5_0, GS_TRI()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
};