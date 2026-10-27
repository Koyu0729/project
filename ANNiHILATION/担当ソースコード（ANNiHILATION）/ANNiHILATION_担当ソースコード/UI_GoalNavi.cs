// 【ファイルの役割】画面外の目標へ向かうナビカーソルを画面端に配置する。
// 【依存・注意点】最初の登録対象はシーン内のGoalAreaで上書きする。登録時にはNaviDataのcursor参照も変更する。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// 目的地ナビゲーションシステム
/// </summary>
public class UI_GoalNavi : UI_PlayerBase
{
    [Header("現在進行中のナビ")]
    [SerializeField] private List<NaviData> m_naviData;

    [Header("画面端の余白")]
    [SerializeField] private float m_screenEdgePadding = 50f;

    [Header("カーソルのサイズ縮小限界距離")]
    [SerializeField] private float m_maxDistance = 100.0f;

    protected override void Awake()
    {
        base.Awake();
    }

    private void Start()
    {
        if (m_naviData == null)
        {
            m_naviData = new List<NaviData>();
        }

        if (m_naviData.Count <= 0)
        {
            Debug.LogWarning("NaviDataの0番目がありません");
            return;
        }

        if (m_naviData[0] == null)
        {
            Debug.LogWarning("NaviDataの0番目がnullです");
            return;
        }

        GoalArea goalArea = FindAnyObjectByType<GoalArea>();

        if (goalArea == null)
        {
            Debug.LogWarning("GoalArea が見つかりません");
            return;
        }

        m_naviData[0].target = goalArea.transform;
        m_naviData[0].active = true;
    }

    private void Update()
    {
        if (m_mainCamera == null || !HasOwner() || m_naviData == null || m_canvas == null)
        {
            return;
        }

        
        for (int i = 0; i < m_naviData.Count; i++)
        {
            NaviData data = m_naviData[i];

            if (data == null || data.target == null || data.cursor == null)
            {
                continue;
            }

            if (!data.active)
            {
                HideUI(data.cursor); 
                continue;
            }
            UpdateNaviCursor(data);
        }
    }

    private void UpdateNaviCursor(NaviData data)
    {
        Transform playerTransform = PlayerTransform;

        if (playerTransform == null)
        {
            return;
        }

        //ターゲットのスクリーン座標
        Vector3 targetScreenPos3D = WorldToScreen(data.target.position);

        //---------------------------------------------------------------
        //ターゲットが今、画面に見えているか？によって、カーソルを消す
        //---------------------------------------------------------------
        bool isInFrontOfCamera = targetScreenPos3D.z > 0f;

        //全部の条件を比較したときにあってればいいので実際、括弧は必要ない
        bool isInsideScreen =
            (targetScreenPos3D.x >= 0f && targetScreenPos3D.x <= Screen.width) &&
            (targetScreenPos3D.y >= 0f && targetScreenPos3D.y <= Screen.height);

        //画面に見えているならば、ナビカーソルを非表示
        if (isInFrontOfCamera && isInsideScreen)
        {
            HideUI(data.cursor);
            //見えない状態の時は、Transformの計算処理が必要ないので、return
            return;
        }
        //ナビカーソルが非表示なら、ナビカーソルを表示
        ShowUI(data.cursor);

        //----------------------------------------------------------------------
        //ナビカーソルのTransform変換用の計算処理
        //----------------------------------------------------------------------

        //プレイヤーの3D座標を、画面上の2D位置に変換
        Vector2 playerScreenPos = WorldToScreen2D(playerTransform.position);

        //ターゲットのスクリーン座標の2次元座標が欲しいので、奥行きを消して、Vector2に変換
        Vector2 targetScreenPos = Vec3ToVec2(targetScreenPos3D);

        //今後カメラ角度や視点変更をした時、バグる可能性があるので記述
        if (targetScreenPos3D.z < 0f)
        {
            targetScreenPos.x = Screen.width - targetScreenPos.x;
            targetScreenPos.y = Screen.height - targetScreenPos.y;
        }

        //方向ベクトルが差分ゼロで方向が作れない場合の保険
        Vector2 dir = (targetScreenPos - playerScreenPos).normalized;
        if (dir == Vector2.zero)
        {
            return;
        }
        
        //ゲーム画面の一回り小さい、短形当たり判定（座標定義っていうのが正しいかも）
        float minX = m_screenEdgePadding;
        float maxX = Screen.width - m_screenEdgePadding;
        float minY = m_screenEdgePadding;
        float maxY = Screen.height - m_screenEdgePadding;

        //方向ベクトルの壁まで進む量の計算結果保持用
        float toEdgeX = 0.0f;
        float toEdgeY = 0.0f;

        //if(方向ベクトルを絶対値に変換した変数（dir.x）が0と近くないなら)
        if (Mathf.Abs(dir.x) > 0.0001f)
        {
            if (dir.x > 0f)
            {
                // 右方向なら右端まで
                toEdgeX = (maxX - playerScreenPos.x) / dir.x;
            }
            else
            {
                // 左方向なら左端まで
                toEdgeX = (minX - playerScreenPos.x) / dir.x;
            }
        }
        else
        {
            toEdgeX = float.MaxValue;
        }

        //if(方向ベクトルを絶対値に変換した変数（dir.y）が0と近くないなら)
        if (Mathf.Abs(dir.y) > 0.0001f)
        {
            if (dir.y > 0f)
            {
                // 下方向ならした下端まで
                toEdgeY = (maxY - playerScreenPos.y) / dir.y;
            }
            else
            {
                // 上方向なら上端まで
                toEdgeY = (minY - playerScreenPos.y) / dir.y;
            }
        }
        else
        {
            toEdgeY = float.MaxValue;
        }

        //X,Y、先に当たる辺までの進行量を取得
        float t = Mathf.Min(toEdgeX, toEdgeY);

        //プレイヤー位置からt進めたナビカーソル位置
        Vector2 navPos = playerScreenPos + dir * t;

        //-------------------------------------------------------------------------
        //座標変換
        //-------------------------------------------------------------------------

        //スクリーン座標（モニターの座標）からキャンバスのローカル座標に変換
        if (!TryScreenToAnchoredPosition(data.cursor, navPos, out Vector2 localPoint))
        {
            return;
        }

        //座標代入　＋　補間（毎フレーム目標座標急に飛ばしていたので、なめらかに近づけるように線形補間する）
        data.cursor.anchoredPosition = Vector2.Lerp(data.cursor.anchoredPosition, localPoint, 20f * Time.deltaTime);

        //----------------------
        //サイズ
        //----------------------
        float distance = Vector3.Distance(playerTransform.position, data.target.position);
        float m = Mathf.InverseLerp(0.0f, m_maxDistance, distance);

        data.cursor.sizeDelta = Vector2.Lerp(data.sizeMax, data.sizeMin, m);

        //----------------------
        //回転
        //----------------------
        if (data.rotate)
        {
            float angle = Mathf.Atan2(dir.y, dir.x) * Mathf.Rad2Deg;
            data.cursor.rotation = Quaternion.Euler(0f, 0f, data.rotateOffset + angle);
        }
    }

    //ランタイム中に新しいミッションを追加するときに使って下さい
    public void AddNavi(NaviData data)
    {
        if (data == null)
        {
            return;
        }

        if (data.target == null || data.cursor == null)
        {
            Debug.LogWarning("NaviData不足");
            return;
        }

        RectTransform instance = Instantiate(data.cursor, m_canvas.transform);

        instance.gameObject.SetActive(false);

        data.cursor = instance;

        m_naviData.Add(data);
    }
    
    //指定したIDのナビゲーションを消す
    public void RemoveNavi(string id)
    {
        for (int i = m_naviData.Count - 1; i >= 0; i--)
        {
            if (m_naviData[i].id == id)
            {
                if (m_naviData[i].cursor != null)
                {
                    Destroy(m_naviData[i].cursor.gameObject);
                }

                m_naviData.RemoveAt(i);
            }
        }
    }
}