// 【ファイルの役割】ゲームの制限時間を取得し、秒数または分と秒で表示する。
// 【依存・注意点】Textがなければ処理を中断し、GameStateがない場合は未取得表示とする。初期値で更新を止めない。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using TMPro;
using UnityEngine;

public class UI_TimeLimit : UI_Base
{
    [SerializeField, ReadOnly] private TextMeshProUGUI m_timeText;
    [SerializeField, ReadOnly] private float m_timeLimit = -1.0f;

    [SerializeField] private bool m_byMinute;
    private void Start()
    {
        m_timeText = GetComponentInParent<TextMeshProUGUI>();
    }

    private void Update()
    {
        if (m_timeText == null)
        {
            return;
        }

        if (GameState.Instance == null)
        {
            m_timeText.text = $"∞";
            return;
        }

        m_timeLimit = Mathf.Max(GameState.Instance.TimeLimit, 0.0f);

        if (m_byMinute)
        {
            int minutes = Mathf.FloorToInt(m_timeLimit / 60.0f);
            int seconds = Mathf.FloorToInt(m_timeLimit % 60.0f);

            m_timeText.text = $"{minutes:00}:{seconds:00}";
        }
        else 
        {
            int seconds = Mathf.FloorToInt(m_timeLimit);

            m_timeText.text = seconds.ToString();
        }
    }
}
