#pragma once
#include "IShaderLoader.hpp"

namespace Engine::Graphics
{
	// ============================================================
	//  PrecompiledShaderLoader（将来実装予定）
	//  RuntimeShaderLoader からの切り替え手順:
	//    1. RuntimeShaderLoader → PrecompiledShaderLoader に差し替え
	//    2. ShaderLoadDesc::path を .hlsl → .cso に変更
	//    3. entryPoint / target は無視されるので変更不要
	//
	//  事前コンパイルの設定方法（Visual Studio）:
	//    対象の .hlsl ファイルを右クリック → プロパティ
	//    → 全般 → 項目の種類 → 「HLSLコンパイラ」を選択
	//    → HLSLコンパイラ → オブジェクトファイル名に出力先(.cso)を指定
	// ============================================================
	//class PrecompiledShaderLoader : public IShaderLoader
	//{
	//public:
	//	// desc.path       : .cso ファイルパス
	//	// desc.entryPoint : 無視される
	//	// desc.target     : 無視される
	//	bool Load(const ShaderLoadDesc& desc, ID3DBlob** ppBlob) override;
	//};
} // Engine::Graphics