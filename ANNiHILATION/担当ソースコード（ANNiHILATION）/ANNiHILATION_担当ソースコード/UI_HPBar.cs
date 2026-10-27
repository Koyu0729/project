// 【ファイルの役割】現在HPと最大HPからゲージ割合を計算し、被ダメージ・回復・低HP演出を更新する。
// 【依存・注意点】現在HPまたは最大HPが変わった場合に更新する。最大HPが0以下ならゲージを0にする。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using UnityEngine;
using UnityEngine.UI;

/// <summary>
/// プレイヤーのHPをUIバーとして表示するクラス。
/// HPの値が変化した時のみ fillAmount を更新し、無駄なUI更新を抑える。
/// </summary>
public class UI_HPBar : UI_PlayerBase
{
    [Header("＊ HPBar＊")]
    [Header("Image")]
    [SerializeField] private Image m_filledImage;

    [Header("In Game")]
    [SerializeField, ReadOnly] private Status m_status;

    [SerializeField, ReadOnly] private Parameter m_currentHP;
    [SerializeField, ReadOnly] private Parameter m_maxHP;

    [Header("Animator")]
    [SerializeField] private string m_damageTriggerName = "damage";
    [SerializeField] private string m_healTriggerName = "heal";
    [SerializeField] private string m_lowHPBoolName = "lowHP";
    [SerializeField] private float m_lowHPRate = 0.3f;

    private bool m_isLowHP = false;

    // 前回の値、更新するかどうか
    private float m_prevCurrentHP = 0.0f;
    private float m_prevMaxHP = 0.0f;
    protected override void Awake()
    {
        base.Awake();
    }

    private void Start()
    {
        if (!HasOwner())
        {
            Debug.LogWarning($"{nameof(UI_HPBar)} : Owner が見つかりません。");
            return;
        }

        m_status = m_playerOwner.Status;

        if (m_status == null)
        {
            Debug.LogWarning($"{nameof(UI_HPBar)} : Status が見つかりません。");
            return;
        }

        m_currentHP = m_status.GetParameter(ParameterStatusType.HP);
        m_maxHP = m_status.GetParameter(ParameterStatusType.MaxHP);

        if (m_currentHP == null || m_maxHP == null)
        {
            Debug.LogWarning($"{nameof(UI_HPBar)} : HP または MaxHP の Parameter が見つかりません。");
            return;
        }

        // 初回はゲージ表示と lowHP 状態だけ合わせる。
        // damage / heal は鳴らさない。
        UpdateHP(true);
    }
    private void Update()
    {
        if (m_status == null)
        {
            return;
        }

        if (m_currentHP == null || m_maxHP == null)
        {
            return;
        }

        // HPの値が前回と異なる場合にのみUIを更新
        if (!Mathf.Approximately(m_prevCurrentHP, m_currentHP.CalculateValue) ||
            !Mathf.Approximately(m_prevMaxHP, m_maxHP.CalculateValue))
        {
            UpdateHP(false);
        }
    }

    /// <summary>
    /// HP表示とHPアニメーションを更新する。
    /// </summary>
    private void UpdateHP(bool isFirstUpdate)
    {
        if (m_filledImage == null)
        {
            return;
        }

        float currentHP = m_currentHP.CalculateValue;
        float maxHP = m_maxHP.CalculateValue;

        Debug.Log($"HP : {currentHP} / {maxHP}");

        if (maxHP <= 0.0f)
        {
            m_filledImage.fillAmount = 0.0f;
            m_prevCurrentHP = currentHP;
            m_prevMaxHP = maxHP;
            UpdateLowHPAnimation(0.0f);
            return;
        }

        float hpRate = Mathf.Clamp01(currentHP / maxHP);

        m_filledImage.fillAmount = hpRate;

        if (isFirstUpdate)
        {
            UpdateLowHPAnimation(hpRate);
        }
        else
        {
            UpdateHPAnimation(currentHP, maxHP);
        }

        // damage / heal 判定のあとに前回値を更新する
        m_prevCurrentHP = currentHP;
        m_prevMaxHP = maxHP;
    }

    /// <summary>
    /// HP変化に応じたアニメーションを再生する。
    /// </summary>
    private void UpdateHPAnimation(float currentHP, float maxHP)
    {
        if (m_animator == null)
        {
            return;
        }

        if (maxHP <= 0.0f)
        {
            return;
        }

        float hpRate = Mathf.Clamp01(currentHP / maxHP);

        if (currentHP < m_prevCurrentHP)
        {
            m_animator.ResetTrigger(m_healTriggerName);
            m_animator.SetTrigger(m_damageTriggerName);
        }
        else if (currentHP > m_prevCurrentHP)
        {
            m_animator.ResetTrigger(m_damageTriggerName);
            m_animator.SetTrigger(m_healTriggerName);
        }

        UpdateLowHPAnimation(hpRate);
    }

    /// <summary>
    /// 低HP状態のアニメーションを更新する。
    /// </summary>
    private void UpdateLowHPAnimation(float hpRate)
    {
        if (m_animator == null)
        {
            return;
        }

        bool isLowHP = hpRate <= m_lowHPRate;

        if (isLowHP == m_isLowHP)
        {
            return;
        }

        m_isLowHP = isLowHP;
        m_animator.SetBool(m_lowHPBoolName, m_isLowHP);
    }
}