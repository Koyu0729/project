// 【ファイルの役割】被ダメージ・死亡イベントからノイズ強度とマスクの時間変化を制御する。
// 【依存・注意点】被ダメージは減衰、死亡は増強する。unscaledDeltaTimeを使用。値はstaticなので複数の制御インスタンスでは競合し得る。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using System.Collections;
using UnityEngine;

/// <summary>
/// ダメージ / 死亡時の PE_Noize 制御。
/// RendererFeature は CurrentIntensity / CurrentMaskStrength を読んで描画直前に反映する。
/// </summary>
public class DamagePostEffectController : MonoBehaviour
{
    /// <summary>
    /// RendererFeature 側がこの値を見て、PE_Noize の上書きを行うか判断する。
    /// </summary>
    public static bool IsControlEnabled { get; private set; } = false;

    /// <summary>
    /// RendererFeature が _Intensity に渡す値。
    /// </summary>
    public static float CurrentIntensity { get; private set; } = 0.0f;

    /// <summary>
    /// RendererFeature が _MaskStrength に渡す値。
    /// </summary>
    public static float CurrentMaskStrength { get; private set; } = 0.0f;

    /// <summary>
    /// 死亡ポストエフェクトが最後まで描画された後に呼ばれる。
    /// シーン遷移側はこのイベントを受けてから LoadScene する。
    /// </summary>
    public static event System.Action OnDeathNoizeFinished;

    /// <summary>
    /// ダメージポストエフェクト終了時に呼ばれる。
    /// </summary>
    public static event System.Action OnDamageNoizeFinished;

    [Header("Target")]
    [SerializeField] private CharacterData m_target;
    [SerializeField] private string m_playerTag = "Player";

    [Header("Control")]
    [SerializeField] private bool m_enableControl = true;

    [Header("Manual Test")]
    [SerializeField] private bool m_useManualControl = false;
    [SerializeField] private float m_manualIntensity = 0.0f;
    [SerializeField] private float m_manualMaskStrength = 0.0f;

    [Header("Initial")]
    [SerializeField] private float m_initialIntensity = 0.0f;
    [SerializeField] private float m_initialMaskStrength = 0.0f;

    [Header("Damage")]
    [SerializeField] private bool m_useDamageNoize = true;
    [SerializeField] private float m_damageStartIntensity = 1.0f;
    [SerializeField] private float m_damageEndIntensity = 0.0f;
    [SerializeField] private float m_damageStartMaskStrength = 1.0f;
    [SerializeField] private float m_damageEndMaskStrength = 0.0f;
    [SerializeField] private float m_damageFadeTime = 0.35f;

    [Header("Death")]
    [SerializeField] private bool m_useDeathNoize = true;
    [SerializeField] private float m_deathStartIntensity = 0.0f;
    [SerializeField] private float m_deathEndIntensity = 1.0f;
    [SerializeField] private float m_deathMaskStrength = 0.0f;
    [SerializeField] private float m_deathFadeTime = 1.0f;

    [Header("Death Finish")]
    [SerializeField] private bool m_disableControlWhenDeathFinished = false;
    [SerializeField] private bool m_waitOneFrameBeforeDeathFinishedEvent = true;

    [Header("Debug")]
    [SerializeField, ReadOnly] private float m_debugCurrentIntensity = 0.0f;
    [SerializeField, ReadOnly] private float m_debugCurrentMaskStrength = 0.0f;
    [SerializeField, ReadOnly] private bool m_isDeath = false;
    [SerializeField, ReadOnly] private bool m_isDeathNoizeFinished = false;
    [SerializeField, ReadOnly] private int m_damageEventCount = 0;

    private Coroutine m_damageCoroutine;
    private Coroutine m_deathCoroutine;

    private void Awake()
    {
        FindTargetIfNeeded();

        ApplyControlEnabled();
        SetCurrentValue(m_initialIntensity, m_initialMaskStrength);
        UpdateDebugValue();
    }

    private void OnEnable()
    {
        ApplyControlEnabled();
        RegisterEvent();
    }

    private void OnDisable()
    {
        UnregisterEvent();
    }

    private void OnDestroy()
    {
        UnregisterEvent();

        if (IsControlEnabled)
        {
            SetCurrentValue(0.0f, 0.0f);
            IsControlEnabled = false;
        }
    }

    private void Update()
    {
        ApplyControlEnabled();

        if (m_useManualControl)
        {
            SetCurrentValue(m_manualIntensity, m_manualMaskStrength);
        }

        UpdateDebugValue();
    }

    private void ApplyControlEnabled()
    {
        if (m_disableControlWhenDeathFinished && m_isDeathNoizeFinished)
        {
            IsControlEnabled = false;
            return;
        }

        IsControlEnabled = m_enableControl;
    }

    private void FindTargetIfNeeded()
    {
        if (m_target != null)
        {
            return;
        }

        GameObject player = GameObject.FindGameObjectWithTag(m_playerTag);

        if (player == null)
        {
            Debug.LogWarning("DamagePostEffectController : Playerタグのオブジェクトが見つかりません");
            return;
        }

        m_target = player.GetComponent<CharacterData>();

        if (m_target == null)
        {
            Debug.LogWarning("DamagePostEffectController : CharacterData が見つかりません");
        }
    }

    private void RegisterEvent()
    {
        if (m_target == null)
        {
            return;
        }

        m_target.OnTakeDamage -= PlayDamageNoize;
        m_target.OnTakeDamage += PlayDamageNoize;

        m_target.OnDeath -= PlayDeathNoize;
        m_target.OnDeath += PlayDeathNoize;
    }

    private void UnregisterEvent()
    {
        if (m_target == null)
        {
            return;
        }

        m_target.OnTakeDamage -= PlayDamageNoize;
        m_target.OnDeath -= PlayDeathNoize;
    }

    private void SetCurrentValue(float intensity, float maskStrength)
    {
        CurrentIntensity = Mathf.Clamp01(intensity);
        CurrentMaskStrength = Mathf.Clamp01(maskStrength);
    }

    /// <summary>
    /// CharacterData.OnTakeDamage は GameObject, int を渡してくる。
    /// ダメージ量による強弱は付けず、受けたら一律演出。
    /// </summary>
    private void PlayDamageNoize(GameObject owner, int damageValue)
    {
        if (m_useManualControl)
        {
            return;
        }

        if (!m_enableControl)
        {
            return;
        }

        if (!m_useDamageNoize)
        {
            return;
        }

        if (m_isDeath)
        {
            return;
        }

        m_damageEventCount++;

        StopDamageCoroutineOnly();

        m_damageCoroutine = StartCoroutine(DamageNoizeCoroutine());

        Debug.Log("DamagePostEffectController : Damage Noize 発火");
    }

    private IEnumerator DamageNoizeCoroutine()
    {
        SetCurrentValue(m_damageStartIntensity, m_damageStartMaskStrength);

        yield return null;

        float timer = 0.0f;

        while (timer < m_damageFadeTime)
        {
            if (m_useManualControl)
            {
                m_damageCoroutine = null;
                yield break;
            }

            if (m_isDeath)
            {
                m_damageCoroutine = null;
                yield break;
            }

            float rate = Mathf.Clamp01(timer / m_damageFadeTime);

            float intensity = Mathf.Lerp(
                m_damageStartIntensity,
                m_damageEndIntensity,
                rate
            );

            float maskStrength = Mathf.Lerp(
                m_damageStartMaskStrength,
                m_damageEndMaskStrength,
                rate
            );

            SetCurrentValue(intensity, maskStrength);

            timer += Time.unscaledDeltaTime;

            yield return null;
        }

        SetCurrentValue(m_damageEndIntensity, m_damageEndMaskStrength);

        m_damageCoroutine = null;

        Debug.Log("DamagePostEffectController : Damage Noize 終了");

        if (OnDamageNoizeFinished != null)
        {
            OnDamageNoizeFinished.Invoke();
        }
    }

    private void PlayDeathNoize()
    {
        if (m_useManualControl)
        {
            return;
        }

        if (!m_enableControl)
        {
            return;
        }

        if (!m_useDeathNoize)
        {
            return;
        }

        if (m_isDeath)
        {
            return;
        }

        m_isDeath = true;
        m_isDeathNoizeFinished = false;

        StopAllNoizeCoroutine();

        m_deathCoroutine = StartCoroutine(DeathNoizeCoroutine());

        Debug.Log("DamagePostEffectController : Death Noize 発火");
    }

    private IEnumerator DeathNoizeCoroutine()
    {
        SetCurrentValue(m_deathStartIntensity, m_deathMaskStrength);

        yield return null;

        float timer = 0.0f;

        while (timer < m_deathFadeTime)
        {
            if (m_useManualControl)
            {
                m_deathCoroutine = null;
                yield break;
            }

            float rate = Mathf.Clamp01(timer / m_deathFadeTime);

            float intensity = Mathf.Lerp(
                m_deathStartIntensity,
                m_deathEndIntensity,
                rate
            );

            SetCurrentValue(intensity, m_deathMaskStrength);

            timer += Time.unscaledDeltaTime;

            yield return null;
        }

        SetCurrentValue(m_deathEndIntensity, m_deathMaskStrength);

        if (m_waitOneFrameBeforeDeathFinishedEvent)
        {
            yield return null;
        }

        m_deathCoroutine = null;
        m_isDeathNoizeFinished = true;

        Debug.Log("DamagePostEffectController : Death Noize 終了");

        if (OnDeathNoizeFinished != null)
        {
            OnDeathNoizeFinished.Invoke();
        }

        if (m_disableControlWhenDeathFinished)
        {
            IsControlEnabled = false;
        }
    }

    private void StopDamageCoroutineOnly()
    {
        if (m_damageCoroutine != null)
        {
            StopCoroutine(m_damageCoroutine);
            m_damageCoroutine = null;
        }
    }

    private void StopAllNoizeCoroutine()
    {
        if (m_damageCoroutine != null)
        {
            StopCoroutine(m_damageCoroutine);
            m_damageCoroutine = null;
        }

        if (m_deathCoroutine != null)
        {
            StopCoroutine(m_deathCoroutine);
            m_deathCoroutine = null;
        }
    }

    private void UpdateDebugValue()
    {
        m_debugCurrentIntensity = CurrentIntensity;
        m_debugCurrentMaskStrength = CurrentMaskStrength;
    }
}