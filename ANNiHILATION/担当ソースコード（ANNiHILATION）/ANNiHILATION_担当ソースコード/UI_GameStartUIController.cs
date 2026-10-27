// 【ファイルの役割】ゲーム開始時に目標メッセージを一定時間表示する。
// 【依存・注意点】InspectorでUI_GoalMessageを指定する。表示の具体的な制御はメッセージUI側へ委譲する。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

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