# ゲーム制作ポートフォリオ

KICK LOOPではアクションゲームの操作を、NodeBattleではノードエディターと実行の仕組みを制作しました。

[ポートフォリオ（PDF）](<ポートフォリオ/小菅友暉_ゲーム制作ポートフォリオ.pdf>)　｜　[KICK LOOPのプレイ動画](<KickLoop/実行動画（KICK LOOP）（完成版）.mp4>)　｜　[NodeBattleのプレイ動画](<ビジュアルプログラミング/実行動画（ビジュアルプログラミング）.mp4>)

## ダウンロードして遊ぶ

1. [ZIPをダウンロード](https://github.com/Koyu0729/project/archive/refs/heads/main.zip)し、すべて展開します。
2. 展開先で遊びたい作品の実行フォルダーを開き、表にあるexeを起動します。

| 作品 | 実行フォルダー | 起動するファイル |
| --- | --- | --- |
| KICK LOOP | [実行ファイル](<KickLoop/実行ファイル（KICK LOOP）/実行ファイル>) | `KICKLOOP.exe` |
| NodeBattle | [NodeBattle](<ビジュアルプログラミング/実行ファイル（ビジュアルプログラミング）/NodeBattle>) | `NodeBattle.exe` |
| ANNiHILATION | [実行環境データ](<ANNiHILATION/実行ファイル（ANNiHILATION）/実行環境データ>) | `ANNiHILATION.exe` |
| DynamicBreaker | [DynamicBreaker](DynamicBreaker/DynamicBreaker) | `DynamicBreaker.exe` |

## 作品一覧

- [KICK LOOP](#kick-loop)
- [NodeBattle（ビジュアルプログラミング）](#nodebattleビジュアルプログラミング)
- [ANNiHILATION](#annihilation)
- [DynamicBreaker](#dynamicbreaker)
- [その他の作品](#その他の作品)

## KICK LOOP

C++ / DirectX 11で制作した個人作品です。壁キックや急降下をつないで、制限時間内にゴールを目指します。

- [アクションの判定・切り替え](KickLoop/%E3%83%97%E3%83%AD%E3%82%B8%E3%82%A7%E3%82%AF%E3%83%88%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%EF%BC%88KICK%20LOOP%EF%BC%89/Project/SourceCode/Player/ActionResolution/playerActionResolver.cpp)：操作要求を受け取り、プレイヤーの状況に応じて行動を決める処理です。
- [GPUパーティクル](KickLoop/%E3%83%97%E3%83%AD%E3%82%B8%E3%82%A7%E3%82%AF%E3%83%88%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%EF%BC%88KICK%20LOOP%EF%BC%89/Project/shader/gpuParticleCS.hlsl)：粒子の位置や寿命を更新するコンピュートシェーダーです。

### 収録内容

- [Visual Studioのソリューション](<KickLoop/プロジェクトファイル（KICK LOOP）/Project/00_GM31_初期化.sln>)
- [実行フォルダー](<KickLoop/実行ファイル（KICK LOOP）/実行ファイル>)
- [プロトタイプのプレイ動画](<KickLoop/実行動画（KICK LOOP）（プロトタイプ）.mp4>)

### 遊び方

[実行フォルダー](<KickLoop/実行ファイル（KICK LOOP）/実行ファイル>)内の `KICKLOOP.exe` を起動してください。

タイトルで「GAME START」に視点を合わせ、ボタンが水色に変わったら左クリックでスタートします。

- `W` `A` `S` `D` で移動します。
- `Space` でジャンプします。空中で蹴れる壁に触れたときにもう一度 `Space` を押すと、壁を蹴って跳べます。
- 空中で `C` を押すと、少し浮き上がってから急降下します。

操作を練習するときは「TUTORIAL」を選びます。全8部屋で、各部屋の課題をクリアしてEnterを押すと次に進みます。最後の部屋ではEnterでタイトルに戻ります。

ゴール後は約5秒でカメラが自動で動き始めます。

### 使用ライブラリ

このソフトウェアには、（株）ＣＲＩ・ミドルウェアの「CRIWARE」が使用されています。 [CRI ADX LE 使用許諾文](<KickLoop/実行ファイル（KICK LOOP）/実行ファイル/CRI_ADX_LE_End_User_License_Agreement.txt>)

モデルとアニメーションの読み込みに Assimp 5.2.4 を使用しています。[Assimp のライセンス](<KickLoop/実行ファイル（KICK LOOP）/実行ファイル/Assimp-LICENSE.txt>)

テクスチャの読み込みにはDirectXTexを使用しています。 [DirectXTexのライセンス](<KickLoop/プロジェクトファイル（KICK LOOP）/Project/Libraries/DirectXTex/DirectXTex-LICENSE.txt>)

ステージデータの読み込みにはnlohmann/jsonを使用しています。[nlohmann/jsonのライセンス](<KickLoop/実行ファイル（KICK LOOP）/実行ファイル/nlohmann-json-LICENSE.txt>)

## NodeBattle（ビジュアルプログラミング）

C++ / DirectX 11の個人作品です。ノードをつないでキャラクターの行動を組み立て、バトルで動きを確かめられます。

- [ノードの実行前チェックと実行](%E3%83%93%E3%82%B8%E3%83%A5%E3%82%A2%E3%83%AB%E3%83%97%E3%83%AD%E3%82%B0%E3%83%A9%E3%83%9F%E3%83%B3%E3%82%B0/%E3%83%97%E3%83%AD%E3%82%B8%E3%82%A7%E3%82%AF%E3%83%88%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%EF%BC%88%E3%83%93%E3%82%B8%E3%83%A5%E3%82%A2%E3%83%AB%E3%83%97%E3%83%AD%E3%82%B0%E3%83%A9%E3%83%9F%E3%83%B3%E3%82%B0%EF%BC%89/NodeBattle_Project/Source/node/runtime/nodeRuntime.cpp)：循環や接続を検証し、ノードごとの行動を順に実行します。

### 収録内容

- [Visual Studioのソリューション](<ビジュアルプログラミング/プロジェクトファイル（ビジュアルプログラミング）/NodeBattle_Project/00_GM31_初期化.sln>)
- [実行フォルダー](<ビジュアルプログラミング/実行ファイル（ビジュアルプログラミング）/NodeBattle>)

<a name="起動とビルド"></a>

### 起動方法

遊ぶときは、実行フォルダーにある `NodeBattle.exe` を起動します。

### 使用ライブラリ

ノードの接続や設定の保存・読み込みにはnlohmann/jsonを使用しています。[nlohmann/jsonのライセンス](<ビジュアルプログラミング/実行ファイル（ビジュアルプログラミング）/NodeBattle/licenses/nlohmann-json-LICENSE.txt>)

## ANNiHILATION

チーム制作作品です。

### 担当箇所

UIとポストエフェクトを担当しました。

- 目標が画面の外にあるとき、画面端にその方向を示すナビを表示します。[UI_GoalNavi.cs](<ANNiHILATION/担当ソースコード（ANNiHILATION）/ANNiHILATION_担当ソースコード/UI_GoalNavi.cs>)
- 被弾したとき、画面にノイズを重ねてダメージを伝えます。[DamagePostEffectController.cs](<ANNiHILATION/担当ソースコード（ANNiHILATION）/ANNiHILATION_担当ソースコード/DamagePostEffectController.cs>)

### 収録内容

- [担当ソースコード](<ANNiHILATION/担当ソースコード（ANNiHILATION）/ANNiHILATION_担当ソースコード>)
- [実行フォルダー](<ANNiHILATION/実行ファイル（ANNiHILATION）/実行環境データ>)

### 起動方法

ZIPをすべて展開し、「実行環境データ」フォルダー内の `ANNiHILATION.exe` を起動してください。

## DynamicBreaker

チーム制作作品です。

### 担当箇所

- フィールドに床ブロックを並べる処理を実装しました。[field.cpp](<DynamicBreaker/実装コード（DynamicBreaker）/field.cpp>) / [field.h](<DynamicBreaker/実装コード（DynamicBreaker）/field.h>)
- ICEステージでシャチが跳び上がり、近くの床ブロックを壊すギミックを実装しました。[syachi.cpp](<DynamicBreaker/実装コード（DynamicBreaker）/syachi.cpp>) / [syachi.h](<DynamicBreaker/実装コード（DynamicBreaker）/syachi.h>)

### 収録内容

- [担当ソースコード](<DynamicBreaker/実装コード（DynamicBreaker）>)
- [担当箇所の説明](<DynamicBreaker/実装コード（DynamicBreaker）/担当箇所.txt>)
- [実行フォルダー](DynamicBreaker/DynamicBreaker)
- [実行動画](DynamicBreaker/%E5%AE%9F%E8%A1%8C%E5%8B%95%E7%94%BB%EF%BC%88DynamicBreaker%EF%BC%89.mp4)
- [起動案内](<DynamicBreaker/ReadMe（DynamicBreaker）.txt>)

### 起動・選択手順

実行フォルダーにある `DynamicBreaker.exe` を起動し、以下の順に選択してください。

**PvE → SOLO → 緑のペンギン → ICE**

ICEステージで、担当したシャチのギミックをご確認いただけます。

### 対応状況

- モード：PvE（SOLO / MULTI）
- キャラクター：緑のペンギン
- ステージ：ICE / SCRAP

PvP、JUNGLE、赤・紫・灰色のペンギンは未対応です。
MULTIを選択後にSOLOで遊ぶ場合は、ゲームを再起動してください。

## その他の作品

以下の作品の動画を収録しています。

- [SPLASTER](<other/SPLASTER.mp4>)
- [か蹴る](<other/1DayTeam/か蹴る.mp4>)
- [シックスチェインバトル](<other/1DayTeam/シックスチェインバトル.mp4>)
- [鉄骨イスター](<other/1DayTeam/鉄骨イスター.mp4>)
