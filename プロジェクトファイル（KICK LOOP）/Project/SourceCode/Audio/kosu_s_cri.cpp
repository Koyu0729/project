#include "kosu_s_cri.h"          // 自作ヘッダのインクルード
#include <io.h>                  // _access 関数用
#include <stdio.h>               // printf など標準 I/O

// 音声データのパス
constexpr const char* ACF_PATH = {
    "sounds/CarGame.acf"
};

constexpr const char* DPS_PATH = {
    "Mixer_0"
};

constexpr const char* ACB_PATHS[SOUNDS_MAX] = {
    "sounds/BGM.acb",
    "sounds/SE.acb",
    "sounds/VOICE.acb"
};

constexpr const char* AWB_PATHS[SOUNDS_MAX] = {
    "sounds/BGM.awb",
    "sounds/SE.awb",
    "sounds/VOICE.awb"
};

// 静的メンバ変数の初期化
CriAtomEx3dListenerHn AudioManager::s_Listener = NULL;
CriAtomExAcbHn AudioManager::sounds[SOUNDS_MAX] = { NULL };
CriAtomExVoicePoolHn AudioManager::s_VoicePool = NULL;

CriAtomExDbasId AudioManager::s_DbasId = CRIATOMEXDBAS_ILLEGAL_ID;

CriAtomExVector AudioManager::s_PlayerPos = { 0.0f, 0.0f, 0.0f };

// 音量設定
// 最終音量 = 個別 volume * Master倍率 * BGM/SE/VOICE倍率
// ただし、最終音量の最大は「個別 volume の10倍」まで。
// 例：
// params.volume = 5.0f なら最大 50.0f
// params.volume = 1.0f なら最大 10.0f
float AudioManager::s_MasterVolume = 1.0f;
float AudioManager::s_BgmVolume = 1.0f;
float AudioManager::s_SeVolume = 1.0f;
float AudioManager::s_VoiceVolume = 1.0f;

// エラーコールバック関数
static void CRIAPI ErrorCallback(
    const CriChar8* errid,
    CriUint32 p1,
    CriUint32 p2,
    CriUint32* parray
)
{
    const CriChar8* msg =
        criErr_ConvertIdToMessage(
            errid,
            p1,
            p2
        );

    OutputDebugStringA("ADX ERROR: ");
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");
}

// ユーザー定義メモリアロケータ（malloc/free）
static void* CRIAPI user_malloc(
    void* obj,
    CriUint32 size
)
{
    (void)obj;

    return malloc(
        size
    );
}

static void CRIAPI user_free(
    void* obj,
    void* ptr
)
{
    (void)obj;

    free(
        ptr
    );
}

float AudioManager::ClampVolume(float volume)
{
    if (volume < 0.0f)
    {
        return 0.0f;
    }

    // Option用の倍率として10倍まで許可。
    if (volume > 10.0f)
    {
        return 10.0f;
    }

    return volume;
}

void AudioManager::SetMasterVolume(float volume)
{
    s_MasterVolume =
        ClampVolume(
            volume
        );
}

void AudioManager::SetBgmVolume(float volume)
{
    s_BgmVolume =
        ClampVolume(
            volume
        );
}

void AudioManager::SetSeVolume(float volume)
{
    s_SeVolume =
        ClampVolume(
            volume
        );
}

void AudioManager::SetVoiceVolume(float volume)
{
    s_VoiceVolume =
        ClampVolume(
            volume
        );
}

float AudioManager::GetMasterVolume()
{
    return s_MasterVolume;
}

float AudioManager::GetBgmVolume()
{
    return s_BgmVolume;
}

float AudioManager::GetSeVolume()
{
    return s_SeVolume;
}

float AudioManager::GetVoiceVolume()
{
    return s_VoiceVolume;
}

float AudioManager::GetCategoryVolumeRate(Sounds categoryId)
{
    switch (categoryId)
    {
    case BGM:
        return
            s_MasterVolume *
            s_BgmVolume;

    case SE:
        return
            s_MasterVolume *
            s_SeVolume;

    case VOICE:
        return
            s_MasterVolume *
            s_VoiceVolume;

    default:
        return s_MasterVolume;
    }
}

#pragma region Init, Uninit, Update

void AudioManager::Init()
{
    // エラー用
    criErr_SetCallback(
        ErrorCallback
    );

    criAtomEx_SetUserAllocator(
        user_malloc,
        user_free,
        NULL
    );

    CriAtomExConfig_WASAPI config;

    criAtomEx_SetDefaultConfig_WASAPI(
        &config
    );

    criAtomEx_Initialize_WASAPI(
        &config,
        NULL,
        0
    );

    // --- D-BAS 作成 ---
    CriAtomExDbasConfig dbasConfig;

    criAtomExDbas_SetDefaultConfig(
        &dbasConfig
    );

    s_DbasId =
        criAtomExDbas_Create(
            &dbasConfig,
            NULL,
            0
        );

    if (s_DbasId == CRIATOMEXDBAS_ILLEGAL_ID)
    {
        OutputDebugStringA("D-BAS 作成失敗\n");
    }
    else
    {
        OutputDebugStringA("D-BAS 作成成功\n");
    }

    // ACFファイル存在確認
    if (criAtomEx_RegisterAcfFile(
        NULL,
        ACF_PATH,
        NULL,
        0
    ))
    {
        OutputDebugStringA("ACF ファイルがはいった\n");
    }

    criAtomEx_AttachDspBusSetting(
        DPS_PATH,
        NULL,
        0
    );

    // ボイスプール設定
    CriAtomExStandardVoicePoolConfig vp_config;

    criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(
        &vp_config
    );

    vp_config.num_voices = 32;

    vp_config.player_config.streaming_flag = CRI_TRUE;

    s_VoicePool =
        criAtomExVoicePool_AllocateStandardVoicePool(
            &vp_config,
            NULL,
            0
        );

    // .ACB ロード
    for (int i = 0; i < SOUNDS_MAX; i++)
    {
        const char* awbPath =
            (_access(AWB_PATHS[i], 0) == -1)
            ? NULL
            : AWB_PATHS[i];

        sounds[i] =
            criAtomExAcb_LoadAcbFile(
                NULL,
                ACB_PATHS[i],
                NULL,
                awbPath,
                NULL,
                0
            );

        if (!sounds[i])
        {
            std::string msg =
                "ACB load failed at category " +
                std::to_string(i) +
                "\n";

            OutputDebugStringA(
                msg.c_str()
            );
        }
        else
        {
            OutputDebugStringA("ACBがよみこまれてます\n");

            DebugCuesName(
                i
            );
        }
    }

    // Listener
    s_Listener =
        criAtomEx3dListener_Create(
            NULL,
            NULL,
            0
        );
}

void AudioManager::Update()
{
    criAtomEx_ExecuteMain();
}

void AudioManager::Uninit()
{
    // ACB解放
    for (int i = 0; i < SOUNDS_MAX; i++)
    {
        if (sounds[i])
        {
            criAtomExAcb_Release(
                sounds[i]
            );

            sounds[i] = NULL;
        }
    }

    if (s_Listener)
    {
        criAtomEx3dListener_Destroy(
            s_Listener
        );

        s_Listener = NULL;
    }

    if (s_VoicePool)
    {
        criAtomExVoicePool_Free(
            s_VoicePool
        );

        s_VoicePool = NULL;
    }

    if (s_DbasId != CRIATOMEXDBAS_ILLEGAL_ID)
    {
        criAtomExDbas_Destroy(
            s_DbasId
        );

        s_DbasId = CRIATOMEXDBAS_ILLEGAL_ID;
    }

    criAtomEx_Finalize_WASAPI();
}

#pragma endregion

#pragma region Create,Stop,Destroy

// 音声再生ハンドルを作る
CriAtomExPlayerHn AudioManager::CreatePlayer()
{
    return criAtomExPlayer_Create(
        NULL,
        NULL,
        0
    );
}

// 音声再生ハンドルを解放する
void AudioManager::DestroyPlayer(CriAtomExPlayerHn player)
{
    if (!player)
    {
        return;
    }

    criAtomExPlayer_Destroy(
        player
    );
}

// 指定ハンドルの音声を止める
void AudioManager::StopPlayer(
    CriAtomExPlayerHn player,
    bool immediate
)
{
    if (!player)
    {
        return;
    }

    if (immediate)
    {
        criAtomExPlayer_StopWithoutReleaseTime(
            player
        );
    }
    else
    {
        criAtomExPlayer_Stop(
            player
        );
    }
}

#pragma endregion

CriAtomExPlaybackId AudioManager::PlaySeStart(
    CriAtomExPlayerHn player,
    const SePlayParams& params,
    CriAtomEx3dSourceHn source
)
{
#pragma region エラーログ

    if (player == nullptr)
    {
        OutputDebugStringA("player が NULL です\n");
        return -1;
    }

    if (params.categoryId < 0 || params.categoryId >= SOUNDS_MAX)
    {
        std::string msg =
            "カテゴリIDが不正: " +
            std::to_string(params.categoryId) +
            "\n";

        OutputDebugStringA(
            msg.c_str()
        );

        return -1;
    }

    if (!sounds[params.categoryId])
    {
        std::string msg =
            "カテゴリID " +
            std::to_string(params.categoryId) +
            " の ACB が未ロードです\n";

        OutputDebugStringA(
            msg.c_str()
        );

        return -1;
    }

    if (!params.cueName)
    {
        OutputDebugStringA("cueName が NULL です\n");
        return -1;
    }

    CriAtomExCueId cueId =
        criAtomExAcb_GetCueIdByName(
            sounds[params.categoryId],
            params.cueName
        );

    if (cueId == -1)
    {
        std::string msg =
            "指定されたCue名が見つかりません: ";

        msg += params.cueName;
        msg += "\n";

        OutputDebugStringA(
            msg.c_str()
        );

        DebugCuesName(
            params.categoryId
        );

        return -1;
    }

#pragma endregion

    criAtomExPlayer_ResetParameters(
        player
    );

    criAtomExPlayer_SetCueId(
        player,
        sounds[params.categoryId],
        cueId
    );


    ApplyVolumeToPlayer(
        player,
        params.categoryId,
        params.volume
    );

    criAtomExPlayer_SetPitch(
        player,
        params.pitch
    );

    criAtomExPlayer_LimitLoopCount(
        player,
        params.loopCount
    );

    if (params.startTimeMs > 0)
    {
        criAtomExPlayer_SetStartTime(
            player,
            params.startTimeMs
        );
    }

    if (params.fadeInMs > 0)
    {
        criAtomExPlayer_SetFadeInTime(
            player,
            params.fadeInMs
        );
    }

    if (params.fadeOutMs > 0)
    {
        criAtomExPlayer_SetFadeOutTime(
            player,
            params.fadeOutMs
        );
    }

    // --- 2D / 3D の切り替え ---
    if (params.is3D)
    {
        if (!source)
        {
            source =
                criAtomEx3dSource_Create(
                    NULL,
                    NULL,
                    0
                );

            criAtomEx3dSource_SetPosition(
                source,
                &params.pos
            );

            criAtomEx3dSource_Update(
                source
            );
        }

        criAtomExPlayer_Set3dSourceHn(
            player,
            source
        );

        criAtomExPlayer_Set3dListenerHn(
            player,
            s_Listener
        );
    }
    else
    {
        criAtomExPlayer_SetPan3dAngle(
            player,
            params.pan
        );
    }

    std::string msg =
        "再生 ";

    msg += params.cueName;
    msg += " volume:";
    msg += std::to_string(params.volume);
    msg += " categoryRate:";
    msg += std::to_string(GetCategoryVolumeRate(params.categoryId));
    msg += "\n";

    OutputDebugStringA(
        msg.c_str()
    );

    return criAtomExPlayer_Start(
        player
    );
}

void AudioManager::StopPlayback(
    CriAtomExPlaybackId id,
    bool immediate
)
{
    if (id == -1)
    {
        return;
    }

    if (immediate)
    {
        criAtomExPlayback_StopWithoutReleaseTime(
            id
        );
    }
    else
    {
        criAtomExPlayback_Stop(
            id
        );
    }
}

void AudioManager::Update_3DSource_Position(
    CriAtomEx3dSourceHn source,
    float x,
    float y,
    float z
)
{
    if (!source)
    {
        return;
    }

    CriAtomExVector p =
    {
        x,
        y,
        z
    };

    criAtomEx3dSource_SetPosition(
        source,
        &p
    );

    criAtomEx3dSource_Update(
        source
    );
}

void AudioManager::Update_3DListener_Position(
    const float& x,
    const float& y,
    const float& z
)
{
    s_PlayerPos =
    {
        x,
        y,
        z
    };

    criAtomEx3dListener_SetPosition(
        s_Listener,
        &s_PlayerPos
    );

    criAtomEx3dListener_Update(
        s_Listener
    );
}

#pragma region デバック用

// 音声キューの名前をデバッグ出力する
void AudioManager::DebugCuesName(int number)
{
    if (number < 0 || number >= SOUNDS_MAX)
    {
        OutputDebugStringA("不正なカテゴリ番号です\n");
        return;
    }

    if (!sounds[number])
    {
        OutputDebugStringA("ACB がロードされていません\n");
        return;
    }

    CriSint32 numCues =
        criAtomExAcb_GetNumCues(
            sounds[number]
        );

    for (CriSint32 i = 0; i < numCues; i++)
    {
        CriAtomExCueInfo info;

        if (criAtomExAcb_GetCueInfoByIndex(
            sounds[number],
            i,
            &info
        ) == CRI_TRUE)
        {
            std::string msg =
                "Cue[" +
                std::to_string(i) +
                "]: ";

            msg += info.name;
            msg += "\n";

            OutputDebugStringA(
                msg.c_str()
            );
        }
    }
}

// 再生中の音声に設定音量を反映する
void AudioManager::ApplyVolumeToPlayer(
    CriAtomExPlayerHn player,
    Sounds categoryId,
    float baseVolume
)
{
    if (!player)
    {
        return;
    }

    if (baseVolume < 0.0f)
    {
        baseVolume = 0.0f;
    }

    float volumeRate =
        GetCategoryVolumeRate(
            categoryId
        );

    float finalVolume =
        baseVolume *
        volumeRate;

    float maxFinalVolume =
        baseVolume *
        10.0f;

    if (finalVolume < 0.0f)
    {
        finalVolume = 0.0f;
    }

    if (finalVolume > maxFinalVolume)
    {
        finalVolume =
            maxFinalVolume;
    }

    criAtomExPlayer_SetVolume(
        player,
        finalVolume
    );

    criAtomExPlayer_UpdateAll(
        player
    );
}

#pragma endregion