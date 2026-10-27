using UnityEngine;

/// <summary>
/// エイムポイント照準UI
/// 生成された AimPoint の位置にカーソルUIを表示する。
/// ポーズ中はカーソル更新しない。
/// </summary>
public class UI_AimPoint : UI_PlayerBase
{
    [Header("カーソル実体")]
    [SerializeField] private RectTransform cursor;

    [Header("エイムポイント参照")]
    [SerializeField] private AimPoint playerAimPoint;

    [Header("Pause")]
    [SerializeField] private bool m_hideCursorWhenPause = true;

    private Transform aimPointTransform;

    [Header("Pause")]
    [SerializeField] bool m_rotate = true;

    protected override void Awake()
    {
        base.Awake();

        FindAimPoint();
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

        if (playerAimPoint == null || aimPointTransform == null)
        {
            FindAimPoint();

            if (playerAimPoint == null || aimPointTransform == null)
            {
                return;
            }
        }

        UpdateAimPointCursor();
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

    private void FindAimPoint()
    {
        playerAimPoint = FindAnyObjectByType<AimPoint>();

        if (playerAimPoint == null)
        {
            aimPointTransform = null;
            return;
        }

        aimPointTransform = playerAimPoint.transform;

        Debug.Log($"UI_AimPoint : AimPointを取得しました。Position = {aimPointTransform.position}");
    }

    private void UpdateAimPointCursor()
    {
        Transform playerTransform = PlayerTransform;

        if (playerTransform == null)
        {
            return;
        }

        // Aim Point(Clone) の現在の Transform Position を使う
        Vector3 aimPointWorldPos = aimPointTransform.position;

        // AimPoint のワールド座標をスクリーン座標へ変換
        Vector3 targetScreenPos3D = WorldToScreen(aimPointWorldPos);

        // プレイヤーのスクリーン座標
        Vector2 playerScreenPos = WorldToScreen2D(playerTransform.position);

        // AimPoint のスクリーン座標を Vector2 に変換
        Vector2 targetScreenPos = Vec3ToVec2(targetScreenPos3D);

        // カメラの後ろにある場合の補正
        if (targetScreenPos3D.z < 0.0f)
        {
            targetScreenPos.x = Screen.width - targetScreenPos.x;
            targetScreenPos.y = Screen.height - targetScreenPos.y;
        }

        // プレイヤーから AimPoint への画面上の方向ベクトル
        Vector2 dir = targetScreenPos - playerScreenPos;

        if (dir.sqrMagnitude <= 0.0001f)
        {
            return;
        }

        dir.Normalize();

        // スクリーン座標を cursor.anchoredPosition 用の座標へ変換
        if (!TryScreenToAnchoredPosition(cursor, targetScreenPos, out Vector2 localPoint))
        {
            return;
        }

        // AimPoint(Clone) の位置に UI を置く
        cursor.anchoredPosition = localPoint;

        if (m_rotate)
        {
            // カーソルを AimPoint 方向へ向ける
            float angle = Mathf.Atan2(dir.y, dir.x) * Mathf.Rad2Deg;

            cursor.rotation = Quaternion.Euler(0.0f, 0.0f, angle + 90.0f);
        }
    }
}