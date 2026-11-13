using UnityEngine;
using TMPro;

public class UI_PlayerStatusText : UI_PlayerBase
{
    private enum PlayerStatusKind
    {
        HP,
        AbilityEnergy,
        Speed,
        ViewDistance,
        Heal,
        AbilityEnergyHeal,
        HearingRange,
        ViewAngle,
    }

    private enum StatusDisplayType
    {
        CurrentOnly,
        CurrentAndMax,
    }

    [Header("＊ Player Status Text ＊")]
    [Header("Text")]
    [SerializeField] private TextMeshProUGUI m_nameText;
    [SerializeField] private TextMeshProUGUI m_valueText;

    [Header("Select")]
    [SerializeField] private PlayerStatusKind m_statusKind = PlayerStatusKind.HP;

    [Header("Display")]
    [SerializeField] private int m_decimalCount = 1;

    [Header("Position - Root")]
    [SerializeField] private RectTransform m_positionTarget;
    [SerializeField] private bool m_useDefaultPosition = true;
    [SerializeField] private Vector2 m_defaultPosition = Vector2.zero;

    [Header("Position - Text Offset")]
    [SerializeField] private Vector2 m_nameTextOffset = Vector2.zero;
    [SerializeField] private Vector2 m_valueTextOffset = Vector2.zero;
    [SerializeField] private bool m_updatePositionEveryFrame = true;

    [Header("In Game")]
    [SerializeField, ReadOnly] private Status m_status;

    [SerializeField, ReadOnly] private Parameter m_currentParameter;
    [SerializeField, ReadOnly] private Parameter m_maxParameter;

    [SerializeField, ReadOnly] private float m_currentValue;
    [SerializeField, ReadOnly] private float m_maxValue;

    private StatusDisplayType m_displayType = StatusDisplayType.CurrentOnly;

    private string m_displayName = "";
    private string m_unit = "";

    private float m_prevCurrentValue = float.MinValue;
    private float m_prevMaxValue = float.MinValue;

    private RectTransform m_nameTextRect;
    private RectTransform m_valueTextRect;

    private Vector2 m_baseRootPosition;
    private Vector2 m_baseNameTextPosition;
    private Vector2 m_baseValueTextPosition;

    private bool m_prevUseDefaultPosition;
    private Vector2 m_prevDefaultPosition;
    private Vector2 m_prevNameTextOffset;
    private Vector2 m_prevValueTextOffset;

    protected override void Awake()
    {
        base.Awake();

        InitializeRectTransforms();
        CacheBasePositions();
        ApplyPositions();
        CachePrevPositionValues();
    }

    private void Start()
    {
        if (!HasOwner())
        {
            Debug.LogWarning($"{nameof(UI_PlayerStatusText)} : Owner が見つかりません。");
            return;
        }

        m_status = m_playerOwner.Status;

        if (m_status == null)
        {
            Debug.LogWarning($"{nameof(UI_PlayerStatusText)} : Status が見つかりません。");
            return;
        }

        InitializeStatus();
        UpdateStatus(true);
    }

    private void Update()
    {
        if (m_updatePositionEveryFrame)
        {
            UpdatePositionIfNeeded();
        }

        if (m_status == null)
        {
            return;
        }

        if (m_currentParameter == null)
        {
            return;
        }

        UpdateStatus(false);
    }

    private void InitializeRectTransforms()
    {
        if (m_positionTarget == null)
        {
            m_positionTarget = transform as RectTransform;
        }

        if (m_nameText != null)
        {
            m_nameTextRect = m_nameText.rectTransform;
        }

        if (m_valueText != null)
        {
            m_valueTextRect = m_valueText.rectTransform;
        }
    }

    private void CacheBasePositions()
    {
        if (m_positionTarget != null)
        {
            m_baseRootPosition = m_positionTarget.anchoredPosition;
        }

        if (m_nameTextRect != null)
        {
            m_baseNameTextPosition = m_nameTextRect.anchoredPosition;
        }

        if (m_valueTextRect != null)
        {
            m_baseValueTextPosition = m_valueTextRect.anchoredPosition;
        }
    }

    private void UpdatePositionIfNeeded()
    {
        bool isChanged =
            m_prevUseDefaultPosition != m_useDefaultPosition ||
            !ApproximatelyVector2(
                m_prevDefaultPosition,
                m_defaultPosition
            ) ||
            !ApproximatelyVector2(
                m_prevNameTextOffset,
                m_nameTextOffset
            ) ||
            !ApproximatelyVector2(
                m_prevValueTextOffset,
                m_valueTextOffset
            );

        if (!isChanged)
        {
            return;
        }

        ApplyPositions();
        CachePrevPositionValues();
    }

    private void ApplyPositions()
    {
        if (m_positionTarget != null)
        {
            if (m_useDefaultPosition)
            {
                m_positionTarget.anchoredPosition = m_defaultPosition;
            }
            else
            {
                m_positionTarget.anchoredPosition = m_baseRootPosition;
            }
        }

        if (m_nameTextRect != null)
        {
            m_nameTextRect.anchoredPosition = m_baseNameTextPosition + m_nameTextOffset;
        }

        if (m_valueTextRect != null)
        {
            m_valueTextRect.anchoredPosition = m_baseValueTextPosition + m_valueTextOffset;
        }
    }

    private void CachePrevPositionValues()
    {
        m_prevUseDefaultPosition = m_useDefaultPosition;
        m_prevDefaultPosition = m_defaultPosition;
        m_prevNameTextOffset = m_nameTextOffset;
        m_prevValueTextOffset = m_valueTextOffset;
    }

    public void SetDefaultPosition(Vector2 position)
    {
        m_defaultPosition = position;
        m_useDefaultPosition = true;

        ApplyPositions();
        CachePrevPositionValues();
    }

    public void SetNameTextOffset(Vector2 offset)
    {
        m_nameTextOffset = offset;

        ApplyPositions();
        CachePrevPositionValues();
    }

    public void SetValueTextOffset(Vector2 offset)
    {
        m_valueTextOffset = offset;

        ApplyPositions();
        CachePrevPositionValues();
    }

    private bool ApproximatelyVector2(
        Vector2 a,
        Vector2 b
    )
    {
        return Mathf.Approximately(
            a.x,
            b.x
        ) &&
        Mathf.Approximately(
            a.y,
            b.y
        );
    }

    private void InitializeStatus()
    {
        if (TryInitializeFixedStatus())
        {
            return;
        }

        bool hasMax = TryGetStatusSetting(
            m_statusKind,
            out ParameterStatusType currentType,
            out ParameterStatusType maxType,
            out m_displayName,
            out m_unit,
            out m_displayType
        );

        m_currentParameter = m_status.GetParameter(currentType);

        if (hasMax)
        {
            m_maxParameter = m_status.GetParameter(maxType);
        }
        else
        {
            m_maxParameter = null;
        }

        if (m_nameText != null)
        {
            m_nameText.text = m_displayName;
        }

        m_prevCurrentValue = float.MinValue;
        m_prevMaxValue = float.MinValue;
    }

    private bool TryInitializeFixedStatus()
    {
        switch (m_statusKind)
        {
            case PlayerStatusKind.Heal:
                SetFixedStatus(
                    "再生力",
                    0.0f,
                    "/s"
                );
                return true;

            case PlayerStatusKind.HearingRange:
                SetFixedStatus(
                    "音達範囲",
                    15.0f,
                    "m"
                );
                return true;
        }

        return false;
    }

    private void SetFixedStatus(
        string displayName,
        float value,
        string unit
    )
    {
        m_currentParameter = null;
        m_maxParameter = null;

        m_displayName = displayName;
        m_unit = unit;

        m_currentValue = value;
        m_maxValue = 0.0f;

        if (m_nameText != null)
        {
            m_nameText.text = displayName;
        }

        if (m_valueText != null)
        {
            m_valueText.text = $"{FormatValue(value, m_decimalCount)} {unit}";
        }
    }

    private void UpdateStatus(bool isFirstUpdate)
    {
        if (m_valueText == null)
        {
            return;
        }

        if (m_currentParameter == null)
        {
            return;
        }

        float currentValue = m_currentParameter.CalculateValue;

        if (m_displayType == StatusDisplayType.CurrentOnly)
        {
            UpdateCurrentOnlyStatus(
                currentValue,
                isFirstUpdate
            );
            return;
        }

        if (m_displayType == StatusDisplayType.CurrentAndMax)
        {
            UpdateCurrentAndMaxStatus(
                currentValue,
                isFirstUpdate
            );
        }
    }

    private void UpdateCurrentOnlyStatus(
        float currentValue,
        bool isFirstUpdate
    )
    {
        if (!isFirstUpdate &&
            Mathf.Approximately(
                m_prevCurrentValue,
                currentValue
            ))
        {
            return;
        }

        m_currentValue = currentValue;
        m_prevCurrentValue = currentValue;

        m_valueText.text = CreateCurrentOnlyText(currentValue);
    }

    private void UpdateCurrentAndMaxStatus(
        float currentValue,
        bool isFirstUpdate
    )
    {
        if (m_maxParameter == null)
        {
            m_valueText.text = "-";
            return;
        }

        float maxValue = m_maxParameter.CalculateValue;

        bool isCurrentChanged = !Mathf.Approximately(
            m_prevCurrentValue,
            currentValue
        );

        bool isMaxChanged = !Mathf.Approximately(
            m_prevMaxValue,
            maxValue
        );

        if (!isFirstUpdate &&
            !isCurrentChanged &&
            !isMaxChanged)
        {
            return;
        }

        m_currentValue = currentValue;
        m_maxValue = maxValue;

        m_prevCurrentValue = currentValue;
        m_prevMaxValue = maxValue;

        m_valueText.text = CreateCurrentAndMaxText(
            currentValue,
            maxValue
        );
    }

    private bool TryGetStatusSetting(
        PlayerStatusKind kind,
        out ParameterStatusType currentType,
        out ParameterStatusType maxType,
        out string displayName,
        out string unit,
        out StatusDisplayType displayType
    )
    {
        currentType = ParameterStatusType.HP;
        maxType = ParameterStatusType.MaxHP;
        displayName = "";
        unit = "";
        displayType = StatusDisplayType.CurrentOnly;

        switch (kind)
        {
            case PlayerStatusKind.HP:
                currentType = ParameterStatusType.HP;
                maxType = ParameterStatusType.MaxHP;
                displayName = "生命力";
                unit = "";
                displayType = StatusDisplayType.CurrentAndMax;
                return true;

            case PlayerStatusKind.AbilityEnergy:
                currentType = ParameterStatusType.AbilityEnergy;
                maxType = ParameterStatusType.MaxAbilityEnergy;
                displayName = "ES値";
                unit = "";
                displayType = StatusDisplayType.CurrentAndMax;
                return true;

            case PlayerStatusKind.Speed:
                currentType = ParameterStatusType.Speed;
                maxType = ParameterStatusType.Speed;
                displayName = "移動速度";
                unit = "m/s";
                displayType = StatusDisplayType.CurrentOnly;
                return false;

            case PlayerStatusKind.ViewDistance:
                currentType = ParameterStatusType.ViewDistance;
                maxType = ParameterStatusType.ViewDistance;
                displayName = "視程";
                unit = "";
                displayType = StatusDisplayType.CurrentOnly;
                return false;

            case PlayerStatusKind.AbilityEnergyHeal:
                currentType = ParameterStatusType.AbilityEnergyHeal;
                maxType = ParameterStatusType.AbilityEnergyHeal;
                displayName = "ES再生";
                unit = "/s";
                displayType = StatusDisplayType.CurrentOnly;
                return false;

            case PlayerStatusKind.ViewAngle:
                currentType = ParameterStatusType.ViewAngle;
                maxType = ParameterStatusType.ViewAngle;
                displayName = "視野";
                unit = "°";
                displayType = StatusDisplayType.CurrentOnly;
                return false;
        }

        return false;
    }

    private string CreateCurrentOnlyText(float currentValue)
    {
        string currentText = FormatValue(
            currentValue,
            m_decimalCount
        );

        if (string.IsNullOrEmpty(m_unit))
        {
            return currentText;
        }

        return $"{currentText} {m_unit}";
    }

    private string CreateCurrentAndMaxText(
        float currentValue,
        float maxValue
    )
    {
        string currentText = FormatValue(
            currentValue,
            m_decimalCount
        );

        string maxText = FormatValue(
            maxValue,
            m_decimalCount
        );

        if (string.IsNullOrEmpty(m_unit))
        {
            return $"{currentText} / {maxText}";
        }

        return $"{currentText} / {maxText} {m_unit}";
    }

    private string FormatValue(
        float value,
        int decimalCount
    )
    {
        decimalCount = Mathf.Clamp(
            decimalCount,
            0,
            4
        );

        switch (decimalCount)
        {
            case 0:
                return Mathf.RoundToInt(value).ToString();

            case 1:
                return value.ToString("0.0");

            case 2:
                return value.ToString("0.00");

            case 3:
                return value.ToString("0.000");

            case 4:
                return value.ToString("0.0000");
        }

        return value.ToString("0.0");
    }
}
