// 【ファイルの役割】プレイヤーを参照するUIへ、所有者とTransformの共通アクセスを提供する。
// 【依存・注意点】AwakeでPlayerタグを検索する。後からの生成や所有者の変更には自動で追従しない。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

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
