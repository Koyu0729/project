#pragma once

#include <cri_le_atom_ex.h>
#include <cri_le_atom_asr.h>
#include <cri_le_atom_wasapi.h>
#include <cri_le_error.h>
#include <windows.h>
#include <string>

// acbの数
// サンプルの音声(内容は下記の通り)
// BGM    2D用
// SE     3D用
// VOICE  ボイス用
enum Sounds
{
    SOUNDS_NULL = -1,

    BGM,
    SE,
    VOICE,

    SOUNDS_MAX
};

// 音声再生時の設定
struct SePlayParams
{
    const char* cueName = nullptr;             // 再生するCue名
    float            volume = 1.0f;            // 個別音量
    float            pitch = 0.0f;             // ピッチ
    int              loopCount = 0;            // CRIATOMPLAYER_FORCE_LOOP / NO_LOOP_LIMITATION / 任意回数
    CriSint64        startTimeMs = 0;          // 再生開始位置(ms)
    Sounds           categoryId = SOUNDS_NULL; // 参照するACBカテゴリ
    int              priority = 0;             // 再生優先度
    int              fadeInMs = 0;             // フェードイン時間(ms)
    int              fadeOutMs = 0;            // フェードアウト時間(ms)
    float            pan = 0.0f;               // 2Dパン(-1.0～+1.0)

    // 3D再生を開始するかどうか（初期位置は pos を使用）
    bool             is3D = false;
    CriAtomExVector  pos = { 0.0f, 0.0f, 0.0f };
};

class AudioManager
{
public:
    // ゲームループ内で音声処理を更新する
    static void Init();
    static void Update();
    static void Uninit();

    // 登録済みの音声キュー名を確認する
    static void DebugCuesName(int number);


    // 疑似マスター音量。
    // 実際には BGM / SE / VOICE の再生時音量に掛ける。
    static void SetMasterVolume(float volume);

    // BGM全体倍率
    static void SetBgmVolume(float volume);

    // SE全体倍率
    static void SetSeVolume(float volume);

    // VOICE全体倍率
    static void SetVoiceVolume(float volume);

    static float GetMasterVolume();
    static float GetBgmVolume();
    static float GetSeVolume();
    static float GetVoiceVolume();

    // categoryId に応じた最終カテゴリ倍率を返す。
    // 例：SEなら Master * SE
    static float GetCategoryVolumeRate(Sounds categoryId);

    static void ApplyVolumeToPlayer(
        CriAtomExPlayerHn player,
        Sounds categoryId,
        float baseVolume
    );

    // プレイヤー作成
    static CriAtomExPlayerHn CreatePlayer();

    // プレイヤー破棄
    static void DestroyPlayer(CriAtomExPlayerHn player);

    // プレイヤー単位の停止
    static void StopPlayer(CriAtomExPlayerHn player, bool immediate = false);


    // 再生
    static CriAtomExPlaybackId PlaySeStart(
        CriAtomExPlayerHn player,
        const SePlayParams& params,
        CriAtomEx3dSourceHn source = nullptr
    );

    // 再生停止（ID）
    static void StopPlayback(CriAtomExPlaybackId id, bool immediate = false);


    // 音源の位置を更新する
    static void Update_3DSource_Position(
        CriAtomEx3dSourceHn source,
        float x,
        float y,
        float z
    );

    // 音声を聞く位置を更新する
    static void Update_3DListener_Position(
        const float& x,
        const float& y,
        const float& z
    );
private:
    // 内部処理用
    static CriAtomEx3dListenerHn s_Listener;
    static CriAtomExAcbHn sounds[SOUNDS_MAX];
    static CriAtomExVoicePoolHn s_VoicePool;

    static CriAtomExDbasId s_DbasId;

    static CriAtomExVector s_PlayerPos;


    static float s_MasterVolume;
    static float s_BgmVolume;
    static float s_SeVolume;
    static float s_VoiceVolume;

    static float ClampVolume(float volume);

};