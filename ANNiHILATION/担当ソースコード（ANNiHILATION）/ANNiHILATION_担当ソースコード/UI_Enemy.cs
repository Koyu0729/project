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
