#include "kosuCRI.h" 
#include <io.h>		 // _access 関数用
#include <stdio.h>	 // printf など標準 I/O

// 読み込むACBとAWBのファイル名を指定する
constexpr const char *ACF_PATH = {
	"sounds/CarGame.acf"};
constexpr const char *DPS_PATH = {
	"Mixer_0"};
constexpr const char *ACB_PATHS[SOUNDS_MAX] = {
	"sounds/BGM.acb",
	"sounds/SE.acb",
	"sounds/VOICE.acb"};
constexpr const char *AWB_PATHS[SOUNDS_MAX] = {
	"sounds/BGM.awb",
	"sounds/SE.awb",
	"sounds/VOICE.awb"};

// 静的メンバ変数の初期化
CriAtomEx3dListenerHn AudioManager::s_Listener = NULL;	  // 3Dリスナー
CriAtomExAcbHn AudioManager::sounds[SOUNDS_MAX] = {NULL}; // ACBデータ
CriAtomExPlayerHn AudioManager::s_Player = NULL;		  // プレイヤー
CriAtomExVoicePoolHn AudioManager::s_VoicePool = NULL;	  // ボイスプール

CriAtomExDbasId AudioManager::s_DbasId = CRIATOMEXDBAS_ILLEGAL_ID;

CriAtomExVector AudioManager::s_PlayerPos = {0.0f, 0.0f, 0.0f};
// 初期化関数
void AudioManager::Init()
{
	// エラー用
	criErr_SetCallback(ErrorCallback); // エラーハンドラ登録

	criAtomEx_SetUserAllocator(AllocateMemory, FreeMemory, NULL); // ユーザーアロケータ登録

	CriAtomExConfig_WASAPI config;				   // WASAPI用設定構造体
	criAtomEx_SetDefaultConfig_WASAPI(&config);	   // デフォルト値を設定
	criAtomEx_Initialize_WASAPI(&config, NULL, 0); // ライブラリ初期化

	// D-BAS 作成
	CriAtomExDbasConfig dbasConfig;
	criAtomExDbas_SetDefaultConfig(&dbasConfig);
	s_DbasId = criAtomExDbas_Create(&dbasConfig, NULL, 0);

	if (s_DbasId == CRIATOMEXDBAS_ILLEGAL_ID)
	{
		OutputDebugStringA("D-BAS 作成失敗\n");
	}
	else
	{
		OutputDebugStringA("D-BAS 作成成功\n");
	}

	// ACFファイル存在確認
	if (criAtomEx_RegisterAcfFile(NULL, ACF_PATH, NULL, 0))
	{
		OutputDebugStringA("ACF ファイルがはいった\n");
	}

	criAtomEx_AttachDspBusSetting(DPS_PATH, NULL, 0); // DSPバス設定 attach

	// ボイスプール設定
	CriAtomExStandardVoicePoolConfig vp_config;
	criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&vp_config);
	vp_config.player_config.streaming_flag = CRI_TRUE; // ストリーミング有効
	s_VoicePool = criAtomExVoicePool_AllocateStandardVoicePool(&vp_config, NULL, 0);

	// .ACB ロード
	for (int i = 0; i < SOUNDS_MAX; i++)
	{
		const char *awbPath;

		if ((_access(AWB_PATHS[i], 0) == -1))
		{
			awbPath = NULL;
		}
		else
		{
			awbPath = AWB_PATHS[i];
		}
		sounds[i] = criAtomExAcb_LoadAcbFile(NULL, ACB_PATHS[i], NULL, awbPath, NULL, 0);

		if (!sounds[i])
		{
			std::string msg = "ACB load failed at category " + std::to_string(i) + "\n";
			OutputDebugStringA(msg.c_str());
		}
		else
		{
			OutputDebugStringA("ACBがよみこまれてます\n");
			DebugCuesName(i);
		}
	}

	// 音声プレイヤーと3D音響のリスナーを生成する
	s_Player = criAtomExPlayer_Create(NULL, NULL, 0);
	s_Listener = criAtomEx3dListener_Create(NULL, NULL, 0);
}

// 終了処理
void AudioManager::Uninit()
{
	StopPlayer(true);

	if (s_Player)
	{
		criAtomExPlayer_Destroy(s_Player);
		s_Player = NULL;
	}

	for (int i = 0; i < SOUNDS_MAX; i++)
	{
		if (sounds[i])
		{
			criAtomExAcb_Release(sounds[i]);
			sounds[i] = NULL;
		}
	}

	if (s_Listener)
	{
		criAtomEx3dListener_Destroy(s_Listener);
		s_Listener = NULL;
	}
	if (s_VoicePool)
	{
		criAtomExVoicePool_Free(s_VoicePool);
		s_VoicePool = NULL;
	}

	// D-BAS 解放
	if (s_DbasId != CRIATOMEXDBAS_ILLEGAL_ID)
	{
		criAtomExDbas_Destroy(s_DbasId);
		s_DbasId = CRIATOMEXDBAS_ILLEGAL_ID;
	}

	criAtomEx_DetachDspBusSetting();
	criAtomEx_Finalize();

	// 音声ライブラリのエラー通知の登録を解除する
	criErr_SetCallback(NULL); // エラーハンドラ解除
}
// 毎フレーム更新
void AudioManager::Update()
{
	criAtomEx_ExecuteMain(); // 音声ライブラリのメイン処理を更新する
}

#pragma region Init, Uninit, Update
// 効果音再生＋ID登録
CriAtomExPlaybackId AudioManager::PlaySeStart(const SePlayParams &params, CriAtomEx3dSourceHn source)
{
#pragma region エラーログ
	if (params.categoryId < 0 || params.categoryId >= SOUNDS_MAX)
	{
		std::string msg = "カテゴリIDが不正: ";
		msg += std::to_string(params.categoryId);
		msg += "\n";
		OutputDebugStringA(msg.c_str());
		return -1;
	}

	// ACBロード済みチェック
	if (!sounds[params.categoryId])
	{
		std::string msg = "カテゴリID " + std::to_string(params.categoryId) + " の ACB が未ロードです\n";
		OutputDebugStringA(msg.c_str());
		return -1;
	}

	// Cue 名チェック
	if (!params.cueName)
	{
		OutputDebugStringA("cueName が NULL です\n");
		return -1;
	}
#pragma endregion

	// キュー名からID取得（カテゴリごとに正しい ACB を参照）
	CriAtomExCueId cueId = criAtomExAcb_GetCueIdByName(sounds[params.categoryId], params.cueName);

	if (cueId == -1)
	{
		std::string msg = "指定されたCue名が見つかりません: ";
		msg += params.cueName;
		msg += "\n";
		OutputDebugStringA(msg.c_str());

		DebugCuesName(params.categoryId);

		return -1;
	}

	criAtomExPlayer_ResetParameters(s_Player);
	criAtomExPlayer_SetCueId(s_Player, sounds[params.categoryId], cueId);
	// [設定]
	// 音量・ピッチ
	criAtomExPlayer_SetVolume(s_Player, params.volume);
	criAtomExPlayer_SetPitch(s_Player, params.pitch);

	// ループ設定
	criAtomExPlayer_LimitLoopCount(s_Player, params.loopCount);

	// 再生開始位置
	if (params.startTimeMs > 0)
	{
		criAtomExPlayer_SetStartTime(s_Player, params.startTimeMs);
	}

	// フェードイン/アウト設定
	if (params.fadeInMs > 0)
	{
		criAtomExPlayer_SetFadeInTime(s_Player, params.fadeInMs);
	}
	if (params.fadeOutMs > 0)
	{
		criAtomExPlayer_SetFadeOutTime(s_Player, params.fadeOutMs);
	}

	// 2D / 3D の切り替え
	if (params.is3D)
	{
		if (!source)
		{
			source = criAtomEx3dSource_Create(NULL, NULL, 0);
			criAtomEx3dSource_SetPosition(source, &params.pos);
			criAtomEx3dSource_Update(source);
		}
		criAtomExPlayer_Set3dSourceHn(s_Player, source);
		criAtomExPlayer_Set3dListenerHn(s_Player, s_Listener);
	}
	else
	{
		criAtomExPlayer_SetPan3dAngle(s_Player, params.pan);
	}

	// 再生
	std::string msg = "再生 ";
	msg += params.cueName;
	msg += "\n";
	OutputDebugStringA(msg.c_str());

	return criAtomExPlayer_Start(s_Player);
}

// 停止
void AudioManager::StopPlayback(CriAtomExPlaybackId id, bool immediate)
{
	if (id == -1)
		return;

	if (immediate)
	{
		criAtomExPlayback_StopWithoutReleaseTime(id);
	}
	else
	{
		criAtomExPlayback_Stop(id);
	}
}

void AudioManager::StopPlayer(bool immediate)
{
	if (!s_Player)
		return;

	if (immediate)
	{
		criAtomExPlayer_StopWithoutReleaseTime(s_Player);
	}
	else
	{
		criAtomExPlayer_Stop(s_Player);
	}
}
#pragma endregion

#pragma region 効果音再生
// 3D音源の位置を設定して反映する
void AudioManager::Update_3DSource_Position(CriAtomEx3dSourceHn source, float x, float y, float z)
{
	if (!source)
		return;

	CriAtomExVector p = {x, y, z};
	criAtomEx3dSource_SetPosition(source, &p);
	criAtomEx3dSource_Update(source);
}
#pragma endregion

#pragma region 3D音響の関数達
void AudioManager::Update_3DListener_Position(const float &x, const float &y, const float &z)
{
	s_PlayerPos.x = x;
	s_PlayerPos.y = y;
	s_PlayerPos.z = z;
	criAtomEx3dListener_SetPosition(s_Listener, &s_PlayerPos);
	criAtomEx3dListener_Update(s_Listener);
}

// 速度（ドップラー効果用）
void AudioManager::Config3DSource_Velocity(CriAtomEx3dSourceHn source, float vx, float vy, float vz)
{
	if (!source)
		return;

	CriAtomExVector v = {vx, vy, vz};
	criAtomEx3dSource_SetVelocity(source, &v);
	criAtomEx3dSource_Update(source);
}

void AudioManager::Config3DListener_Orientation(CriAtomExVector forward,
												CriAtomExVector up)
{
	criAtomEx3dListener_SetOrientation(s_Listener, &forward, &up);
	criAtomEx3dListener_Update(s_Listener);
}

// 読み込んだ音声データのキュー名を出力する
void AudioManager::DebugCuesName(int number)
{
	if (!sounds[number])
	{
		OutputDebugStringA("BGM ACB がロードされていません\n");
		return;
	}

	// numCues を取得してからループ
	CriSint32 numCues = criAtomExAcb_GetNumCues(sounds[number]);

	for (CriSint32 i = 0; i < numCues; i++)
	{
		CriAtomExCueInfo info;

		if (criAtomExAcb_GetCueInfoByIndex(sounds[number], i, &info) == CRI_TRUE)
		{
			std::string msg = "Cue[" + std::to_string(i) + "]: ";
			msg += info.name;
			msg += "\n";
			OutputDebugStringA(msg.c_str());
		}
	}
}

// エラーコールバック関数
void CRIAPI AudioManager::ErrorCallback(const CriChar8 *errid, CriUint32 p1, CriUint32 p2, CriUint32 *parray)
{
	const CriChar8 *msg = criErr_ConvertIdToMessage(errid, p1, p2); // エラーメッセージ文字列に変換
	OutputDebugStringA("ADX ERROR: ");								// デバッグ出力（前置き）
	OutputDebugStringA(msg);										// 実際のメッセージ
	OutputDebugStringA("\n");										// 改行
}
// 音声ライブラリで使うメモリを確保する
// ユーザー定義メモリアロケータ（malloc/free）
void *CRIAPI AudioManager::AllocateMemory(void *obj, CriUint32 size)
{
	return malloc(size);
}
#pragma endregion
#pragma region デバック用
void CRIAPI AudioManager::FreeMemory(void *obj, void *ptr)
{
	free(ptr);
}
#pragma endregion
