using UnityEngine;

/// <summary>
/// プレイヤーUI用の基底クラス
/// </summary>
public class UI_PlayerBase : UI_Base
{
    [Header("＊ PlayerUIBase＊")]
    [Header("In Game(Base)")]
    [SerializeField, ReadOnly] protected CharacterData m_playerOwner;

    protected Transform PlayerTransform
    {
        get
        {
            if (m_playerOwner == null)
            {
                return null;
            }

            return m_playerOwner.transform;
        }
    }

    protected override void Awake()
    {
        base.Awake();

        GameObject playerObj = GameObject.FindGameObjectWithTag("Player");

        if (playerObj == null)
        {
            Debug.LogWarning($"{nameof(UI_PlayerBase)} : Playerタグのオブジェクトが見つかりませんでした。");
            return;
        }

        m_playerOwner = playerObj.GetComponent<CharacterData>();

        if (m_playerOwner == null)
        {
            Debug.LogWarning($"{nameof(UI_PlayerBase)} : PlayerタグのオブジェクトにCharacterDataが見つかりませんでした。");
            return;
        }
    }

    protected bool HasOwner()
    {
        return m_playerOwner != null;
    }
}
