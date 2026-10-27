// 【ファイルの役割】3D HPバーの旧実装をコメントとして保持している。
// 【依存・注意点】クラス全体がコメントアウトされており、このファイルから実行される処理はない。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

//using UnityEngine;
//using UnityEngine.UI;

///// <summary>
///// プレイヤーのHPをUIバーとして表示するクラス。
///// HPの値が変化した時のみ fillAmount を更新し、無駄なUI更新を抑える。
///// </summary>
//public class UI_HPBar3D : UI_PlayerBase
//{
//    [Header("＊ HPBar＊")]
//    [Header("Image")]
//    [SerializeField] private RectTransform m_hpBar;

//    [SerializeField] private Image m_filledImage;

//    [Header("Follow")]
//    [Tooltip("プレイヤーの背中側にどれだけ離して置くか")]
//    [SerializeField] private float m_backDistance = 1.0f;

//    [Tooltip("頭上にどれだけ上げるか")]
//    [SerializeField] private float m_heightOffset = 1.5f;

//    [Tooltip("画面上でさらにずらす量")]
//    [SerializeField] private Vector2 m_screenOffset = Vector2.zero;

//    [Header("Rotation")]
//    [Tooltip("プレイヤーの向きに合わせてUIを回転させるか")]
//    [SerializeField] private bool m_usePlayerDirectionRotation = true;

//    [Tooltip("弧の画像の向き調整。画像が逆なら 180 などにする")]
//    [SerializeField] private float m_rotationOffset = 0.0f;

//    [Tooltip("円周上の角度オフセット")]
//    [SerializeField] private float m_aroundAngleOffset = 0.0f;

//    [Header("In Game")]
//    [SerializeField, ReadOnly] private BaseStatus m_status;

//    [SerializeField, ReadOnly] private Parameter m_currentHP;
//    [SerializeField, ReadOnly] private Parameter m_maxHP;

//    [Header("Animator")]
//    [SerializeField] private string m_damageTriggerName = "damage";
//    [SerializeField] private string m_healTriggerName = "heal";
//    [SerializeField] private string m_lowHPBoolName = "lowHP";
//    [SerializeField] private float m_lowHPRate = 0.3f;

//    private bool m_isLowHP = false;

//    // 前回の値、更新するかどうか
//    private float m_prevCurrentHP = 0.0f;
//    protected override void Awake()
//    {
//        base.Awake();
//    }

//    private void Start()
//    {
//        if (m_player == null)
//        {
//            Debug.LogWarning($"{nameof(UI_HPBar)} : Player が見つかりません。");
//            return;
//        }

//        m_status = m_player.GetComponent<BaseStatus>();

//        if (m_status == null)
//        {
//            Debug.LogWarning($"{nameof(UI_HPBar)} : BaseStatus が見つかりません。");
//            return;
//        }

//        m_currentHP = m_status.GetParameter(ParameterStatusType.HP);
//        m_maxHP = m_status.GetParameter(ParameterStatusType.MaxHP);

//        // 初回はゲージ表示と lowHP 状態だけ合わせる。
//        // damage / heal は鳴らさない。
//        UpdateHP(true);
//    }
//    private void Update()
//    {
//        //プレイヤーの3D座標を、画面上の2D位置に変換
//        Vector2 playerScreenPos = WorldToScreen2D(m_player.position);

//        //----------------------
//        //回転
//        //----------------------
//        UpdatePositionAndRotation();

//        // HPの値が前回と異なる場合にのみUIを更新
//        if (!Mathf.Approximately(m_prevCurrentHP, m_currentHP.CalculateValue))
//        {
//            UpdateHP(false);
//        }
//    }

//    /// <summary>
//    /// プレイヤーの背中側にUIを配置し、必要なら向きも合わせる。
//    /// </summary>
//    private void UpdatePositionAndRotation()
//    {
//        Vector3 aroundWorldPos = GetAroundPlayerWorldPosition();

//        Vector2 uiScreenPos = WorldToScreen2D(aroundWorldPos);

//        m_hpBar.position = uiScreenPos + m_screenOffset;

//        if (!m_usePlayerDirectionRotation)
//        {
//            return;
//        }

//        Vector2 playerScreenPos = WorldToScreen2D(m_player.position + Vector3.up * m_heightOffset);

//        Vector2 dir = uiScreenPos - playerScreenPos;

//        if (dir.sqrMagnitude <= 0.001f)
//        {
//            return;
//        }

//        float angle = Mathf.Atan2(dir.y, dir.x) * Mathf.Rad2Deg;

//        m_hpBar.rotation = Quaternion.Euler(0.0f, 0.0f, angle + m_rotationOffset);
//    }

//    /// <summary>
//    /// プレイヤーの周囲の円周上にUIを配置する。
//    /// 0度なら背中側、プラスで右回り、マイナスで左回りにずらす。
//    /// </summary>
//    private Vector3 GetAroundPlayerWorldPosition()
//    {
//        // 背中方向
//        Vector3 backDir = -m_player.forward;

//        // Y軸回転で、背中方向を円周上にずらす
//        Vector3 rotatedDir = Quaternion.Euler(0.0f, m_aroundAngleOffset, 0.0f) * backDir;

//        // プレイヤー中心から rotatedDir 方向に距離分ずらす
//        Vector3 worldPos =
//            m_player.position
//            + rotatedDir * m_backDistance
//            + Vector3.up * m_heightOffset;

//        return worldPos;
//    }

//    /// <summary>
//    /// HP表示とHPアニメーションを更新する。
//    /// </summary>
//    private void UpdateHP(bool isFirstUpdate)
//    {
//        if (m_filledImage == null)
//        {
//            return;
//        }

//        float currentHP = m_currentHP.CalculateValue;
//        float maxHP = m_maxHP.CalculateValue;

//        Debug.Log($"HP : {currentHP} / {maxHP}");

//        if (maxHP <= 0.0f)
//        {
//            m_filledImage.fillAmount = 0.0f;
//            m_prevCurrentHP = currentHP;
//            return;
//        }

//        float hpRate = Mathf.Clamp01(currentHP / maxHP);

//        m_filledImage.fillAmount = hpRate;

//        if (isFirstUpdate)
//        {
//            UpdateLowHPAnimation(hpRate);
//        }
//        else
//        {
//            UpdateHPAnimation(currentHP, maxHP);
//        }

//        // damage / heal 判定のあとに前回値を更新する
//        m_prevCurrentHP = currentHP;
//    }

//    /// <summary>
//    /// HP変化に応じたアニメーションを再生する。
//    /// </summary>
//    private void UpdateHPAnimation(float currentHP, float maxHP)
//    {
//        if (m_animator == null)
//        {
//            return;
//        }

//        if (maxHP <= 0.0f)
//        {
//            return;
//        }

//        float hpRate = Mathf.Clamp01(currentHP / maxHP);

//        if (currentHP < m_prevCurrentHP)
//        {
//            m_animator.ResetTrigger(m_healTriggerName);
//            m_animator.SetTrigger(m_damageTriggerName);
//        }
//        else if (currentHP > m_prevCurrentHP)
//        {
//            m_animator.ResetTrigger(m_damageTriggerName);
//            m_animator.SetTrigger(m_healTriggerName);
//        }

//        UpdateLowHPAnimation(hpRate);
//    }

//    /// <summary>
//    /// 低HP状態のアニメーションを更新する。
//    /// </summary>
//    private void UpdateLowHPAnimation(float hpRate)
//    {
//        if (m_animator == null)
//        {
//            return;
//        }

//        bool isLowHP = hpRate <= m_lowHPRate;

//        if (isLowHP == m_isLowHP)
//        {
//            return;
//        }

//        m_isLowHP = isLowHP;
//        m_animator.SetBool(m_lowHPBoolName, m_isLowHP);
//    }
//}