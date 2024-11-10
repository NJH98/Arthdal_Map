
#include "Shader_Engine_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_DiffuseTexture;
texture2D g_NormalTexture;
texture2D g_MaskTexture;

float2 g_MaskUV_Move;
vector g_vCamPosition;
float3 g_Color;

struct VS_IN
{
	/* InputSlot : 0 */
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float3 vTangent : TANGENT;
    
	/* InputSlot : 1 */
    float4 vRight : TEXCOORD1;
    float4 vUp : TEXCOORD2;
    float4 vLook : TEXCOORD3;
    float4 vTranslation : TEXCOORD4;
    uint DepthNum : TEXCOORD5;
    uint InstanceID : SV_InstanceID;
    
};

struct VS_OUT_NORMAL
{
    float4 vPosition : SV_POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    uint DepthNum : TEXCOORD5;
};

/* 1. 정점의 변환과정을 수행한다. */
/* 2. 정점의 구성 정보를 변형한다. */
VS_OUT_NORMAL VS_MAIN( /*정점*/VS_IN In)
{
    VS_OUT_NORMAL Out = (VS_OUT_NORMAL) 0;
	
    matrix TransformMatrix = float4x4(In.vRight, In.vUp, In.vLook, In.vTranslation);

	/* 로컬상태에서의 움직임 구현. */
    vector vPosition = mul(float4(In.vPosition, 1.f), TransformMatrix);


    vPosition = mul(vPosition, g_ViewMatrix);
    vPosition = mul(vPosition, g_ProjMatrix);

    Out.vPosition = vPosition;
    Out.vTexcoord = In.vTexcoord;
    Out.vNormal = normalize(mul(vector(In.vNormal, 0.f), g_WorldMatrix)).xyz;
    Out.vProjPos = Out.vPosition;
    Out.vTangent = normalize(mul(vector(In.vTangent, 0.f), g_WorldMatrix)).xyz;
    Out.vBinormal = normalize(cross(Out.vNormal, Out.vTangent));
    Out.DepthNum = In.DepthNum;
    
    return Out;
}

struct PS_IN_NORMAL
{
    float4 vPosition : SV_POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    uint DepthNum : TEXCOORD5;
};

struct PS_OUT
{
    vector vDiffuse : SV_TARGET0;
    vector vNormal : SV_TARGET1;
    vector vDepth : SV_TARGET2;
    vector vPickDepth : SV_TARGET3;
};

PS_OUT PS_MAIN_NORMAL(PS_IN_NORMAL In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);

    float2 vNormalXY = g_NormalTexture.Sample(LinearSampler, In.vTexcoord).rg * 2.0f - 1.0f;
	/* 로컬상의 변환되지 않은 노말벡터를 구했다. */
	/* 로컬스페이스 => 정점의로컬스페이스(x), 노멀벡터당 하나씩 로컬스페이스를 독립적으로 구성했다. */
    float vNormalZ = sqrt(saturate(1.0f - dot(vNormalXY, vNormalXY)));
	
    float3 vNormal = float3(vNormalXY, vNormalZ);
    //float3			vNormal = float3(vNormalDesc.b, vNormalDesc.g, vNormalDesc.r) * 2.f - 1.f;
	
    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

    vNormal = normalize(mul(vNormal, WorldMatrix));

    if (0.3f >= vDiffuse.a)
        discard;

    Out.vDiffuse = vDiffuse;

	// 최종 노말값
	// -1.f ~ 1.f -> 0.f ~ 1.f  최소값 0으로 최댓값 1로 조정
    Out.vNormal = vector(vNormal * 0.5f + 0.5f, 0.f);
	
	// 픽셀피킹을 위한 깊이값 , g_DepthNum = 피킹된 객체를 찾아내기위한 고유 번호
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.f, 0.f, 0.f);
    Out.vPickDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, float(In.DepthNum), 1.f);
	
    return Out;
}


technique11 DefaultTechnique
{
    pass Basic
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, vector(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL ();
    }


}   