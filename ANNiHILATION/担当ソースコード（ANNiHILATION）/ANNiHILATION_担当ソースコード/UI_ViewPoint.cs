// 【ファイルの役割】視点を示すワールド座標にUIカーソルを追従させ、方向に合わせて回転する。
// 【依存・注意点】PlayerViewPointer、所有プレイヤー、ゲーム用カメラとCanvasを参照する。ポーズ中の表示も制御する。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using UnityEngine;

/// <summary>
/// ビューポイント照準UI
/// 生成された PlayerViewPoint の位置にカーソルUIを表示する。
/// ポーズ中はカーソル更新しない。
/// </summary>
public class UI_ViewPoint : UI_PlayerBase
{
    [Header("カーソル実体")]
    [SerializeField] private RectTransform cursor;

    [Header("ビューポイント参照")]
    [SerializeField] private PlayerViewPointer playerViewPoint;

    [Header("Pause")]
    [SerializeField] private bool m_hideCursorWhenPause = true;

    protected override void Awake()
    {
        base.Awake();

        // Inspectorで入っていなければ、シーン上から自動取得
        if (playerViewPoint == null)
        {
            playerViewPoint = FindAnyObjectByType<PlayerViewPointer>();
        }

        if (playerViewPoint != null)
        {
            Debug.Log($"PlayerViewPoint Position : {playerViewPoint.transform.position}");
        }
    }

    private void LateUpdate()
    {
        if (IsPause())
        {
            SetCursorVisible(false);
            return;
        }

        SetCursorVisible(true);

        if (m_mainCamera == null || !HasOwner() || m_canvas == null)
        {
            return;
        }

        if (cursor == null)
        {
            return;
        }

        // PlayerViewPoint(Clone) が Awake より後に生成される場合に備えて探し直す
        if (playerViewPoint == null)
        {
            playerViewPoint = FindAnyObjectByType<PlayerViewPointer>();

            if (playerViewPoint == null)
            {
                return;
            }
        }

        UpdateViewPointCursor();
    }

    private bool IsPause()
    {
        if (GameManager.Instance == null)
        {
            return false;
        }

        return GameManager.Instance.IsPause;
    }

    private void SetCursorVisible(bool visible)
    {
        if (!m_hideCursorWhenPause)
        {
            return;
        }

        if (cursor == null)
        {
            return;
        }

        GameObject cursorObject = cursor.gameObject;

        if (cursorObject.activeSelf == visible)
        {
            return;
        }

        cursorObject.SetActive(visible);
    }

    private void UpdateViewPointCursor()
    {
        Transform playerTransform = PlayerTransform;

        if (playerTransform == null)
        {
            return;
        }

        // Player View Point(Clone) の Transform Position を使う
        Vector3 viewPointWorldPos = playerViewPoint.transform.position;

        // PlayerViewPoint のワールド座標をスクリーン座標へ変換
        Vector3 targetScreenPos3D = WorldToScreen(viewPointWorldPos);

        // プレイヤーのスクリーン座標
        Vector2 playerScreenPos = WorldToScreen2D(playerTransform.position);

        // PlayerViewPoint のスクリーン座標を Vector2 に変換
        Vector2 targetScreenPos = Vec3ToVec2(targetScreenPos3D);

        // カメラの後ろにある場合の補正
        if (targetScreenPos3D.z < 0.0f)
        {
            targetScreenPos.x = Screen.width - targetScreenPos.x;
            targetScreenPos.y = Screen.height - targetScreenPos.y;
        }

        // プレイヤーから PlayerViewPoint への画面上の方向ベクトル
        Vector2 dir = targetScreenPos - playerScreenPos;

        //if (dir.sqrMagnitude <= 0.0001f)
        if (dir.sqrMagnitude <= 0.5f)
        {
            return;
        }

        dir.Normalize();

        // スクリーン座標を cursor.anchoredPosition 用の座標へ変換
        if (!TryScreenToAnchoredPosition(cursor, targetScreenPos, out Vector2 localPoint))
        {
            return;
        }

        // Player View Point(Clone) の位置に UI を置く
        cursor.anchoredPosition = localPoint;

        // カーソルを PlayerViewPoint 方向へ向ける
        float angle = Mathf.Atan2(dir.y, dir.x) * Mathf.Rad2Deg;

        cursor.rotation = Quaternion.Euler(0.0f, 0.0f, angle + 90.0f);
    }
}