using UnityEngine;

/// <summary>
/// ゲーム開始時に表示するUIを管理するクラス。
/// </summary>
public class UI_GameStartUIController : MonoBehaviour
{
    [Header("Goal Message UI")]
    [SerializeField] private UI_GoalMessage m_goalMessageUI;

    [Header("Display Time")]
    [SerializeField] private float m_messageDisplayTime = 3.0f;

    private void Start()
    {
        if (m_goalMessageUI == null)
        {
            return;
        }

        m_goalMessageUI.ShowMessage(m_messageDisplayTime);
    }
}