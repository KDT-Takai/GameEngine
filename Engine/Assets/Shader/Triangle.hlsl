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
// 今回は座標変換なし（NDC座標をそのまま渡す）
// 将来: float4x4 の変換行列を定数バッファから受け取ってここで掛ける
PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = float4(input.position, 1.0f);
    output.color = input.color;
    return output;
}

// ピクセルシェーダ
// 今回は頂点カラーをそのまま返す（各ピクセルで補間された色が来る）
// 将来: テクスチャサンプリングやライティング計算をここに追加する
float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
