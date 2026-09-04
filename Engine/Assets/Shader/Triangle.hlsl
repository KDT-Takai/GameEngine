#pragma pack_matrix(row_major)

cbuffer TransformBuffer : register(b0)
{
    float4x4 wvp; // World * View * Projection 行列
};

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

// 頂点シェーダ
PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = mul(float4(input.position, 1.0f), wvp);
    output.color = input.color;
    return output;
}

// ピクセルシェーダ
float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
