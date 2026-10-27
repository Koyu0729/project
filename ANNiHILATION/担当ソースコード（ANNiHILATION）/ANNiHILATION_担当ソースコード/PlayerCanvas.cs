// 【ファイルの役割】プレイヤーCanvas用のクラス枠を保持する。
// 【依存・注意点】現在のStartとUpdateに有効な処理はない。UI_PlayerBaseから継承する初期化処理は実行される。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using UnityEngine;

public class PlayerCanvas :UI_PlayerBase
{
    //[Header("Component")]
    //[SerializeField] private WeaponDisplay[] m_weaponDisplay = new WeaponDisplay[2];
    protected override void Awake()
    {
        base.Awake();
    }

    void Start()
    {
        //if (m_weaponDisplay[0] != null)
        //{
        //    m_weaponDisplay[0].SetData(m_characterEquipment.Guns[0]);
        //}

        //if (m_weaponDisplay[1] != null)
        //{
        //    m_weaponDisplay[1].SetData(m_characterEquipment.Guns[1]);
        //}
    }

    void Update()
    {




    }
}
