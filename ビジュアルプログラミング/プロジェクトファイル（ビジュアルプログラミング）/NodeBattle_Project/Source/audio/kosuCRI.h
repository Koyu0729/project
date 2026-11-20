#pragma once
//  3D音響CRI
//　　　　　　　　　　　　　                                         　by　小菅　友暉

#include <cri_le_atom_ex.h>
#include <cri_le_atom_asr.h>
#include <cri_le_atom_wasapi.h>
#include <cri_le_error.h>
#include <windows.h>
#include <string>

// 読み込む音声データの分類
// BGM・SE・VOICEのACBを分類して保持する
//BGM    ２D用
//SE　　 ３D用
// VOICE用の音声データ
enum Sounds {
    SOUNDS_NULL = -1,

    BGM,
    SE,
    VOICE,

    SOUNDS_MAX
};

// 音声の再生開始時に使用する設定
struct SePlayParams {
    const char* cueName = nullptr;             // 再生するCue名
    float            volume = 1.0f;            // 音量
    float            pitch = 0.0f;             // ピッチ
    int              loopCount = 1;            // CRIATOMPLAYER_FORCE_LOOP / NO_LOOP_LIMITATION / 任意回数
    CriSint64        startTimeMs = 0;          // 再生開始位置(ms)
    Sounds           categoryId = SOUNDS_NULL; // 参照するACBカテゴリ
    int              priority = 0;             // 再生優先度の設定値（未指定時は0）
    int              fadeInMs = 0;             // 再生開始時のフェードイン時間
    int              fadeOutMs = 0; // 再生終了時のフェードアウト時間            
    float            pan = 0.0f;               // 2Dパン(-1.0～+1.0)

    // 3D再生を開始するかどうか（初期位置は pos を使用）
    bool             is3D = false;
    CriAtomExVector  pos = { 0.0f, 0.0f, 0.0f };
};

class AudioManager {
public:
    // 音声の初期化・終了処理と、毎フレームの更新
    static void Init();
    static void Uninit();
    static void Update();

    // ゲームオブジェクト関連

    //再生（Start）
    static CriAtomExPlaybackId PlaySeStart(const SePlayParams& params, CriAtomEx3dSourceHn source = nullptr);

    static void StopPlayback(CriAtomExPlaybackId id, bool immediate = false);
    // 音声プレイヤーで再生している音を停止する
    static void StopPlayer(bool immediate = false);

   //位置更新（SetPosition → Update を内包）
   static void Update_3DSource_Position(CriAtomEx3dSourceHn source, float x, float y, float z);

   // 更新
#pragma region 3D使うなら確定
   // 3D音響のリスナー位置を更新する
   static void Update_3DListener_Position(const float& x, const float& y, const float& z);

   // 音源の速度を設定し、ドップラー効果の計算に使う
        static void Config3DSource_Velocity(CriAtomEx3dSourceHn source, float vx, float vy, float vz);
#pragma endregion

#pragma region 3Dのちょっとした工夫用
    // リスナーの前方向と上方向を設定する
    static void Config3DListener_Orientation(CriAtomExVector forward, CriAtomExVector up);

    static void DebugCuesName(int number);
#pragma endregion

private:
    // 音声ライブラリのエラー通知と、メモリ確保・解放のコールバック
    static void CRIAPI ErrorCallback(const CriChar8 *errid, CriUint32 p1, CriUint32 p2, CriUint32 *parray);
    static void *CRIAPI AllocateMemory(void *obj, CriUint32 size);
    static void CRIAPI FreeMemory(void *obj, void *ptr);

    static CriAtomEx3dListenerHn s_Listener;
    static CriAtomExAcbHn sounds[SOUNDS_MAX];
    static CriAtomExPlayerHn s_Player;
    static CriAtomExVoicePoolHn s_VoicePool;

    static CriAtomExDbasId s_DbasId;

    static CriAtomExVector s_PlayerPos;
};

