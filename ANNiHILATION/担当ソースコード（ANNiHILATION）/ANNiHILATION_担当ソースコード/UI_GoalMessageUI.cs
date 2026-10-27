// 【ファイルの役割】目標メッセージを表示し、指定時間後に非表示へ移行する。
// 【依存・注意点】待ち時間はTime.timeScaleの影響を受ける。クラス名はUI_GoalMessageで、ファイル名と異なる。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using System.Collections;
using TMPro;
using UnityEngine;

/// <summary>
/// 画面中央上部に「ゴールに向かえ」などの目標メッセージを表示するUI。
/// </summary>
public class UI_GoalMessage : UI_Base
{
    [Header("＊ GoalMessageUI＊")]
    [Header("Image")]
    [SerializeField, ReadOnly] private TextMeshProUGUI m_messageText;

    [Header("Message")]
    [SerializeField] private string m_defaultMessage = "ゴールに向かえ";

    private Coroutine m_showCoroutine;

    protected override void Awake()
    {
        base.Awake();

        if (m_animator == null)
        {
            m_animator = GetComponent<Animator>();
        }
        m_messageText = GetComponentInChildren<TextMeshProUGUI>(true);

        SetText(m_defaultMessage);

        gameObject.SetActive(false);
    }

    //インスペクター用の表示関数。引数なしで呼び出された場合は、デフォルトのメッセージを表示する。
    public void ShowMessage(float displayTime)
    {
        ShowMessage(m_defaultMessage, displayTime);
    }

    //プログラム用の表示関数。引数なしで呼び出された場合は、デフォルトのメッセージを表示する。
    public void ShowMessage(string message, float displayTime)
    {
        gameObject.SetActive(true);

        SetText(message);

        if (m_showCoroutine != null)
        {
            StopCoroutine(m_showCoroutine);
            m_showCoroutine = null;
        }

        m_showCoroutine = StartCoroutine(ShowMessageCoroutine(displayTime));
    }

    /// <summary>
    /// 表示中の文字だけを変更する。
    /// デフォルトメッセージは変更しない。
    /// </summary>
    public void SetText(string message)
    {
        if (m_messageText != null)
        {
            m_messageText.text = message;
        }
    }

    /// <summary>
    /// デフォルトメッセージを変更する。
    /// ShowMessage(float displayTime)で表示される文字も変わる。
    /// </summary>
    public void SetDefaultMessage(string message)
    {
        m_defaultMessage = message;
        SetText(message);
    }

    // メッセージを表示してから一定時間後に非表示にするコルーチン
    private IEnumerator ShowMessageCoroutine(float displayTime)
    {
        if (m_useAnimation && m_animator != null)
        {
            m_animator.ResetTrigger(m_hideTriggerName);
            m_animator.SetTrigger(m_showTriggerName);
        }

        yield return new WaitForSeconds(displayTime);

        if (m_useAnimation && m_animator != null)
        {
            m_animator.ResetTrigger(m_showTriggerName);
            m_animator.SetTrigger(m_hideTriggerName);
        }
        else
        {
            gameObject.SetActive(false);
        }

        m_showCoroutine = null;
    }

    /// <summary>
    /// 表示中のメッセージを即停止して非表示にする。
    /// アニメーションは再生しない。
    /// </summary>
    public void StopMessage()
    {
        if (m_showCoroutine != null)
        {
            StopCoroutine(m_showCoroutine);
            m_showCoroutine = null;
        }

        gameObject.SetActive(false);
    }
}