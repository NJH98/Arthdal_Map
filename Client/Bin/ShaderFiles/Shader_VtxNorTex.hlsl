#include "Shader_Engine_Defines.hlsli"

matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

vector			g_vBrushPos = vector(30.f, 0.f, 10.f, 1.f);
float			g_fBrushRange = 7.f;

texture2D		g_DiffuseTexture[2]; /* 지형 픽셀이 빛을 받으면 반사해야할 재질정보를 담은것이다. */
texture2D		g_NomalTexture[2];
texture2D		g_BrushTexture;
texture2D		g_MaskTexture;

float3			g_MouseWorldPos;

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
    vector vMask = g_MaskTexture.Sample(LinearSampler, In.vTexcoord);

	// 디퓨즈 이미지
	vector		vSourDiffuse = g_DiffuseTexture[0].Sample(LinearSampler, In.vTexcoord * 30.f);
	vector		vDestDiffuse = g_DiffuseTexture[1].Sample(LinearSampler, In.vTexcoord * 30.f);

	vector		vMtrlDiffuse = vSourDiffuse * vMask.r + vDestDiffuse * (1.f - vMask.r) ;

	// 최종 색상
	Out.vDiffuse = vector(vMtrlDiffuse.rgb, 1.f);
	
	// 노말 이미지
    vector vSourNomal = g_NomalTexture[0].Sample(LinearSampler, In.vTexcoord * 30.f);
    vector vDestNomal = g_NomalTexture[1].Sample(LinearSampler, In.vTexcoord * 30.f);

    vector vMtrlNomal = vSourNomal * vMask.r + vDestNomal * (1.f - vMask.r);

	// 최종 노말값..?
    Out.vNormal = vector(vMtrlNomal.rgb, 1.f);
	// -1.f ~ 1.f -> 0.f ~ 1.f
    //Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	
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