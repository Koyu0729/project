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
            m_timeText.text = $"Åá";
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
