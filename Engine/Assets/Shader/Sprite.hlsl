#pragma pack_matrix(row_major)

// 定数バッファ
cbuffer TransformBuffer : register(b0)
{
    float4x4 wvp;
};

cbuffer SpriteBuffer : register(b1)
{
    float4 color;
    float4 uvRect; // { u, v, w, h }
};

// テクスチャ・サンプラー
Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

// 頂点シェーダ入力
struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

// ピクセルシェーダ入力
struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// 頂点シェーダ
PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = mul(float4(input.position, 1.0f), wvp);
    
    // uvRect に基づいて UV を変換
    output.uv = float2(
        uvRect.x + input.uv.x * uvRect.z,
        uvRect.y + input.uv.y * uvRect.w
    );
    
    return output;
}

// ピクセルシェーダ
float4 PSMain(PSInput input) : SV_TARGET
{
    // テクスチャサンプリング × カラー
    float4 texColor = gTexture.Sample(gSampler, input.uv);
    return texColor * color;
}