using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

/// <summary>
/// プレイヤーのアビリティゲージUIを管理するクラス。
/// 
/// ・Frame と Capsule は別Prefabとして複製して使う
/// ・PlaceOnly は配置するだけの追加Prefabとして使う
/// ・一番最初の位置は Inspector で指定した RectTransform から取得する
/// ・Spacing は毎フレーム反映できる
/// ・StartPositionSource をScene上で動かすと、生成済みUIも追従する
/// ・Frame は MaxAbilityEnergy の残り分として fillAmount を変更する
/// ・Capsule は AbilityEnergy の現在値として fillAmount を変更する
/// ・MaxAbilityEnergy は表示するUI数と、最後の小数部分の表示に使う
/// </summary>
public class UI_AbilityGauge : UI_PlayerBase
{
    [Header("生成元")]
    [SerializeField] private RectTransform m_framePrefab;
    [SerializeField] private RectTransform m_capsulePrefab;

    [Header("配置するだけの追加Prefab")]
    [SerializeField] private RectTransform m_placeOnlyPrefab;

    [Header("生成先")]
    [SerializeField] private RectTransform m_parent;

    [Header("配置")]
    [SerializeField] private RectTransform m_startPositionSource;
    [SerializeField] private int m_maxUICount = 10;
    [SerializeField] private float m_spacing = 80.0f;

    [Header("配置の自動更新")]
    [SerializeField] private bool m_updatePositionEveryFrame = true;

    [Header("参照")]
    [SerializeField, ReadOnly] private Status m_status;

    [SerializeField, ReadOnly] private Parameter m_abilityEnergy;
    [SerializeField, ReadOnly] private Parameter m_maxAbilityEnergy;

    private readonly List<RectTransform> m_roots = new List<RectTransform>();
    private readonly List<Image> m_capsuleImages = new List<Image>();
    private readonly List<Image> m_frameImages = new List<Image>();
    private readonly List<RectTransform> m_placeOnlyObjects = new List<RectTransform>();

    private Vector2 m_startPosition;

    private Vector2 m_prevStartPosition;
    private float m_prevSpacing = -1.0f;

    private float m_prevAbilityEnergy = -1.0f;
    private float m_prevMaxAbilityEnergy = -1.0f;

    protected override void Awake()
    {
        base.Awake();
    }

    private void Start()
    {
        if (!HasOwner())
        {
            Debug.LogWarning($"{nameof(UI_Ability)} : Owner が見つかりません。");
            return;
        }

        m_status = m_playerOwner.Status;

        if (m_status == null)
        {
            Debug.LogWarning($"{nameof(UI_Ability)} : Status が見つかりません。");
            return;
        }

        m_abilityEnergy = m_status.GetParameter(ParameterStatusType.AbilityEnergy);
        m_maxAbilityEnergy = m_status.GetParameter(ParameterStatusType.MaxAbilityEnergy);

        if (m_abilityEnergy == null)
        {
            Debug.LogWarning($"{nameof(UI_Ability)} : AbilityEnergy が見つかりません。");
            return;
        }

        if (m_maxAbilityEnergy == null)
        {
            Debug.LogWarning($"{nameof(UI_Ability)} : MaxAbilityEnergy が見つかりません。");
            return;
        }

        CreateUI();

        UpdateMaxCount(m_maxAbilityEnergy.DefaultValue);
        UpdateCapsule(m_abilityEnergy.DefaultValue);

        m_prevAbilityEnergy = m_abilityEnergy.DefaultValue;
        m_prevMaxAbilityEnergy = m_maxAbilityEnergy.DefaultValue;
    }

    private void Update()
    {
        if (m_updatePositionEveryFrame)
        {
            UpdateLayoutIfNeeded();
        }

        if (m_status == null)
        {
            return;
        }

        if (m_abilityEnergy == null || m_maxAbilityEnergy == null)
        {
            return;
        }

        if (!Mathf.Approximately(m_prevMaxAbilityEnergy, m_maxAbilityEnergy.DefaultValue))
        {
            UpdateMaxCount(m_maxAbilityEnergy.DefaultValue);
            UpdateCapsule(m_abilityEnergy.DefaultValue);

            m_prevMaxAbilityEnergy = m_maxAbilityEnergy.DefaultValue;
        }

        if (!Mathf.Approximately(m_prevAbilityEnergy, m_abilityEnergy.DefaultValue))
        {
            UpdateMaxCount(m_maxAbilityEnergy.DefaultValue);
            UpdateCapsule(m_abilityEnergy.DefaultValue);

            m_prevAbilityEnergy = m_abilityEnergy.DefaultValue;
        }
    }

    /// <summary>
    /// Frame / PlaceOnly / Capsule を指定数だけ生成する。
    /// </summary>
    private void CreateUI()
    {
        if (m_parent == null)
        {
            m_parent = transform as RectTransform;
        }

        if (m_parent == null)
        {
            Debug.LogWarning($"{nameof(UI_Ability)} : m_parent が設定されていません。");
            return;
        }

        if (m_framePrefab == null)
        {
            Debug.LogWarning($"{nameof(UI_Ability)} : m_framePrefab が設定されていません。");
            return;
        }

        if (m_capsulePrefab == null)
        {
            Debug.LogWarning($"{nameof(UI_Ability)} : m_capsulePrefab が設定されていません。");
            return;
        }

        if (m_startPositionSource == null)
        {
            Debug.LogWarning($"{nameof(UI_Ability)} : m_startPositionSource が設定されていません。");
            return;
        }

        ClearGeneratedLists();

        m_framePrefab.gameObject.SetActive(false);
        m_capsulePrefab.gameObject.SetActive(false);

        if (m_placeOnlyPrefab != null)
        {
            m_placeOnlyPrefab.gameObject.SetActive(false);
        }

        for (int i = 0; i < m_maxUICount; i++)
        {
            RectTransform root = new GameObject($"AbilityRoot_{i}", typeof(RectTransform)).GetComponent<RectTransform>();
            root.SetParent(m_parent, false);

            root.anchorMin = m_startPositionSource.anchorMin;
            root.anchorMax = m_startPositionSource.anchorMax;
            root.pivot = m_startPositionSource.pivot;
            root.sizeDelta = m_startPositionSource.sizeDelta;

            RectTransform frame = Instantiate(m_framePrefab, root);
            frame.gameObject.SetActive(true);
            frame.anchoredPosition = Vector2.zero;

            RectTransform placeOnly = null;

            if (m_placeOnlyPrefab != null)
            {
                placeOnly = Instantiate(m_placeOnlyPrefab, root);
                placeOnly.gameObject.SetActive(true);
                placeOnly.anchoredPosition = Vector2.zero;
            }

            RectTransform capsule = Instantiate(m_capsulePrefab, root);
            capsule.gameObject.SetActive(true);
            capsule.anchoredPosition = Vector2.zero;

            Image frameImage = frame.GetComponent<Image>();
            Image capsuleImage = capsule.GetComponent<Image>();

            if (capsuleImage == null)
            {
                Debug.LogWarning($"{nameof(UI_Ability)} : CapsulePrefab に Image がありません。");
                continue;
            }

            if (frameImage != null)
            {
                frameImage.fillAmount = 0.0f;
            }

            capsuleImage.fillAmount = 0.0f;

            m_roots.Add(root);
            m_capsuleImages.Add(capsuleImage);
            m_frameImages.Add(frameImage);
            m_placeOnlyObjects.Add(placeOnly);
        }

        ForceUpdateLayout();
    }

    /// <summary>
    /// 生成済みUIの管理リストを初期化する。
    /// </summary>
    private void ClearGeneratedLists()
    {
        m_roots.Clear();
        m_capsuleImages.Clear();
        m_frameImages.Clear();
        m_placeOnlyObjects.Clear();
    }

    /// <summary>
    /// StartPositionSource や Spacing が変わっていたら配置を更新する。
    /// </summary>
    private void UpdateLayoutIfNeeded()
    {
        if (m_startPositionSource == null)
        {
            return;
        }

        Vector2 currentStartPosition = m_startPositionSource.anchoredPosition;

        bool isStartPositionChanged = !ApproximatelyVector2(m_prevStartPosition, currentStartPosition);
        bool isSpacingChanged = !Mathf.Approximately(m_prevSpacing, m_spacing);

        if (!isStartPositionChanged && !isSpacingChanged)
        {
            return;
        }

        m_startPosition = currentStartPosition;

        ApplyPositions();

        m_prevStartPosition = currentStartPosition;
        m_prevSpacing = m_spacing;
    }

    /// <summary>
    /// 強制的に配置を更新する。
    /// </summary>
    private void ForceUpdateLayout()
    {
        if (m_startPositionSource == null)
        {
            return;
        }

        m_startPosition = m_startPositionSource.anchoredPosition;

        ApplyPositions();

        m_prevStartPosition = m_startPosition;
        m_prevSpacing = m_spacing;
    }

    /// <summary>
    /// StartPosition と Spacing を元にUI位置を並べ直す。
    /// </summary>
    private void ApplyPositions()
    {
        for (int i = 0; i < m_roots.Count; i++)
        {
            if (m_roots[i] == null)
            {
                continue;
            }

            m_roots[i].anchoredPosition = new Vector2(
                m_startPosition.x + m_spacing * i,
                m_startPosition.y
            );
        }
    }

    /// <summary>
    /// AbilityEnergy に応じて Capsule の fillAmount を更新する。
    /// </summary>
    private void UpdateCapsule(float currentEnergy)
    {
        int uiCount = GetUICount();

        if (uiCount <= 0)
        {
            return;
        }

        float maxAbilityEnergy = m_maxAbilityEnergy.DefaultValue;
        maxAbilityEnergy = Mathf.Clamp(maxAbilityEnergy, 0.0f, uiCount);

        currentEnergy = Mathf.Clamp(currentEnergy, 0.0f, maxAbilityEnergy);

        for (int i = 0; i < uiCount; i++)
        {
            if (m_capsuleImages[i] == null)
            {
                continue;
            }

            float capsuleFill = Mathf.Clamp01(currentEnergy - i);

            m_capsuleImages[i].fillAmount = capsuleFill;
        }
    }

    /// <summary>
    /// MaxAbilityEnergy に応じて表示するゲージ数を変更する。
    /// 
    /// MaxAbilityEnergy が小数の場合は、最後の Frame だけ fillAmount で表示する。
    /// 例：
    /// MaxAbilityEnergy = 3.0  → 3個表示
    /// MaxAbilityEnergy = 3.5  → 4個表示、4個目の最大枠は0.5
    /// MaxAbilityEnergy = 2.25 → 3個表示、3個目の最大枠は0.25
    /// </summary>
    private void UpdateMaxCount(float maxAbilityEnergy)
    {
        int uiCount = GetUICount();

        if (uiCount <= 0)
        {
            return;
        }

        maxAbilityEnergy = Mathf.Clamp(maxAbilityEnergy, 0.0f, uiCount);

        float currentEnergy = m_abilityEnergy.DefaultValue;
        currentEnergy = Mathf.Clamp(currentEnergy, 0.0f, maxAbilityEnergy);

        int visibleCount = Mathf.CeilToInt(maxAbilityEnergy);

        for (int i = 0; i < uiCount; i++)
        {
            if (m_roots[i] == null)
            {
                continue;
            }

            bool isVisible = i < visibleCount;

            m_roots[i].gameObject.SetActive(isVisible);

            if (!isVisible)
            {
                if (m_frameImages[i] != null)
                {
                    m_frameImages[i].fillAmount = 0.0f;
                }

                if (m_capsuleImages[i] != null)
                {
                    m_capsuleImages[i].fillAmount = 0.0f;
                }

                continue;
            }

            if (m_frameImages[i] == null)
            {
                continue;
            }

            float maxFill = Mathf.Clamp01(maxAbilityEnergy - i);
            float currentFill = Mathf.Clamp01(currentEnergy - i);

            float frameFill = maxFill - currentFill;
            frameFill = Mathf.Clamp01(frameFill);

            m_frameImages[i].fillAmount = frameFill;
        }
    }

    /// <summary>
    /// 安全に使えるUI数を返す。
    /// </summary>
    private int GetUICount()
    {
        return Mathf.Min(
            m_roots.Count,
            m_capsuleImages.Count
        );
    }

    /// <summary>
    /// Vector2 の近似比較。
    /// </summary>
    private bool ApproximatelyVector2(Vector2 a, Vector2 b)
    {
        return Mathf.Approximately(a.x, b.x)
            && Mathf.Approximately(a.y, b.y);
    }
}