// 【ファイルの役割】UIの表示・非表示、Animator制御、画面座標とCanvas座標の変換を共通化する。
// 【依存・注意点】Camera.mainと親Canvasを取得する。非表示アニメーションを使う場合は完了イベントの設定が必要。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using UnityEngine;
using System.Collections;

/// <summary>
/// すべてのUIクラスの基底クラス。
/// カメラ・Canvas参照取得、座標変換、UI表示切替などの共通機能を持つ。
/// </summary>
public class UI_Base : MonoBehaviour
{
    [Header("＊ UIBase＊")]
    [Header("UI / Camera(Base)")]

    [SerializeField, ReadOnly] protected Camera m_mainCamera;

    [SerializeField, ReadOnly] protected Camera m_canvasCamera;

    [SerializeField, ReadOnly] protected Canvas m_canvas;

    [Header("Animation(Base)")]
    [SerializeField, ReadOnly] protected Animator m_animator;
    [SerializeField] protected bool m_useAnimation = true;

    [Header("Animator Trigger Name(Base)")]
    [SerializeField] protected string m_showTriggerName = "show";
    [SerializeField] protected string m_hideTriggerName = "hide";

    private Coroutine m_autoHideCoroutine;

    protected virtual void Awake()
    {
        // 3Dワールド座標をスクリーン座標に変換するためのカメラ。
        // UI_Cameraではなく、ゲーム画面を映しているMainCameraを使う。
        m_mainCamera = Camera.main;

        if (m_canvas == null)
        {
            m_canvas = GetComponentInParent<Canvas>();
        }

        // Canvas座標変換に使うカメラを取得する。
        // Overlayならnull、Screen Space - CameraならCanvasのRender Cameraを使う。
        RefreshCanvasCamera();

        if (m_animator == null)
        {
            m_animator = GetComponent<Animator>();
        }

        if (m_mainCamera == null)
        {
            Debug.LogWarning($"{nameof(UI_Base)} : MainCamera が見つかりませんでした。ゲーム用カメラに MainCamera タグが付いているか確認してください。");
        }

        if (m_canvas == null)
        {
            Debug.LogWarning($"{nameof(UI_Base)} : Canvas が見つかりませんでした。UIがCanvas配下にあるか確認してください。");
        }
    }

    //============================================================
    // Public
    // 外部クラスから呼ぶ用
    //============================================================

    /// <summary>
    /// 外部からUIを表示する。
    /// </summary>
    public void Show()
    {
        ShowSelf();
    }

    /// <summary>
    /// 外部からUIを非表示にする。
    /// 非表示アニメーションを使用する。
    /// </summary>
    public void Hide()
    {
        HideSelf();
    }

    /// <summary>
    /// 外部からUIを即非表示にする。
    /// 非表示アニメーションは再生しない。
    /// </summary>
    public void HideImmediately()
    {
        HideSelfImmediately();
    }

    /// <summary>
    /// 外部からUIを表示し、指定秒数後に非表示にする。
    /// </summary>
    public void ShowForSeconds(float displayTime)
    {
        ShowSelfForSeconds(displayTime);
    }

    //============================================================
    // Protected
    // 継承先クラスから呼ぶ用
    //============================================================

    /// <summary>
    /// CanvasのRenderModeに合わせて、Canvas座標変換用カメラを更新する。
    /// </summary>
    protected void RefreshCanvasCamera()
    {
        m_canvasCamera = null;

        if (m_canvas == null)
        {
            return;
        }

        switch (m_canvas.renderMode)
        {
            case RenderMode.ScreenSpaceOverlay:
                // Overlayはカメラ不要。
                m_canvasCamera = null;
                break;

            case RenderMode.ScreenSpaceCamera:
                // Screen Space - CameraはCanvasのRender Cameraを使う。
                m_canvasCamera = m_canvas.worldCamera;

                if (m_canvasCamera == null)
                {
                    Debug.LogWarning($"{nameof(UI_Base)} : Canvas が Screen Space - Camera ですが Render Camera が設定されていません。Canvas の Render Camera に UI_Camera を設定してください。");
                }

                break;

            case RenderMode.WorldSpace:
                // World SpaceもCanvasのEvent Camera / World Cameraを使う。
                m_canvasCamera = m_canvas.worldCamera;

                if (m_canvasCamera == null)
                {
                    Debug.LogWarning($"{nameof(UI_Base)} : Canvas が World Space ですが World Camera が設定されていません。");
                }

                break;
        }
    }

    /// <summary>
    /// Vector3 の X,Y を Vector2 に変換する。
    /// </summary>
    protected Vector2 Vec3ToVec2(Vector3 value)
    {
        return new Vector2(value.x, value.y);
    }

    /// <summary>
    /// ワールド座標をスクリーン座標へ変換する。
    /// ここは必ず、3Dワールドを映しているMainCamera基準で計算する。
    /// </summary>
    protected Vector3 WorldToScreen(Vector3 worldPos)
    {
        if (m_mainCamera == null)
        {
            return Vector3.zero;
        }

        return m_mainCamera.WorldToScreenPoint(worldPos);
    }

    /// <summary>
    /// ワールド座標をスクリーン座標(Vector2)へ変換する。
    /// </summary>
    protected Vector2 WorldToScreen2D(Vector3 worldPos)
    {
        Vector3 screenPos = WorldToScreen(worldPos);
        return Vec3ToVec2(screenPos);
    }

    /// <summary>
    /// スクリーン座標をCanvas内ローカル座標へ変換する。
    /// OverlayならCameraはnull。
    /// Screen Space - CameraならCanvasのRender Cameraを使う。
    /// </summary>
    protected Vector2 ScreenToCanvasLocalPoint(Vector2 screenPos)
    {
        if (m_canvas == null)
        {
            return Vector2.zero;
        }

        // 実行中にCanvas設定が変わった場合にも対応できるように更新する。
        RefreshCanvasCamera();

        RectTransform canvasRect = m_canvas.GetComponent<RectTransform>();

        RectTransformUtility.ScreenPointToLocalPointInRectangle(
            canvasRect,
            screenPos,
            m_canvasCamera,
            out Vector2 localPoint
        );

        return localPoint;
    }

    /// <summary>
    /// スクリーン座標を、指定したUIの anchoredPosition に代入できる座標へ変換する。
    /// target の親 RectTransform を基準に変換する。
    /// </summary>
    protected bool TryScreenToAnchoredPosition(RectTransform target, Vector2 screenPos, out Vector2 anchoredPosition)
    {
        anchoredPosition = Vector2.zero;

        if (target == null)
        {
            return false;
        }

        RectTransform parentRect = target.parent as RectTransform;

        if (parentRect == null)
        {
            return false;
        }

        RefreshCanvasCamera();

        return RectTransformUtility.ScreenPointToLocalPointInRectangle(
            parentRect,
            screenPos,
            m_canvasCamera,
            out anchoredPosition
        );
    }

    /// <summary>
    /// 自分自身のUIを表示する。
    /// Animatorがある場合は show アニメーションを再生する。
    /// </summary>
    protected void ShowSelf()
    {
        if (!gameObject.activeSelf)
        {
            gameObject.SetActive(true);
        }

        PlayShowAnimation();
    }

    /// <summary>
    /// 自分自身のUIを非表示にする。
    /// Animatorがある場合は hide アニメーションを再生する。
    /// Animatorがない場合は即非表示。
    /// </summary>
    protected void HideSelf()
    {
        HideSelf(true);
    }

    /// <summary>
    /// 自分自身のUIを即非表示にする。
    /// 非表示アニメーションは再生しない。
    /// </summary>
    protected void HideSelfImmediately()
    {
        StopAutoHide();

        gameObject.SetActive(false);
    }

    /// <summary>
    /// 自分自身のUIを表示し、指定秒数後に非表示にする。
    /// </summary>
    protected void ShowSelfForSeconds(float displayTime)
    {
        ShowSelf();
        HideSelfAfterDelay(displayTime);
    }

    /// <summary>
    /// 表示アニメーションを再生する。
    /// </summary>
    protected void PlayShowAnimation()
    {
        if (!m_useAnimation)
        {
            return;
        }

        if (m_animator == null)
        {
            return;
        }

        m_animator.ResetTrigger(m_hideTriggerName);
        m_animator.SetTrigger(m_showTriggerName);
    }

    /// <summary>
    /// 非表示アニメーションを再生する。
    /// </summary>
    protected void PlayHideAnimation()
    {
        if (!m_useAnimation)
        {
            return;
        }

        if (m_animator == null)
        {
            return;
        }

        m_animator.ResetTrigger(m_showTriggerName);
        m_animator.SetTrigger(m_hideTriggerName);
    }

    /// <summary>
    /// アニメーションのTriggerをリセットする。
    /// </summary>
    protected void ResetAnimationTriggers()
    {
        if (m_animator == null)
        {
            return;
        }

        m_animator.ResetTrigger(m_showTriggerName);
        m_animator.ResetTrigger(m_hideTriggerName);
    }

    /// <summary>
    /// Animatorを止める。
    /// </summary>
    protected void StopAnimation()
    {
        if (m_animator == null)
        {
            return;
        }

        ResetAnimationTriggers();
    }

    /// <summary>
    /// 指定秒数後に自分自身のUIを非表示にする。
    /// </summary>
    protected void HideSelfAfterDelay(float delayTime)
    {
        StopAutoHide();

        m_autoHideCoroutine = StartCoroutine(HideSelfAfterDelayCoroutine(delayTime));
    }

    /// <summary>
    /// 自動非表示処理を止める。
    /// </summary>
    protected void StopAutoHide()
    {
        if (m_autoHideCoroutine != null)
        {
            StopCoroutine(m_autoHideCoroutine);
            m_autoHideCoroutine = null;
        }
    }

    /// <summary>
    /// 指定UIを表示する。
    /// 子UIやナビカーソルなど、任意のRectTransform表示用。
    /// </summary>
    protected void ShowUI(RectTransform target)
    {
        if (target != null && !target.gameObject.activeSelf)
        {
            target.gameObject.SetActive(true);
        }
    }

    /// <summary>
    /// 指定UIを非表示にする。
    /// 子UIやナビカーソルなど、任意のRectTransform非表示用。
    /// </summary>
    protected void HideUI(RectTransform target)
    {
        if (target != null && target.gameObject.activeSelf)
        {
            target.gameObject.SetActive(false);
        }
    }

    //============================================================
    // Private
    // UIBase内部だけで使う処理
    //============================================================

    private void HideSelf(bool stopAutoHide)
    {
        if (stopAutoHide)
        {
            StopAutoHide();
        }

        if (m_useAnimation && m_animator != null)
        {
            PlayHideAnimation();
        }
        else
        {
            gameObject.SetActive(false);
        }
    }

    private IEnumerator HideSelfAfterDelayCoroutine(float delayTime)
    {
        yield return new WaitForSeconds(delayTime);

        m_autoHideCoroutine = null;

        HideSelf(false);
    }

    //============================================================
    // Animation Event
    // Animation Clip側から呼ぶ用
    //============================================================

    /// <summary>
    /// Animation Eventから呼ぶ。
    /// 非表示アニメーション終了時に、ゲームオブジェクトを非アクティブにする。
    /// </summary>
    public virtual void OnHideAnimationFinished()
    {
        gameObject.SetActive(false);
    }

    //============================================================
    // プロパティ関数
    //============================================================

    protected Camera MainCamera
    {
        get { return m_mainCamera; }
    }

    protected Camera CanvasCamera
    {
        get { return m_canvasCamera; }
    }

    protected Canvas UICanvas
    {
        get { return m_canvas; }
    }

    protected Animator Animator
    {
        get { return m_animator; }
    }

    protected bool UseAnimation
    {
        get { return m_useAnimation; }
    }
}