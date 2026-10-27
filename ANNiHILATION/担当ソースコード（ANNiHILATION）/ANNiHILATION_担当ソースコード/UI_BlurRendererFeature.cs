using System;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.RenderGraphModule;
using UnityEngine.Rendering.RenderGraphModule.Util;
using UnityEngine.Rendering.Universal;

/// <summary>
/// Main Cameraに描画された背景画像を低解像度でブラーし、
/// "_UIBlurTexture" という名前のグローバルテクスチャとして登録するRendererFeature。
///
/// Camera Stack / UI Camera構成でも使えるように、
/// RenderGraph内の一時Textureではなく、永続RTHandleへブラー結果を書き込む。
///
/// 想定する描画構成:
///     Main Camera
///         World / FoW / 標準PostProcess / _UIBlurTexture生成
///
///     UI Camera
///         Camera Canvasを描画
///         UI_BlurMask.shader が _UIBlurTexture を読む
///
/// 注意:
///     このRendererFeatureは Base Camera のみで実行する。
///     Overlay Camera側で実行すると、UI Cameraの描画結果を元にブラーしようとして壊れる。
/// </summary>
public sealed class UI_BlurRendererFeature : ScriptableRendererFeature
{
    [Serializable]
    public class UI_BlurSettings
    {
        [Header("Blur Shader")]
        [Tooltip("背景ブラー用のFullscreen Shaderを指定する。UI_Blur.shader を入れる。")]
        public Shader blurShader;

        [Header("Blur Settings")]
        [Tooltip("ブラー用テクスチャの解像度を下げる倍率。2なら半分、4なら1/4。大きいほど軽いが粗くなる。")]
        [Range(1, 8)]
        public int downSample = 2;

        [Tooltip("横方向のブラーの強さ。")]
        [Range(0.0f, 10.0f)]
        public float horizontalBlur = 2.0f;

        [Tooltip("縦方向のブラーの強さ。")]
        [Range(0.0f, 10.0f)]
        public float verticalBlur = 2.0f;

        [Header("Render Order")]
        [Tooltip("どのタイミングで背景ブラーを作るか。UI Cameraで読むなら AfterRenderingPostProcessing 推奨。")]
        public RenderPassEvent renderPassEvent = RenderPassEvent.AfterRenderingPostProcessing;
    }

    [SerializeField]
    private UI_BlurSettings m_settings = new UI_BlurSettings();

    private Material m_blurMaterial;
    private UI_BlurRenderPass m_blurRenderPass;

    public override void Create()
    {
        DisposeMaterial();

        if (m_settings == null || m_settings.blurShader == null)
        {
            m_blurRenderPass = null;
            return;
        }

        m_blurMaterial = CoreUtils.CreateEngineMaterial(m_settings.blurShader);

        m_blurRenderPass = new UI_BlurRenderPass(
            m_blurMaterial,
            m_settings
        )
        {
            renderPassEvent = m_settings.renderPassEvent
        };
    }

    public override void AddRenderPasses(
        ScriptableRenderer renderer,
        ref RenderingData renderingData
    )
    {
        if (m_blurRenderPass == null)
        {
            return;
        }

        if (m_blurMaterial == null)
        {
            return;
        }

        if (renderingData.cameraData.cameraType != CameraType.Game)
        {
            return;
        }

        // Camera Stack構成では、Overlay Camera側では実行しない。
        // _UIBlurTextureはMain Camera(Base Camera)の結果から作る。
        if (renderingData.cameraData.renderType != CameraRenderType.Base)
        {
            return;
        }

        m_blurRenderPass.renderPassEvent = m_settings.renderPassEvent;
        m_blurRenderPass.ConfigureInput(ScriptableRenderPassInput.Color);

        renderer.EnqueuePass(m_blurRenderPass);
    }

    protected override void Dispose(bool disposing)
    {
        DisposeMaterial();

        if (m_blurRenderPass != null)
        {
            m_blurRenderPass.Dispose();
            m_blurRenderPass = null;
        }
    }

    private void DisposeMaterial()
    {
        if (m_blurMaterial == null)
        {
            return;
        }

        CoreUtils.Destroy(m_blurMaterial);
        m_blurMaterial = null;
    }

#if UNITY_EDITOR

    private void OnValidate()
    {
        Create();
    }

#endif

    private sealed class UI_BlurRenderPass : ScriptableRenderPass
    {
        private static readonly int HorizontalBlurId = Shader.PropertyToID("_HorizontalBlur");
        private static readonly int VerticalBlurId = Shader.PropertyToID("_VerticalBlur");
        private static readonly int UIBlurTextureId = Shader.PropertyToID("_UIBlurTexture");

        private const string CopyAndHorizontalPassName = "UI Blur Copy And Horizontal";
        private const string VerticalPassName = "UI Blur Vertical";

        private readonly Material m_blurMaterial;
        private readonly UI_BlurSettings m_settings;

        private RTHandle m_tempBlurTexture;
        private RTHandle m_uiBlurTexture;

        public UI_BlurRenderPass(
            Material blurMaterial,
            UI_BlurSettings settings
        )
        {
            m_blurMaterial = blurMaterial;
            m_settings = settings;

            requiresIntermediateTexture = true;
        }

        public void Dispose()
        {
            if (m_tempBlurTexture != null)
            {
                m_tempBlurTexture.Release();
                m_tempBlurTexture = null;
            }

            if (m_uiBlurTexture != null)
            {
                m_uiBlurTexture.Release();
                m_uiBlurTexture = null;
            }
        }

        public override void RecordRenderGraph(
            RenderGraph renderGraph,
            ContextContainer frameData
        )
        {
            if (m_blurMaterial == null || m_settings == null)
            {
                return;
            }

            UniversalResourceData resourceData =
                frameData.Get<UniversalResourceData>();

            UniversalCameraData cameraData =
                frameData.Get<UniversalCameraData>();

            if (resourceData.isActiveTargetBackBuffer)
            {
                return;
            }

            TextureHandle cameraColorTexture =
                resourceData.activeColorTexture;

            if (!cameraColorTexture.IsValid())
            {
                return;
            }

            UpdateMaterialParameters();

            RenderTextureDescriptor blurDescriptor =
                CreateBlurDescriptor(cameraData);

            if (blurDescriptor.width <= 0 || blurDescriptor.height <= 0)
            {
                return;
            }

            AllocatePersistentTextures(blurDescriptor);

            if (m_tempBlurTexture == null || m_uiBlurTexture == null)
            {
                return;
            }

            // RenderGraph内のTextureHandleではなく、
            // 永続RTHandleのRenderTexture本体をグローバル登録する。
            // Base Cameraで書いた結果をOverlay CameraのUIシェーダから読むため。
            Shader.SetGlobalTexture(
                UIBlurTextureId,
                m_uiBlurTexture.rt
            );

            TextureHandle tempTexture =
                renderGraph.ImportTexture(m_tempBlurTexture);

            TextureHandle persistentBlurTexture =
                renderGraph.ImportTexture(m_uiBlurTexture);

            if (!tempTexture.IsValid() || !persistentBlurTexture.IsValid())
            {
                return;
            }

            RenderGraphUtils.BlitMaterialParameters horizontalParams =
                new RenderGraphUtils.BlitMaterialParameters(
                    cameraColorTexture,
                    tempTexture,
                    m_blurMaterial,
                    0
                );

            renderGraph.AddBlitPass(
                horizontalParams,
                CopyAndHorizontalPassName
            );

            RenderGraphUtils.BlitMaterialParameters verticalParams =
                new RenderGraphUtils.BlitMaterialParameters(
                    tempTexture,
                    persistentBlurTexture,
                    m_blurMaterial,
                    1
                );

            renderGraph.AddBlitPass(
                verticalParams,
                VerticalPassName
            );
        }

        private RenderTextureDescriptor CreateBlurDescriptor(
            UniversalCameraData cameraData
        )
        {
            RenderTextureDescriptor descriptor =
                cameraData.cameraTargetDescriptor;

            int downSample = Mathf.Max(1, m_settings.downSample);

            descriptor.width =
                Mathf.Max(1, descriptor.width / downSample);

            descriptor.height =
                Mathf.Max(1, descriptor.height / downSample);

            descriptor.depthBufferBits = 0;
            descriptor.msaaSamples = 1;

            descriptor.useMipMap = false;
            descriptor.autoGenerateMips = false;
            descriptor.enableRandomWrite = false;
            descriptor.bindMS = false;

            // カメラのHDRフォーマット(B10G11R11_UFloatPack32など)を引き継がない。
            // UIで読むブラー用テクスチャなので、普通のRGBAに固定する。
            descriptor.graphicsFormat =
                UnityEngine.Experimental.Rendering.GraphicsFormat.R8G8B8A8_UNorm;

            descriptor.colorFormat = RenderTextureFormat.ARGB32;

            return descriptor;
        }

        private void AllocatePersistentTextures(
            RenderTextureDescriptor descriptor
        )
        {
            RenderingUtils.ReAllocateHandleIfNeeded(
                ref m_tempBlurTexture,
                descriptor,
                FilterMode.Bilinear,
                TextureWrapMode.Clamp,
                name: "_UIBlurTempTexture"
            );

            RenderingUtils.ReAllocateHandleIfNeeded(
                ref m_uiBlurTexture,
                descriptor,
                FilterMode.Bilinear,
                TextureWrapMode.Clamp,
                name: "_UIBlurTexture"
            );
        }

        private void UpdateMaterialParameters()
        {
            m_blurMaterial.SetFloat(
                HorizontalBlurId,
                m_settings.horizontalBlur
            );

            m_blurMaterial.SetFloat(
                VerticalBlurId,
                m_settings.verticalBlur
            );
        }
    }
}