# ModelConverter (ビルド済みツール)

`.fbx` を GameEngine ランタイム用バイナリ `.mdl` に変換するオフラインCLIツール。

このディレクトリにあるのはビルド済みの成果物のみ(`ModelConverter.exe` + `assimp-vc143-mt.dll`)。
ソースコード・設計ドキュメント・変更方法は独立リポジトリ [ModelConverter](https://github.com/ShaF-u/ModelConverter) を参照。
`.mdl` フォーマット仕様・`Vertex` レイアウトはそちらの `docs/DESIGN.md` が正。

## 使い方

```powershell
.\ModelConverter.exe <input.fbx> [output.mdl]
```

`output.mdl` を省略した場合、入力ファイルと同じ場所・同じ名前で拡張子だけ `.mdl` になる。

## 更新方法

1. `ModelConverter` リポジトリ側でソースを変更・ビルド(Release|x64)
2. `bin\Release\ModelConverter.exe` と `bin\Release\assimp-vc143-mt.dll` をこのディレクトリへ上書きコピー
