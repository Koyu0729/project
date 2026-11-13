using UnityEngine;
using UnityEngine.UI;

public class UI_Ability : UI_PlayerBase
{
    [System.Serializable]
    private class AbilityIcon
    {
        public AbilityDataTable dataTable;
        public Sprite icon;
    }

    [Header("Component")]
    [SerializeField] private Image[] m_abilityIcons = new Image[3];

    [Header("Icon")]
    [SerializeField] private Sprite m_defaultIcon;
    [SerializeField] private AbilityIcon[] m_iconTables;

    [Header("Display")]
    [SerializeField] private bool m_hideEmptySlot = false;

    [Header("In Game")]
    [SerializeField, ReadOnly] private AbilityData[] m_displayAbilities = new AbilityData[3];

    private AbilityData[] m_prevAbilities = new AbilityData[3];

    private void Start()
    {
        Refresh(true);
    }

    private void Update()
    {
        Refresh(false);
    }

    private void Refresh(bool isForce)
    {
        if (m_abilityIcons == null)
        {
            return;
        }

        int slotCount = m_abilityIcons.Length;
        if (m_displayAbilities == null || m_displayAbilities.Length != slotCount ||
            m_prevAbilities == null || m_prevAbilities.Length != slotCount)
        {
            m_displayAbilities = new AbilityData[slotCount];
            m_prevAbilities = new AbilityData[slotCount];
            isForce = true;
        }

        bool isChanged = isForce;

        for (int i = 0; i < m_abilityIcons.Length; i++)
        {
            AbilityData ability = GetAbility(i);

            m_displayAbilities[i] = ability;

            if (m_prevAbilities[i] != ability)
            {
                isChanged = true;
            }
        }

        if (!isChanged)
        {
            return;
        }

        for (int i = 0; i < m_abilityIcons.Length; i++)
        {
            m_prevAbilities[i] = m_displayAbilities[i];
            ApplyIcon(i, m_displayAbilities[i]);
        }
    }

    private AbilityData GetAbility(int index)
    {
        if (!HasOwner())
        {
            return null;
        }

        if (m_playerOwner.AbilityHolder == null)
        {
            return null;
        }

        if (m_playerOwner.AbilityHolder.Objects == null)
        {
            return null;
        }

        if (index < 0 || index >= m_playerOwner.AbilityHolder.Objects.Length)
        {
            return null;
        }

        return m_playerOwner.AbilityHolder.Objects[index];
    }
    private void ApplyIcon(int index, AbilityData ability)
    {
        if (index < 0 || index >= m_abilityIcons.Length)
        {
            return;
        }

        Image iconImage = m_abilityIcons[index];

        if (iconImage == null)
        {
            return;
        }

        Sprite icon = null;

        if (ability != null)
        {
            icon = GetIcon(ability.DataTable);
        }

        if (icon == null)
        {
            icon = m_defaultIcon;
        }

        iconImage.sprite = icon;

        if (m_hideEmptySlot)
        {
            iconImage.gameObject.SetActive(ability != null);
            return;
        }

        iconImage.gameObject.SetActive(true);
        iconImage.enabled = icon != null;

        Color iconColor = iconImage.color;
        iconColor.a = 1.0f;
        iconImage.color = iconColor;
    }


    private Sprite GetIcon(AbilityDataTable dataTable)
    {
        if (dataTable == null || m_iconTables == null)
        {
            return null;
        }

        for (int i = 0; i < m_iconTables.Length; i++)
        {
            if (m_iconTables[i] == null)
            {
                continue;
            }

            if (m_iconTables[i].dataTable == dataTable)
            {
                return m_iconTables[i].icon;
            }
        }

        for (int i = 0; i < m_iconTables.Length; i++)
        {
            if (m_iconTables[i] == null || m_iconTables[i].dataTable == null)
            {
                continue;
            }

            if (m_iconTables[i].dataTable.GetType() == dataTable.GetType())
            {
                return m_iconTables[i].icon;
            }
        }

        return null;
    }
}