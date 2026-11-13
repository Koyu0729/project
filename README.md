# ゲーム制作ポートフォリオ

KICK LOOPではアクションゲームの操作を、NodeBattleではノードエディターと実行の仕組みを制作しました。

## ダウンロードして遊ぶ

1. [ZIPをダウンロード](https://github.com/Koyu0729/project/archive/refs/heads/main.zip)し、すべて展開します。
2. 展開先で遊びたい作品の実行フォルダーを開き、表にあるexeを起動します。

| 作品 | 実行フォルダー | 起動するファイル |
| --- | --- | --- |
| KICK LOOP | [実行ファイル](<KickLoop/実行ファイル（KICK LOOP）/実行ファイル>) | `KICKLOOP.exe` |
| NodeBattle | [NodeBattle](<ビジュアルプログラミング/実行ファイル（ビジュアルプログラミング）/NodeBattle>) | `NodeBattle.exe` |
| DynamicBreaker | [DynamicBreaker](DynamicBreaker/DynamicBreaker) | `DynamicBreaker.exe` |
| ANNiHILATION | [実行環境データ](<ANNiHILATION/実行ファイル（ANNiHILATION）/実行環境データ>) | `ANNiHILATION.exe` |

Windows 10／11のx64向けです。DLL・素材・音声は実行フォルダーに同梱しています。遊ぶときにVisual Studio、Unity Editor、CRI SDKのインストールは不要です。

## 作品一覧

- [KICK LOOP](#kick-loop)
- [NodeBattle（ビジュアルプログラミング）](#nodebattleビジュアルプログラミング)
- [ANNiHILATION](#annihilation)
- [DynamicBreaker](#dynamicbreaker)
- その他の作品
- ポートフォリオ

## KICK LOOP

C++ / DirectX 11で制作した個人作品です。壁キックや急降下をつないで、制限時間内にゴールを目指します。

- [プレイ動画（完成版）](KickLoop/%E5%AE%9F%E8%A1%8C%E5%8B%95%E7%94%BB%EF%BC%88KICK%20LOOP%EF%BC%89%EF%BC%88%E5%AE%8C%E6%88%90%E7%89%88%EF%BC%89.mp4)
- [アクションの判定・切り替え](KickLoop/%E3%83%97%E3%83%AD%E3%82%B8%E3%82%A7%E3%82%AF%E3%83%88%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%EF%BC%88KICK%20LOOP%EF%BC%89/Project/SourceCode/Player/ActionResolution/playerActionResolver.cpp)：操作要求を受け取り、プレイヤーの状況に応じて行動を決める処理です。
- [GPUパーティクル](KickLoop/%E3%83%97%E3%83%AD%E3%82%B8%E3%82%A7%E3%82%AF%E3%83%88%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%EF%BC%88KICK%20LOOP%EF%BC%89/Project/shader/gpuParticleCS.hlsl)：粒子の位置や寿命を更新するコンピュートシェーダーです。

### 収録内容

- プロジェクトファイル
- [実行フォルダー](<KickLoop/実行ファイル（KICK LOOP）/実行ファイル>)
- 実行動画：完成版
- 実行動画：プロトタイプ

### 遊び方

マウスで「START」に視点を合わせます。ボタンが水色に変わって手前に寄ったら、左クリックでゲームを開始します。

ゴール後は約5秒でカメラが自動で動き始めます。

### Visual Studioでビルドする

Visual Studio 2022で「C++ によるデスクトップ開発」（MSVC v143、Windows SDK）をインストールします。

1. [リポジトリをZIPでダウンロード](https://github.com/Koyu0729/project/archive/refs/heads/main.zip)して展開します。
2. 「プロジェクトファイル（KICK LOOP）」の [00_GM31_初期化.sln](<KickLoop/プロジェクトファイル（KICK LOOP）/Project/00_GM31_初期化.sln>) を開きます。
3. `x64` の `Debug` または `Release` を選んで F5 を押します。

AssimpとDirectXTexは同梱しています。ソースのビルドには、下記のCRI SDKが必要です。exeと実行に必要なファイルは `Project/build/bin/x64/Debug/` または `Project/build/bin/x64/Release/` に出力されます。

#### ソースビルド用のCRI SDK

[CRI ADX LE公式ページ](https://game.criware.jp/products/adx-le/)から **ADX LE Native SDK** を入手し、同じバージョンのWindows x64用ファイルを `Project/` 内の次の場所に配置します。

| ファイル | 配置先 |
| --- | --- |
| SDKのヘッダー一式 | `Libraries/cri/include/` |
| `cri_ware_pcx64_le_import.lib` | `Libraries/lib/x64/` |
| `cri_ware_pcx64_le.dll` | `runtime/` |

配置後にソリューションを開き、`Debug | x64` または `Release | x64` でF5を押します。

### 使用ライブラリ

このソフトウェアには、（株）ＣＲＩ・ミドルウェアの「CRIWARE」が使用されています。 [CRI ADX LE 使用許諾文](<KickLoop/実行ファイル（KICK LOOP）/実行ファイル/CRI_ADX_LE_End_User_License_Agreement.txt>)

モデルとアニメーションの読み込みに Assimp 5.2.4 を使用しています。[Assimp のライセンス](<KickLoop/実行ファイル（KICK LOOP）/実行ファイル/Assimp-LICENSE.txt>)

テクスチャの読み込みにはDirectXTexを使用しています。 [DirectXTexのライセンス](<KickLoop/プロジェクトファイル（KICK LOOP）/Project/Libraries/DirectXTex/DirectXTex-LICENSE.txt>)

## ANNiHILATION

チーム制作作品です。

### 担当箇所

UI

### 収録内容

- [担当ソースコード](<ANNiHILATION/担当ソースコード（ANNiHILATION）/ANNiHILATION_担当ソースコード>)
- [実行フォルダー](<ANNiHILATION/実行ファイル（ANNiHILATION）/実行環境データ>)

### 起動方法

実行フォルダーにある `ANNiHILATION.exe` を起動します。

チームのプロジェクトで開発する場合は、Unity 6.3（6000.3.11f1）で開きます。Visual Studioからは「Attach to Unity and Play」でUnity Editorの再生とデバッグを開始できます。

## DynamicBreaker

チーム制作作品です。

### 担当箇所

- field.cpp / field.h：フィールドの床配置・生成
- syachi.cpp / syachi.h：ギミックであるシャチの動作制御
- CRIの導入検討・実装の試行（最終的な導入には至っていません）

### 収録内容

- 担当ソースコード
- 担当箇所の説明
- [実行フォルダー](DynamicBreaker/DynamicBreaker)
- [実行動画](DynamicBreaker/%E5%AE%9F%E8%A1%8C%E5%8B%95%E7%94%BB%EF%BC%88DynamicBreaker%EF%BC%89.mp4)
- 起動案内

### 起動・選択手順

実行フォルダーにある `DynamicBreaker.exe` を起動し、以下の順に選択してください。

**PvE → SOLO → 緑のペンギン → ICE**

ICEステージで、担当したシャチのギミックをご確認いただけます。

このリポジトリには担当ソースの抜粋を収録しています。完全な制作プロジェクトでは、Visual Studio 2022で `DynamicBreaker.sln` を開き、`Debug | x64` または `Release | x64` で実行できます。

### 対応状況

- モード：PvE（SOLO / MULTI）
- キャラクター：緑のペンギン
- ステージ：ICE / SCRAP

PvP、JUNGLE、赤・紫・灰色のペンギンは未対応です。
MULTIを選択後にSOLOで遊ぶ場合は、ゲームを再起動してください。

## NodeBattle（ビジュアルプログラミング）

C++ / DirectX 11の個人作品です。ノードをつないでキャラクターの行動を組み立て、バトルで動きを確かめられます。

- [実行動画](%E3%83%93%E3%82%B8%E3%83%A5%E3%82%A2%E3%83%AB%E3%83%97%E3%83%AD%E3%82%B0%E3%83%A9%E3%83%9F%E3%83%B3%E3%82%B0/%E5%AE%9F%E8%A1%8C%E5%8B%95%E7%94%BB%EF%BC%88%E3%83%93%E3%82%B8%E3%83%A5%E3%82%A2%E3%83%AB%E3%83%97%E3%83%AD%E3%82%B0%E3%83%A9%E3%83%9F%E3%83%B3%E3%82%B0%EF%BC%89.mp4)
- [ノードの実行前チェックと実行](%E3%83%93%E3%82%B8%E3%83%A5%E3%82%A2%E3%83%AB%E3%83%97%E3%83%AD%E3%82%B0%E3%83%A9%E3%83%9F%E3%83%B3%E3%82%B0/%E3%83%97%E3%83%AD%E3%82%B8%E3%82%A7%E3%82%AF%E3%83%88%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%EF%BC%88%E3%83%93%E3%82%B8%E3%83%A5%E3%82%A2%E3%83%AB%E3%83%97%E3%83%AD%E3%82%B0%E3%83%A9%E3%83%9F%E3%83%B3%E3%82%B0%EF%BC%89/NodeBattle_Project/Source/node/runtime/nodeRuntime.cpp)：循環や接続を検証し、ノードごとの行動を順に実行します。

### 収録内容

- [Visual Studioのソリューション](<ビジュアルプログラミング/プロジェクトファイル（ビジュアルプログラミング）/NodeBattle_Project/00_GM31_初期化.sln>)
- [実行フォルダー](<ビジュアルプログラミング/実行ファイル（ビジュアルプログラミング）/NodeBattle>)
- 実行動画

### 起動とビルド

遊ぶときは、実行フォルダーにある `NodeBattle.exe` を起動します。

ソースを実行するには、下記のCRI SDKを配置し、Visual Studio 2022の「C++ によるデスクトップ開発」でソリューションを開きます。`Debug | x64` または `Release | x64` を選んでF5を押します。

ソースビルド用のSDKは、[CRI ADX LE公式ページ](https://game.criware.jp/products/adx-le/)から入手したWindows x64用SDKのヘッダーを `Library/cri/include/` に、`cri_ware_pcx64_le_import.lib` を `lib/x64/` に、`cri_ware_pcx64_le.dll` を `NodeBattle_Project/` に配置します。

## その他の作品

以下の作品の動画を収録しています。

- SPLASTER
- か蹴る
- シックスチェインバトル
- 鉄骨イスター
