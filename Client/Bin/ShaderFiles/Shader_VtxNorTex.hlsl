#include "Shader_Engine_Defines.hlsli"

matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D		g_DiffuseTexture[7];
texture2D		g_NomalTexture[7];
texture2D		g_MaskTexture[2];

struct VS_IN
{
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{	
	float4 vPosition : SV_POSITION;
	float4 vNormal : NORMAL;
	float2 vTexcoord : TEXCOORD0;
	float4 vWorldPos : TEXCOORD1;
	float4 vProjPos : TEXCOORD2;
};

VS_OUT VS_MAIN(/*정점*/VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;	
	
	vector		vPosition = mul(vector(In.vPosition, 1.f), g_WorldMatrix);

	vPosition = mul(vPosition, g_ViewMatrix);
	vPosition = mul(vPosition, g_ProjMatrix);

	Out.vPosition = vPosition;
	Out.vNormal = normalize(mul(vector(In.vNormal, 0.f), g_WorldMatrix));
	Out.vTexcoord = In.vTexcoord;
	Out.vWorldPos = mul(vector(In.vPosition, 1.f), g_WorldMatrix);
	Out.vProjPos = vPosition;

	return Out;
}


struct PS_IN
{
	float4 vPosition : SV_POSITION;
	float4 vNormal : NORMAL;
	float2 vTexcoord : TEXCOORD0;
	float4 vWorldPos : TEXCOORD1;
	float4 vProjPos : TEXCOORD2;
};

struct PS_OUT
{
	vector vDiffuse : SV_TARGET0;
	vector vNormal : SV_TARGET1;
	vector vDepth : SV_TARGET2;
	vector vPickDepth : SV_TARGET3;
};


PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;
	
	// 마스크 이미지
    vector vMask0 = g_MaskTexture[0].Sample(LinearSampler, In.vTexcoord);
    vector vMask1 = g_MaskTexture[1].Sample(LinearSampler, In.vTexcoord);

	// 디퓨즈 이미지
    vector		vBaseDiffuse = g_DiffuseTexture[0].Sample(LinearSampler, In.vTexcoord * 30.f);
	vector		vDestDiffuse1 = g_DiffuseTexture[1].Sample(LinearSampler, In.vTexcoord * 30.f);
    vector		vDestDiffuse2 = g_DiffuseTexture[2].Sample(LinearSampler, In.vTexcoord * 30.f);
    vector		vDestDiffuse3 = g_DiffuseTexture[3].Sample(LinearSampler, In.vTexcoord * 30.f);
    vector		vDestDiffuse4 = g_DiffuseTexture[4].Sample(LinearSampler, In.vTexcoord * 30.f);
    vector		vDestDiffuse5 = g_DiffuseTexture[5].Sample(LinearSampler, In.vTexcoord * 30.f);
    vector		vDestDiffuse6 = g_DiffuseTexture[6].Sample(LinearSampler, In.vTexcoord * 30.f);

    vector		vMtrlDiffuse = vBaseDiffuse * vMask0.r + vDestDiffuse1 * (1.f - vMask0.r);
    vMtrlDiffuse = vMtrlDiffuse * vMask0.g + vDestDiffuse2 * (1.f - vMask0.g);
    vMtrlDiffuse = vMtrlDiffuse * vMask0.b + vDestDiffuse3 * (1.f - vMask0.b);
    vMtrlDiffuse = vMtrlDiffuse * vMask1.r + vDestDiffuse4 * (1.f - vMask1.r);
    vMtrlDiffuse = vMtrlDiffuse * vMask1.g + vDestDiffuse5 * (1.f - vMask1.g);
    vMtrlDiffuse = vMtrlDiffuse * vMask1.b + vDestDiffuse6 * (1.f - vMask1.b);
	
	// 최종 색상
	Out.vDiffuse = vector(vMtrlDiffuse.rgb, 1.f);
	
	// 노말 텍스처에서 X, Y만 샘플링하고 Z 채널을 계산하여 노말 벡터를 구성합니다.
    float2 vBaseNormalXY = g_NomalTexture[0].Sample(LinearSampler, In.vTexcoord * 30.f).rg * 2.0f - 1.0f;
    float2 vDestNormal1XY = g_NomalTexture[1].Sample(LinearSampler, In.vTexcoord * 30.f).rg * 2.0f - 1.0f;
    float2 vDestNormal2XY = g_NomalTexture[2].Sample(LinearSampler, In.vTexcoord * 30.f).rg * 2.0f - 1.0f;
    float2 vDestNormal3XY = g_NomalTexture[3].Sample(LinearSampler, In.vTexcoord * 30.f).rg * 2.0f - 1.0f;
    float2 vDestNormal4XY = g_NomalTexture[4].Sample(LinearSampler, In.vTexcoord * 30.f).rg * 2.0f - 1.0f;
    float2 vDestNormal5XY = g_NomalTexture[5].Sample(LinearSampler, In.vTexcoord * 30.f).rg * 2.0f - 1.0f;
    float2 vDestNormal6XY = g_NomalTexture[6].Sample(LinearSampler, In.vTexcoord * 30.f).rg * 2.0f - 1.0f;

	// X, Y를 기반으로 Z 성분을 계산하여 float3 형태로 노말 벡터를 구성합니다.
    float3 vBaseNormal = float3(vBaseNormalXY, sqrt(saturate(1.0f - dot(vBaseNormalXY, vBaseNormalXY))));
    float3 vDestNormal1 = float3(vDestNormal1XY, sqrt(saturate(1.0f - dot(vDestNormal1XY, vDestNormal1XY))));
    float3 vDestNormal2 = float3(vDestNormal2XY, sqrt(saturate(1.0f - dot(vDestNormal2XY, vDestNormal2XY))));
    float3 vDestNormal3 = float3(vDestNormal3XY, sqrt(saturate(1.0f - dot(vDestNormal3XY, vDestNormal3XY))));
    float3 vDestNormal4 = float3(vDestNormal4XY, sqrt(saturate(1.0f - dot(vDestNormal4XY, vDestNormal4XY))));
    float3 vDestNormal5 = float3(vDestNormal5XY, sqrt(saturate(1.0f - dot(vDestNormal5XY, vDestNormal5XY))));
    float3 vDestNormal6 = float3(vDestNormal6XY, sqrt(saturate(1.0f - dot(vDestNormal6XY, vDestNormal6XY))));

	// 마스크 값을 기반으로 노말 벡터들을 혼합합니다.
    float3 vMtrlNormal = vBaseNormal * vMask0.r + vDestNormal1 * (1.f - vMask0.r);
    vMtrlNormal = vMtrlNormal * vMask0.g + vDestNormal2 * (1.f - vMask0.g);
    vMtrlNormal = vMtrlNormal * vMask0.b + vDestNormal3 * (1.f - vMask0.b);
    vMtrlNormal = vMtrlNormal * vMask1.r + vDestNormal4 * (1.f - vMask1.r);
    vMtrlNormal = vMtrlNormal * vMask1.g + vDestNormal5 * (1.f - vMask1.g);
    vMtrlNormal = vMtrlNormal * vMask1.b + vDestNormal6 * (1.f - vMask1.b);
	
	// 최종 노말 값 조정 (-1 ~ 1 범위를 0 ~ 1 범위로 변환)
    //Out.vNormal = float4((In.vNormal.xyz + vMtrlNormal) * 0.5f + 0.5f, 0.f);
    float3 vFinalNormal = normalize(In.vNormal.xyz * 2.0f - 1.0f + (vMtrlNormal * 2.0f - 1.0f));
    Out.vNormal = float4(vFinalNormal * 0.5f + 0.5f, 0.0f);

	
	// 픽셀피킹을 위한 깊이값
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.f, 0.f, 0.f);
    Out.vPickDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.f, 1.f);

	return Out;
}

technique11	DefaultTechnique
{
	pass Terrain
	{
		SetRasterizerState(RS_Default);
		// SetRasterizerState(RS_Wireframe);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, vector(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

}