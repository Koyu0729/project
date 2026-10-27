// 【ファイルの役割】低HP用の色・暗さ・ビネット形状をVolumeパラメータとして保持する。
// 【依存・注意点】有効かつintensityが0より大きい場合に描画対象となる。値を変える制御は別の処理が担当する。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;

[VolumeComponentMenu("Post-processing Custom/PE_LowHP")]
[VolumeRequiresRendererFeatures(typeof(CustomPostProcessEffectRendererFeature))]
[SupportedOnRenderPipeline(typeof(UniversalRenderPipelineAsset))]
[DisplayInfo(name = "PE_LowHP")]
public sealed class PE_LowHP : VolumeComponent, IPostProcessComponent
{
    [Header("Main")]
    public ClampedFloatParameter intensity =
        new ClampedFloatParameter(0.0f, 0.0f, 1.0f);

    [Header("Vignette")]
    public ColorParameter vignetteColor =
        new ColorParameter(new Color(1.0f, 0.0f, 0.0f, 1.0f), true);

    public ClampedFloatParameter colorStrength =
        new ClampedFloatParameter(1.2f, 0.0f, 3.0f);

    public ClampedFloatParameter vignettePower =
        new ClampedFloatParameter(2.5f, 0.1f, 8.0f);

    public ClampedFloatParameter darkness =
        new ClampedFloatParameter(0.25f, 0.0f, 1.0f);

    [Header("Vignette Mask")]
    [Tooltip("ビネットの形を制御するマスク。白い場所ほど赤くなる。")]
    public TextureParameter vignetteMaskTexture =
        new TextureParameter(null, false);

    [Tooltip("ビネットマスクの影響度。0なら通常ビネット、1ならマスク完全適用。")]
    public ClampedFloatParameter vignetteMaskStrength =
        new ClampedFloatParameter(0.0f, 0.0f, 1.0f);

    [Tooltip("ビネットマスクを反転するか。")]
    public BoolParameter invertVignetteMask =
        new BoolParameter(false);

    public bool IsActive()
    {
        return active && intensity.value > 0.0f;
    }

    public bool IsTileCompatible()
    {
        return false;
    }
}