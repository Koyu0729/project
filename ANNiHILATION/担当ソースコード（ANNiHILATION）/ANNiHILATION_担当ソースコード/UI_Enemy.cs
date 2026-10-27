// 【ファイルの役割】EnemyManagerの敵数をテキスト表示し、数が変わったときに更新する。
// 【依存・注意点】Startでシーン内のEnemyManagerと子Textを取得する。参照がない場合のnull対策は別途必要。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using TMPro;
using UnityEngine;

public class UI_Enemy : MonoBehaviour
{
    [SerializeField]
    private TextMeshProUGUI m_enemyCountText;

    [SerializeField, ReadOnly]
    private EnemyManager m_enemyManager;

    int m_prevEnemyCount = -1;

    void Start()
    {
        m_enemyCountText = GetComponentInChildren<TextMeshProUGUI>();

        m_enemyManager = FindAnyObjectByType<EnemyManager>();
    }

    void Update()
    {
        if(m_enemyManager.EnemyCount != m_prevEnemyCount)
        {
            m_enemyCountText.text = m_enemyManager.EnemyCount.ToString();
        }
        m_prevEnemyCount = m_enemyManager.EnemyCount;
    }
}
