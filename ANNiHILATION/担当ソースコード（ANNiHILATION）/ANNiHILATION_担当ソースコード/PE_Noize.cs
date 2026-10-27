using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;

/// <summary>
/// PE_Noize用のVolumeComponent。
///
/// Global Volume / Local Volume のProfileに追加して使う。
/// CustomPostProcessEffectRendererFeature側は、このVolumeの値を読んでShaderへ渡す。
///
/// Shader側のプロパティ名は、このクラスのフィールド名から自動変換される。
///
/// 例:
/// intensity              -> _Intensity
/// noiseScale             -> _NoiseScale
/// maskTexture            -> _MaskTexture
/// maskStrength           -> _MaskStrength
/// invertMask             -> _InvertMask
/// </summary>
[VolumeComponentMenu("Post-processing Custom/PE_Noize")]
[VolumeRequiresRendererFeatures(typeof(CustomPostProcessEffectRendererFeature))]
[SupportedOnRenderPipeline(typeof(UniversalRenderPipelineAsset))]
[DisplayInfo(name = "PE_Noize")]
public sealed class PE_Noize : VolumeComponent, IPostProcessComponent
{
    #region MAIN_PARAMETERS

    [Header("Main")]

    /// <summary>
    /// エフェクト全体の強さ。
    ///
    /// 0の場合はRendererFeature側でPass自体を追加しない。
    ///
    /// Shader側:
    /// _Intensity
    /// </summary>
    [Tooltip("エフェクト全体の強さ。0なら処理しない。")]
    public ClampedFloatParameter intensity =
        new ClampedFloatParameter(1.0f, 0.0f, 1.0f);

    /// <summary>
    /// 横ラインノイズの細かさ。
    ///
    /// Shader側:
    /// _NoiseScale
    /// </summary>
    [Tooltip("ノイズの細かさ。大きいほど細かくなる。")]
    public ClampedFloatParameter noiseScale =
        new ClampedFloatParameter(120.0f, 1.0f, 500.0f);

    /// <summary>
    /// ノイズや歪みの動く速さ。
    ///
    /// Shader側:
    /// _NoiseSpeed
    /// </summary>
    [Tooltip("ノイズの動く速さ。")]
    public ClampedFloatParameter noiseSpeed =
        new ClampedFloatParameter(15.0f, 0.0f, 100.0f);

    #endregion

    #region MASK_PARAMETERS

    [Header("Mask")]

    /// <summary>
    /// エフェクト範囲を制御するマスクテクスチャ。
    ///
    /// 注意:
    /// ここで Texture2D.whiteTexture を入れてはいけない。
    /// VolumeProfileのシリアライズ中に whiteTexture を呼ぶとUnityExceptionになる。
    ///
    /// null の場合は CustomPostProcessEffectRendererFeature 側で
    /// Texture2D.whiteTexture に差し替える。
    ///
    /// Shader側:
    /// _MaskTexture
    /// </summary>
    [Tooltip("エフェクト範囲を制御するマスク。白い場所ほど効果が強い。")]
    public TextureParameter maskTexture =
        new TextureParameter(null, false);

    /// <summary>
    /// マスクの影響度。
    ///
    /// 0:
    ///     マスク無効。画面全体に今まで通りかかる。
    ///
    /// 1:
    ///     マスク完全適用。
    ///
    /// Shader側:
    /// _MaskStrength
    /// </summary>
    [Tooltip("マスクの影響度。0ならマスク無効、1なら完全適用。")]
    public ClampedFloatParameter maskStrength =
        new ClampedFloatParameter(0.0f, 0.0f, 1.0f);

    /// <summary>
    /// マスクを反転するか。
    ///
    /// false:
    ///     白い場所に効果。
    ///
    /// true:
    ///     黒い場所に効果。
    ///
    /// Shader側:
    /// _InvertMask
    /// </summary>
    [Tooltip("マスクを反転するか。")]
    public BoolParameter invertMask =
        new BoolParameter(false);

    #endregion

    #region DISTORTION_PARAMETERS

    [Header("Distortion")]

    /// <summary>
    /// 歪み全体の強さ。
    ///
    /// Shader側:
    /// _DistortionStrength
    /// </summary>
    [Tooltip("歪み全体の強さ。")]
    public ClampedFloatParameter distortionStrength =
        new ClampedFloatParameter(1.0f, 0.0f, 5.0f);

    /// <summary>
    /// なめらかな波歪みの強さ。
    ///
    /// Shader側:
    /// _WaveStrength
    /// </summary>
    [Tooltip("なめらかな波歪みの強さ。")]
    public ClampedFloatParameter waveStrength =
        new ClampedFloatParameter(1.0f, 0.0f, 5.0f);

    /// <summary>
    /// 横ラインごとの細かいズレの強さ。
    ///
    /// Shader側:
    /// _RowJitterStrength
    /// </summary>
    [Tooltip("横ラインごとのズレの強さ。")]
    public ClampedFloatParameter rowJitterStrength =
        new ClampedFloatParameter(1.0f, 0.0f, 5.0f);

    /// <summary>
    /// 太い帯が横にズレる強さ。
    ///
    /// Shader側:
    /// _BandStrength
    /// </summary>
    [Tooltip("太い帯ズレの強さ。")]
    public ClampedFloatParameter bandStrength =
        new ClampedFloatParameter(1.0f, 0.0f, 5.0f);

    #endregion

    #region COLOR_NOISE_PARAMETERS

    [Header("Color / Noise")]

    /// <summary>
    /// RGBチャンネルのズレ量。
    ///
    /// Shader側:
    /// _RgbShiftStrength
    /// </summary>
    [Tooltip("RGB色ズレの強さ。")]
    public ClampedFloatParameter rgbShiftStrength =
        new ClampedFloatParameter(1.0f, 0.0f, 5.0f);

    /// <summary>
    /// 粒ノイズの強さ。
    ///
    /// Shader側:
    /// _GrainStrength
    /// </summary>
    [Tooltip("粒ノイズの強さ。")]
    public ClampedFloatParameter grainStrength =
        new ClampedFloatParameter(0.5f, 0.0f, 5.0f);

    /// <summary>
    /// 走査線の強さ。
    ///
    /// Shader側:
    /// _ScanlineStrength
    /// </summary>
    [Tooltip("走査線の強さ。")]
    public ClampedFloatParameter scanlineStrength =
        new ClampedFloatParameter(0.5f, 0.0f, 5.0f);

    #endregion

    #region POST_PROCESS_COMPONENT

    /// <summary>
    /// このVolumeが有効かどうか。
    ///
    /// activeはVolumeComponent自体の有効チェック。
    /// intensityが0以下なら処理自体を走らせない。
    /// </summary>
    public bool IsActive()
    {
        return active;
    }

    /// <summary>
    /// タイル互換かどうか。
    ///
    /// 画面全体のUVを歪ませる処理なのでfalse。
    /// </summary>
    public bool IsTileCompatible()
    {
        return false;
    }

    #endregion
}