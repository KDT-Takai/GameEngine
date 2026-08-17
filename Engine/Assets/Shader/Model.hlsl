// 定数バッファ
cbuffer ModelTransformBuffer : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
    float4 color;
};

// テクスチャ・サンプラー
Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

// 頂点シェーダ入力
struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

// ピクセルシェーダ入力
struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

// 頂点シェーダ
PSInput VSMain(VSInput input)
{
    PSInput output;

    float4 worldPos = mul(float4(input.position, 1.0f), world);
    float4 viewPos = mul(worldPos, view);
    output.position = mul(viewPos, projection);

    // 法線をワールド空間に変換
    output.normal = mul(input.normal, (float3x3) world);
    output.uv = input.uv;

    return output;
}

// ピクセルシェーダ
float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = gTexture.Sample(gSampler, input.uv);
    return texColor * color;
}