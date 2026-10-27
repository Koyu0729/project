// 【ファイルの役割】3Dアビリティゲージの旧実装をコメントとして保持している。
// 【依存・注意点】クラス全体がコメントアウトされており、このファイルから実行される処理はない。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

//using System.Collections.Generic;
//using Unity.VisualScripting;
//using UnityEngine;
//using UnityEngine.UI;

///// <summary>
///// プレイヤーのアビリティゲージUIを管理するクラス。
///// 現在値・最大値に応じてUIの表示数とゲージ量を更新する。
///// </summary>
//public class UI_Ability3D : UI_PlayerBase
//{
//    [Header("UI")]
//    // 最大UI数、これ以上は生成しない
//    [SerializeField] private int m_maxCountUI = 10;

//    // 生成するUIのプレハブ、親オブジェクト、オフセット
//    [SerializeField] private RectTransform m_abilityPrefab;
//    [SerializeField] private Transform m_parent;
//    [SerializeField] private Vector3 m_offset;

//    // 生成したUIを管理するリスト
//    private List<RectTransform> m_abilityPrefabList = new List<RectTransform>();
    
//    private List<Image> m_frameList = new List<Image>();
//    private List<Image> m_capsuleList = new List<Image>();
    
//    [Header("参照")]
//    [SerializeField, ReadOnly] private BaseStatus m_status;

//    [SerializeField, ReadOnly] private Parameter m_abilityEnergy;
//    [SerializeField, ReadOnly] private Parameter m_maxAbilityEnergy;

//    // 前回の値、更新するかどうか
//    private float m_prevAbilityEnergy = 0.0f;
//    private float m_prevMaxAbilityEnergy = 0.0f;
//    protected override void Awake()
//    {
//        base.Awake();
//    }

//    private void Start()
//    {
//        if (m_player == null)
//        {
//            Debug.LogWarning($"{nameof(UI_Ability)} : Player が見つかりません。");
//            return;
//        }

//        m_status = m_player.GetComponent<BaseStatus>();

//        if (m_status == null)
//        {
//            Debug.LogWarning($"{nameof(UI_Ability)} : BaseStatus が見つかりません。");
//            return;
//        }

//        // パラメーターの取得
//        m_abilityEnergy = m_status.GetParameter(ParameterStatusType.AbilityEnergy);
//        m_maxAbilityEnergy = m_status.GetParameter(ParameterStatusType.MaxAbilityEnergy);

//        // UIの生成
//        CreateUI();
//    }
//    private void Update()
//    {
//        if (m_status == null)
//        {
//            return;
//        }

//        if (m_abilityEnergy == null || m_maxAbilityEnergy == null)
//        {
//            return;
//        }

//        //--------------------------------------------------
//        //UI更新、値変わるなら更新
//        //--------------------------------------------------
//        if (!Mathf.Approximately(m_prevAbilityEnergy, m_abilityEnergy.DefaultValue))
//        {
//            // カプセルの更新
//            UpdateCapsule(m_abilityEnergy.DefaultValue);
//            // 前回の値を更新
//            m_prevAbilityEnergy = m_abilityEnergy.DefaultValue;
//        }
        
//        if (!Mathf.Approximately(m_prevMaxAbilityEnergy, m_maxAbilityEnergy.DefaultValue))
//        {
//            for (int i = 0; i < m_abilityPrefabList.Count; i++)
//            {
//                if (i < m_maxAbilityEnergy.DefaultValue)
//                {
//                    // 表示
//                    ShowUI(m_abilityPrefabList[i]);
//                }
//                else
//                {
//                    HideUI(m_abilityPrefabList[i]);
//                }
//            }
//            // フレームの更新
//            UpdateFrame(m_maxAbilityEnergy.DefaultValue);
//            // 前回の値を更新
//            m_prevMaxAbilityEnergy = m_maxAbilityEnergy.DefaultValue;
//        }
//    }
//    private void CreateUI()
//    {
//        m_abilityPrefabList.Clear();
//        m_frameList.Clear();
//        m_capsuleList.Clear();

//        for (int i = 0; i < m_maxCountUI; i++)
//        {
//            RectTransform obj = Instantiate(m_abilityPrefab, m_parent);

//            if (obj == null)
//            {
//                Debug.LogError($"{nameof(UI_Ability)} : Instantiate に失敗しました。");
//                return;
//            }

//            // オフセットをかけて配置
//            obj.transform.localPosition = m_offset * i;

//            // フレームとカプセルのイメージを取得
//            Transform frameTransform = obj.transform.Find("abilityFrame");
//            Transform capsuleTransform = obj.transform.Find("abilityCapsule");

//            // どちらも見つからない場合はエラー
//            Image frame = frameTransform.GetComponent<Image>();
//            Image capsule = capsuleTransform.GetComponent<Image>();

//            frame.fillAmount = 1.0f;
//            capsule.fillAmount = 1.0f;

//            // プレハブリストに追加
//            m_abilityPrefabList.Add(obj);
//            // フレームとカプセルをリストに追加
//            m_frameList.Add(frame);
//            m_capsuleList.Add(capsule);
//        }
//    }

//    // カプセルの更新、現在のエネルギーを渡す
//    private void UpdateCapsule(float currentEnergy)
//    {
//        // 整数部分と小数部分を分ける
//        int integer = (int)currentEnergy;
//        float few = currentEnergy - integer;

//        // カプセルは現在のエネルギーに応じて表示するかどうかを決める
//        for (int i = 0; i < m_capsuleList.Count; i++)
//        {
//            if (m_capsuleList[i] == null)
//            {
//                continue;
//            }

//            if (i < integer)
//            {
//                m_capsuleList[i].fillAmount = 1.0f;
//            }
//            else if (i == integer)
//            {
//                m_capsuleList[i].fillAmount = few;
//            }
//            else
//            {
//                m_capsuleList[i].fillAmount = 0.0f;
//            }
//        }
//    }

//    // フレームの更新、最大エネルギーを渡す
//    private void UpdateFrame(float maxEnergy)
//    {
//        // 整数部分と小数部分を分ける
//        int integer = (int)maxEnergy;
//        float few = maxEnergy - integer;

//        // フレームは最大エネルギーに応じて表示するかどうかを決める
//        for (int i = 0; i < m_frameList.Count; i++)
//        {
//            if (m_frameList[i] == null)
//            {
//                continue;
//            }

//            if (i < integer)
//            {
//                m_frameList[i].fillAmount = 1.0f;
//            }
//            else if (i == integer)
//            {
//                m_frameList[i].fillAmount = few;
//            }
//            else
//            {
//                m_frameList[i].fillAmount = 0.0f;
//            }
//        }
//    }
//}