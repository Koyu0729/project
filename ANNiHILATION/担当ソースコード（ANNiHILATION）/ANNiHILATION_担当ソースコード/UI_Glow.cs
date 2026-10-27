// 【ファイルの役割】ImageのMaterialへ発光色と強度を渡す。
// 【依存・注意点】_EmissionColorを設定するが、同梱UI_Glow.shaderは_BloomColorを使うため、その組合せは修正が必要。共有Materialの変更にも注意。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using UnityEngine;
using UnityEngine.UI;

public class UI_Glow : MonoBehaviour
{
    public Color glowColor = new Color(1.0f, 1.0f, 0.5f); // 発光色
    public float glowIntensity = 2.0f; // 発光の強度

    void Start()
    {
        // マテリアルの発光色を設定
        GetComponent<Image>().material.SetColor("_EmissionColor", glowColor * glowIntensity);
    }
}