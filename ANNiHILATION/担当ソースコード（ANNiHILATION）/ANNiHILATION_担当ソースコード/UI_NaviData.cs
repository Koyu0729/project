// 【ファイルの役割】ナビカーソルの対象、表示状態、サイズ、回転設定を保持するデータ型。
// 【依存・注意点】データ自体は描画しない。UI_GoalNaviが参照して表示を更新する。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using UnityEngine;
public enum NaviCursorType
{
    NaviCursor_NULL = -1,

    Goal,
    Place,
    Enemy,
    Boss,

    NaviCursor_MAX
}

[System.Serializable]
public class NaviData
{
    //ミッションをID管理でやると噛み合う
    [Header("識別ID")]
    public string id;

    [Header("カーソル種類")]
    public NaviCursorType type;

    [Header("追跡対象")]
    public Transform target;

    [Header("カーソル実体")]
    public RectTransform cursor;

    public float rotateOffset = 0.0f;

    [Header("カーソルサイズ")]
    public Vector2 sizeMin = new Vector2(50.0f, 50.0f);
    public Vector2 sizeMax = new Vector2(100.0f, 100.0f);

    [Header("回転するか")]
    public bool rotate = true;

    [Header("有効状態")]
    public bool active = true;
}