using UnityEngine;
using UnityEngine.UI;
using TMPro;

/// <summary>
/// 現在装備している武器の弾数・アイコン・リロード状態を表示するUI。
/// </summary>
public class UI_WeaponUI : UI_PlayerBase
{
    [System.Serializable]
    private class GunCategoryIcon
    {
        public GunCategory category;
        public Sprite icon;
    }

    [Header("Component")]
    [SerializeField] private Image m_gunIcon;
    [SerializeField] private TextMeshProUGUI m_allAmmoText;
    [SerializeField] private TextMeshProUGUI m_magazineAmmoText;
    [SerializeField] private Image m_reloadingPanel;

    [Header("Weapon Icon")]
    [SerializeField] private Sprite m_defaultGunIcon;
    [SerializeField] private Sprite m_hammerIcon;
    [SerializeField] private GunCategoryIcon[] m_gunCategoryIcons;

    [Header("Reload Animation")]
    [SerializeField] private RectTransform m_reloadRotateTarget;
    [SerializeField] private float m_reloadRotateSpeed = 360.0f;

    [Header("In Game")]
    [SerializeField, ReadOnly] private WeaponData m_currentWeapon;
    [SerializeField, ReadOnly] private GunAction m_gunAction;

    [SerializeField, ReadOnly] private Parameter m_allAmmo;
    [SerializeField, ReadOnly] private Parameter m_innerMagazineAmmo;
    [SerializeField, ReadOnly] private Parameter m_magazineCapacity;

    [Header("Ammo Value")]
    [SerializeField, ReadOnly] private int m_allAmmoValue;
    [SerializeField, ReadOnly] private int m_currentMagazineAmmoValue;
    [SerializeField, ReadOnly] private int m_maxMagazineAmmoValue;

    [Header("Weapon Header")]
    [SerializeField] private Image m_weaponHeader1;
    [SerializeField] private Image m_weaponHeader2;

    [Header("Weapon Header Highlight")]
    [SerializeField] private float m_headerActiveAlpha = 1.0f;
    [SerializeField] private float m_headerInactiveAlpha = 0.5f;
    [SerializeField] private Vector3 m_headerActiveScale = Vector3.one;
    [SerializeField] private Vector3 m_headerInactiveScale = Vector3.one;

    [SerializeField, ReadOnly] private bool m_isWeaponHeader1Active = true;

    private WeaponData m_prevWeapon;

    private int m_prevAllAmmoValue = -1;
    private int m_prevCurrentMagazineAmmoValue = -1;
    private int m_prevMaxMagazineAmmoValue = -1;

    private bool m_isMagazineAmmoTextDirty = true;

    protected override void Awake()
    {
        base.Awake();

        if (m_reloadRotateTarget == null && m_gunIcon != null)
        {
            m_reloadRotateTarget = m_gunIcon.rectTransform;
        }

        ApplyWeaponHeaderHighlight();
    }

    private void Update()
    {
        if (!HasOwner())
        {
            return;
        }

        if (m_playerOwner.WeaponHolder == null)
        {
            ClearWeaponUI();
            return;
        }

        WeaponData weapon = m_playerOwner.WeaponHolder.CurrentWeapon;

        if (weapon == null)
        {
            ClearWeaponUI();
            return;
        }

        bool isWeaponChanged = weapon != m_prevWeapon;

        if (isWeaponChanged)
        {
            RefreshWeapon(weapon);
            ToggleWeaponHeaderHighlight();
        }

        if (!IsGunWeapon(weapon))
        {
            return;
        }

        UpdateAmmoUI();
        UpdateReloadUI();
    }

    private void RefreshWeapon(WeaponData weapon)
    {
        m_prevWeapon = weapon;
        m_currentWeapon = weapon;

        UpdateWeaponIcon(weapon);

        if (!IsGunWeapon(weapon))
        {
            ClearWeaponParameters();
            ResetAmmoCache();
            SetAmmoUIVisible(false);
            ResetReloadRotation();
            return;
        }

        m_gunAction = weapon.GetComponent<GunAction>();

        Status status = weapon.Status;

        if (status == null)
        {
            Debug.LogWarning($"{nameof(UI_WeaponUI)} : 武器にStatusが見つかりませんでした。");

            ClearWeaponParameters();
            ResetAmmoCache();
            SetAmmoUIVisible(false);
            ResetReloadRotation();
            return;
        }

        m_allAmmo = status.GetParameter(ParameterStatusType.AllAmmo);
        m_innerMagazineAmmo = status.GetParameter(ParameterStatusType.InnerMagezineAmmo);
        m_magazineCapacity = status.GetParameter(ParameterStatusType.MagazineCapacity);

        ResetAmmoCache();
        SetAmmoUIVisible(true);

        UpdateAmmoUI();
        UpdateReloadUI();
    }

    private bool IsGunWeapon(WeaponData weapon)
    {
        if (weapon == null)
        {
            return false;
        }

        return weapon.DataTable is GunDataTable;
    }

    private void UpdateWeaponIcon(WeaponData weapon)
    {
        if (m_gunIcon == null)
        {
            return;
        }

        Color iconColor = m_gunIcon.color;
        iconColor.a = 1.0f;
        m_gunIcon.color = iconColor;

        if (weapon == null || weapon.DataTable == null)
        {
            m_gunIcon.sprite = m_defaultGunIcon;
            return;
        }

        GunDataTable gunDataTable = weapon.DataTable as GunDataTable;

        if (gunDataTable == null)
        {
            m_gunIcon.sprite = m_hammerIcon;
            return;
        }

        Sprite icon = GetGunCategoryIcon(gunDataTable.WeaponCategory);

        if (icon == null)
        {
            icon = m_defaultGunIcon;
        }

        m_gunIcon.sprite = icon;
    }

    private Sprite GetGunCategoryIcon(GunCategory category)
    {
        if (m_gunCategoryIcons == null)
        {
            return null;
        }

        for (int i = 0; i < m_gunCategoryIcons.Length; i++)
        {
            if (m_gunCategoryIcons[i] == null)
            {
                continue;
            }

            if (m_gunCategoryIcons[i].category == category)
            {
                return m_gunCategoryIcons[i].icon;
            }
        }

        return null;
    }

    private void UpdateAmmoUI()
    {
        if (m_allAmmo == null ||
            m_innerMagazineAmmo == null ||
            m_magazineCapacity == null)
        {
            return;
        }

        UpdateAllAmmoValue();
        UpdateCurrentMagazineAmmoValue();
        UpdateMaxMagazineAmmoValue();
        UpdateMagazineAmmoText();
    }

    private void UpdateAllAmmoValue()
    {
        int value = (int)m_allAmmo.CalculateValue;

        if (m_prevAllAmmoValue == value)
        {
            return;
        }

        m_allAmmoValue = value;
        m_prevAllAmmoValue = value;

        if (m_allAmmoText != null)
        {
            m_allAmmoText.text = m_allAmmoValue.ToString();
        }
    }

    private void UpdateCurrentMagazineAmmoValue()
    {
        int value = (int)m_innerMagazineAmmo.CalculateValue;

        if (m_prevCurrentMagazineAmmoValue == value)
        {
            return;
        }

        m_currentMagazineAmmoValue = value;
        m_prevCurrentMagazineAmmoValue = value;

        m_isMagazineAmmoTextDirty = true;
    }

    private void UpdateMaxMagazineAmmoValue()
    {
        int value = (int)m_magazineCapacity.CalculateValue;

        if (m_prevMaxMagazineAmmoValue == value)
        {
            return;
        }

        m_maxMagazineAmmoValue = value;
        m_prevMaxMagazineAmmoValue = value;

        m_isMagazineAmmoTextDirty = true;
    }

    private void UpdateMagazineAmmoText()
    {
        if (!m_isMagazineAmmoTextDirty)
        {
            return;
        }

        if (m_magazineAmmoText != null)
        {
            m_magazineAmmoText.text = m_currentMagazineAmmoValue.ToString();
        }

        m_isMagazineAmmoTextDirty = false;
    }

    private void UpdateReloadUI()
    {
        bool isReloading = m_gunAction != null && m_gunAction.GetReloading();

        if (m_reloadingPanel != null)
        {
            m_reloadingPanel.gameObject.SetActive(isReloading);
        }

        if (m_allAmmoText != null)
        {
            m_allAmmoText.gameObject.SetActive(!isReloading);
        }

        if (m_magazineAmmoText != null)
        {
            m_magazineAmmoText.gameObject.SetActive(!isReloading);
        }

        if (m_reloadRotateTarget == null)
        {
            return;
        }

        if (isReloading)
        {
            m_reloadRotateTarget.Rotate(
                0.0f,
                0.0f,
                -m_reloadRotateSpeed * Time.deltaTime
            );
        }
        else
        {
            ResetReloadRotation();
        }
    }

    private void SetAmmoUIVisible(bool isVisible)
    {
        if (m_allAmmoText != null)
        {
            m_allAmmoText.gameObject.SetActive(isVisible);
        }

        if (m_magazineAmmoText != null)
        {
            m_magazineAmmoText.gameObject.SetActive(isVisible);
        }

        if (m_reloadingPanel != null)
        {
            m_reloadingPanel.gameObject.SetActive(false);
        }
    }

    private void ResetReloadRotation()
    {
        if (m_reloadRotateTarget != null)
        {
            m_reloadRotateTarget.localRotation = Quaternion.identity;
        }
    }

    private void ToggleWeaponHeaderHighlight()
    {
        m_isWeaponHeader1Active = !m_isWeaponHeader1Active;
        ApplyWeaponHeaderHighlight();
    }

    private void ApplyWeaponHeaderHighlight()
    {
        ApplyWeaponHeader(m_weaponHeader1, m_isWeaponHeader1Active);
        ApplyWeaponHeader(m_weaponHeader2, !m_isWeaponHeader1Active);
    }

    private void ApplyWeaponHeader(Image header, bool isActive)
    {
        if (header == null)
        {
            return;
        }

        Color color = header.color;

        if (isActive)
        {
            color.a = m_headerActiveAlpha;
            header.rectTransform.localScale = m_headerActiveScale;
        }
        else
        {
            color.a = m_headerInactiveAlpha;
            header.rectTransform.localScale = m_headerInactiveScale;
        }

        header.color = color;
    }

    private void ClearWeaponUI()
    {
        m_prevWeapon = null;
        m_currentWeapon = null;

        ClearWeaponParameters();
        ResetAmmoCache();

        if (m_allAmmoText != null)
        {
            m_allAmmoText.text = "0";
        }

        if (m_magazineAmmoText != null)
        {
            m_magazineAmmoText.text = "0";
        }

        SetAmmoUIVisible(false);

        if (m_gunIcon != null)
        {
            m_gunIcon.sprite = m_defaultGunIcon;

            Color iconColor = m_gunIcon.color;
            iconColor.a = 1.0f;
            m_gunIcon.color = iconColor;
        }

        ResetReloadRotation();

        ApplyWeaponHeader(m_weaponHeader1, false);
        ApplyWeaponHeader(m_weaponHeader2, false);
    }

    private void ClearWeaponParameters()
    {
        m_gunAction = null;

        m_allAmmo = null;
        m_innerMagazineAmmo = null;
        m_magazineCapacity = null;
    }

    private void ResetAmmoCache()
    {
        m_allAmmoValue = 0;
        m_currentMagazineAmmoValue = 0;
        m_maxMagazineAmmoValue = 0;

        m_prevAllAmmoValue = -1;
        m_prevCurrentMagazineAmmoValue = -1;
        m_prevMaxMagazineAmmoValue = -1;

        m_isMagazineAmmoTextDirty = true;
    }
}
