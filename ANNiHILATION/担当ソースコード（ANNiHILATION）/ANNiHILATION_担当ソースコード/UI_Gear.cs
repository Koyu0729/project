// 【ファイルの役割】装備ギアの先頭スロットに対応するアイコンを表示する。
// 【依存・注意点】ギアが変更されたときに表示を更新する。複数スロット表示には拡張が必要。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using UnityEngine;
using UnityEngine.UI;

public class UI_Gear : UI_PlayerBase
{
    [System.Serializable]
    private class GearIcon
    {
        public GearDataTable dataTable;
        public Sprite icon;
    }

    [Header("Component")]
    [SerializeField] private Image m_gearIcon;

    [Header("Icon")]
    [SerializeField] private Sprite m_defaultIcon;
    [SerializeField] private GearIcon[] m_iconTables;

    [Header("In Game")]
    [SerializeField, ReadOnly] private GearData m_displayGear;

    private GearData m_prevGear;

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
        GearData gear = GetGear();

        m_displayGear = gear;

        if (!isForce && m_prevGear == gear)
        {
            return;
        }

        m_prevGear = gear;

        ApplyIcon(gear);
    }

    private GearData GetGear()
    {
        if (!HasOwner())
        {
            return null;
        }

        if (m_playerOwner.GearHolder == null)
        {
            return null;
        }

        if (m_playerOwner.GearHolder.Objects == null)
        {
            return null;
        }

        if (m_playerOwner.GearHolder.Objects.Length <= 0)
        {
            return null;
        }

        return m_playerOwner.GearHolder.Objects[0];
    }

    private void ApplyIcon(GearData gear)
    {
        if (m_gearIcon == null)
        {
            return;
        }

        Sprite icon = null;

        if (gear != null)
        {
            icon = GetIcon(gear.DataTable);
        }

        if (icon == null)
        {
            icon = m_defaultIcon;
        }

        m_gearIcon.sprite = icon;
        m_gearIcon.enabled = icon != null;

        Color iconColor = m_gearIcon.color;
        iconColor.a = 1.0f;
        m_gearIcon.color = iconColor;
    }

    private Sprite GetIcon(GearDataTable dataTable)
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
            if (m_iconTables[i] == null ||
                m_iconTables[i].dataTable == null)
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