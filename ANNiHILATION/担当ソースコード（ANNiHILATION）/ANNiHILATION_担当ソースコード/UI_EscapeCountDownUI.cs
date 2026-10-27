// 【ファイルの役割】ゴールエリアで脱出までの残り秒数を表示する。
// 【依存・注意点】GoalAreaがカウント中のときだけTextを有効にする。クラス名はUI_EscapeCountDownで、ファイル名と異なる。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using TMPro;
using UnityEngine;

public class UI_EscapeCountDown : UI_Base
{
    [Header("＊ EscapeCountDownUI＊")]
    [Header("参照")]
    [SerializeField] private GoalArea m_goalArea;

    [Header("Text")]
    [SerializeField, ReadOnly] private TextMeshProUGUI m_countText;

    [Header("表示設定")]
    [SerializeField] private string m_format = "脱出まで {0:F1} 秒";

    protected override void Awake()
    {
        base.Awake();

        if (m_countText == null)
        {
            m_countText = GetComponentInChildren<TextMeshProUGUI>(true);
        }

        if (m_countText != null)
        {
            m_countText.enabled = false;
        }

        m_goalArea = FindFirstObjectByType<GoalArea>();

        if (m_goalArea == null)
        {
            Debug.LogWarning($"{nameof(UI_EscapeCountDown)} : GoalArea が見つかりませんでした。");
        }
    }

    private void Update()
    {
        if (m_goalArea == null)
        {
            return;
        }

        if (m_countText == null)
        {
            return;
        }

        if (!m_goalArea.IsCounting)
        {
            if (m_countText.enabled)
            {
                m_countText.enabled = false;
            }
            return;
        }

        float remainingTime = m_goalArea.GetRemainingTime();

        if (!m_countText.enabled)
        {
            m_countText.enabled = true;
        }

        m_countText.text = string.Format(m_format, remainingTime);
    }

}


