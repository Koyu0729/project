// 【ファイルの役割】Volume設定をMaterialへ反映し、フルスクリーンのポストエフェクトを描画する。
// 【依存・注意点】Create、Pass登録、RenderGraph、Volume連携はUnity標準テンプレートにもある構成。名前検索・パラメータ自動転送・ダメージ値連携は標準テンプレートとの差分。作者はコードだけでは特定できない。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using System;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.RenderGraphModule;
using UnityEngine.Rendering.RenderGraphModule.Util;
using UnityEngine.Rendering.Universal;

/// <summary>
/// Unity 6 / URP RenderGraph 専用のカスタムポストエフェクトRendererFeature。
///
/// VolumeComponent名を文字列で指定し、
/// 対応するVolumeComponentが有効なら、指定Materialでフルスクリーンポストエフェクトを実行する。
///
/// 入力画面は URP の Blit 標準である _BlitTexture としてShaderへ渡される。
/// Shader側では _MainTexture ではなく _BlitTexture を読むこと。
///
/// VolumeComponent内のVolumeParameterは自動でMaterialへ渡す。
///
/// 例:
/// intensity      -> _Intensity
/// noiseScale     -> _NoiseScale
/// maskTexture    -> _MaskTexture
/// maskStrength   -> _MaskStrength
/// invertMask     -> _InvertMask
///
/// TextureParameter が null の場合は Texture2D.whiteTexture を渡す。
/// </summary>
public sealed class CustomPostProcessEffectRendererFeature : ScriptableRendererFeature
{
    #region FEATURE_FIELDS

    [Header("Target Volume")]
    [SerializeField]
    private string m_VolumeComponentName = "PE_Noize";

    [Header("Post Effect Material")]
    [SerializeField]
    private Material m_Material;

    [Header("Render Order")]
    [SerializeField]
    private RenderPassEvent m_RenderPassEvent =
        RenderPassEvent.AfterRenderingPostProcessing;

    [Header("Camera Filter")]
    [SerializeField]
    private bool m_ApplyOnlyPlaying = false;

    [SerializeField]
    private bool m_ApplyToSceneView = false;

    [SerializeField]
    private bool m_ApplyToPreviewCamera = false;

    [Header("Debug")]
    [SerializeField]
    private bool m_LogWarnings = false;

    private CustomPostRenderPass m_FullScreenPass;

    #endregion

    #region FEATURE_METHODS

    public override void Create()
    {
        if (m_Material == null)
        {
            m_FullScreenPass = null;
            return;
        }

        m_FullScreenPass = new CustomPostRenderPass(
            name,
            m_VolumeComponentName,
            m_Material,
            m_RenderPassEvent,
            m_LogWarnings
        );
    }

    public override void AddRenderPasses(
        ScriptableRenderer renderer,
        ref RenderingData renderingData
    )
    {
        if (m_Material == null || m_FullScreenPass == null)
        {
            return;
        }

        if (m_ApplyOnlyPlaying && !Application.isPlaying)
        {
            return;
        }

        CameraData cameraData = renderingData.cameraData;

        if (cameraData.cameraType == CameraType.Reflection)
        {
            return;
        }

        if (!m_ApplyToPreviewCamera && cameraData.cameraType == CameraType.Preview)
        {
            return;
        }

        if (!m_ApplyToSceneView && cameraData.isSceneViewCamera)
        {
            return;
        }

        VolumeComponent volumeComponent =
            VolumeComponentUtility.GetVolumeComponentByName(
                m_VolumeComponentName,
                m_LogWarnings
            );

        if (!VolumeComponentUtility.IsVolumeComponentActive(volumeComponent))
        {
            return;
        }

        m_FullScreenPass.SetSettings(
            name,
            m_VolumeComponentName,
            m_Material,
            m_RenderPassEvent,
            m_LogWarnings
        );

        m_FullScreenPass.renderPassEvent = m_RenderPassEvent;

        // 画面カラーを読むPassなのでColor入力を要求する。
        m_FullScreenPass.ConfigureInput(ScriptableRenderPassInput.Color);

        renderer.EnqueuePass(m_FullScreenPass);
    }

    protected override void Dispose(bool disposing)
    {
        if (m_FullScreenPass != null)
        {
            m_FullScreenPass.Dispose();
            m_FullScreenPass = null;
        }
    }

#if UNITY_EDITOR

    private void OnValidate()
    {
        Create();
    }

#endif

    #endregion

    private sealed class CustomPostRenderPass : ScriptableRenderPass
    {
        #region PASS_FIELDS

        private string m_PassName;
        private string m_VolumeComponentName;
        private Material m_Material;
        private bool m_LogWarnings;

        #endregion

        #region PASS_DATA

        private sealed class PassData
        {
            public TextureHandle src;
            public TextureHandle dst;
            public Material material;
            public string volumeComponentName;
            public bool logWarnings;
        }

        #endregion

        #region PASS_CONSTRUCTOR

        public CustomPostRenderPass(
            string passName,
            string volumeComponentName,
            Material material,
            RenderPassEvent renderPassEvent,
            bool logWarnings
        )
        {
            SetSettings(
                passName,
                volumeComponentName,
                material,
                renderPassEvent,
                logWarnings
            );

            profilingSampler = new ProfilingSampler(passName);

            this.renderPassEvent = renderPassEvent;

            requiresIntermediateTexture = true;
        }

        public void SetSettings(
            string passName,
            string volumeComponentName,
            Material material,
            RenderPassEvent renderPassEvent,
            bool logWarnings
        )
        {
            m_PassName = string.IsNullOrEmpty(passName)
                ? "CustomPostProcessPass"
                : passName;

            m_VolumeComponentName = volumeComponentName;
            m_Material = material;
            m_LogWarnings = logWarnings;

            this.renderPassEvent = renderPassEvent;
        }

        #endregion

        #region PASS_RENDER_GRAPH

        public override void RecordRenderGraph(
            RenderGraph renderGraph,
            ContextContainer frameData
        )
        {
            if (m_Material == null)
            {
                return;
            }

            VolumeComponent volumeComponent =
                VolumeComponentUtility.GetVolumeComponentByName(
                    m_VolumeComponentName,
                    m_LogWarnings
                );

            if (!VolumeComponentUtility.IsVolumeComponentActive(volumeComponent))
            {
                return;
            }

            UniversalResourceData resources =
                frameData.Get<UniversalResourceData>();

            TextureHandle src = resources.activeColorTexture;

            if (!src.IsValid())
            {
                return;
            }

            TextureDesc dstDesc = renderGraph.GetTextureDesc(src);
            dstDesc.name = "_CustomPostProcess_" + m_PassName;
            dstDesc.clearBuffer = false;

            TextureHandle dst = renderGraph.CreateTexture(dstDesc);

            using (IRasterRenderGraphBuilder builder =
                   renderGraph.AddRasterRenderPass<PassData>(
                       m_PassName,
                       out PassData passData,
                       profilingSampler
                   ))
            {
                passData.src = src;
                passData.dst = dst;
                passData.material = m_Material;
                passData.volumeComponentName = m_VolumeComponentName;
                passData.logWarnings = m_LogWarnings;

                builder.UseTexture(src, AccessFlags.Read);
                builder.SetRenderAttachment(dst, 0, AccessFlags.Write);

                builder.AllowGlobalStateModification(true);

                builder.SetRenderFunc(
                    (PassData data, RasterGraphContext context) =>
                    {
                        ExecutePass(data, context);
                    }
                );

                resources.cameraColor = dst;
            }
        }

        private static void ExecutePass(
            PassData data,
            RasterGraphContext context
        )
        {
            if (data == null)
            {
                return;
            }

            if (data.material == null)
            {
                return;
            }

            if (!data.src.IsValid())
            {
                return;
            }

            VolumeComponent volumeComponent =
                VolumeComponentUtility.GetVolumeComponentByName(
                    data.volumeComponentName,
                    data.logWarnings
                );

            if (!VolumeComponentUtility.IsVolumeComponentActive(volumeComponent))
            {
                return;
            }

            VolumeComponentUtility.ApplyVolumeParametersToMaterial(
                volumeComponent,
                data.material,
                data.logWarnings
            );

            if (volumeComponent is PE_Noize)
            {
                ApplyDamagePostEffectControllerValue(data.material);
            }

            // 重要:
            // Blitter.BlitTexture を使うと、入力画面はShader側の _BlitTexture に入る。
            // なのでShader側は _MainTexture ではなく _BlitTexture を読む。
            Blitter.BlitTexture(
                context.cmd,
                data.src,
                new Vector4(1.0f, 1.0f, 0.0f, 0.0f),
                data.material,
                0
            );
        }

        #endregion

        #region PASS_DISPOSE

        public void Dispose()
        {
        }

        #endregion

        // VolumeComponentからの値を直接コントローラークラスからMaterialへ渡す。
        //ここでしか書き換えられない
        #region CUSTOM_PARAMETERS
        private static void ApplyDamagePostEffectControllerValue(Material material)
        {
            if (material == null)
            {
                return;
            }

            if (!DamagePostEffectController.IsControlEnabled)
            {
                return;
            }

            if (material.HasProperty("_Intensity"))
            {
                material.SetFloat(
                    "_Intensity",
                    DamagePostEffectController.CurrentIntensity
                );
            }

            if (material.HasProperty("_MaskStrength"))
            {
                material.SetFloat(
                    "_MaskStrength",
                    DamagePostEffectController.CurrentMaskStrength
                );
            }
        }
        #endregion
    }
}

/// <summary>
/// VolumeComponentを名前で探したり、
/// VolumeComponent内のVolumeParameterをMaterialへ渡すための補助クラス。
/// </summary>
internal static class VolumeComponentUtility
{
    #region UTILITY_FIELDS

    private static readonly Dictionary<string, Type> s_VolumeComponentTypeCache =
        new Dictionary<string, Type>();

    private static MethodInfo s_GetComponentGenericMethod;

    #endregion

    #region COMPONENT_FIND

    public static VolumeComponent GetVolumeComponentByName(
        string componentName,
        bool logWarnings
    )
    {
        if (string.IsNullOrEmpty(componentName))
        {
            if (logWarnings)
            {
                Debug.LogWarning(
                    "[VolumeComponentUtility] Volume Component Name が空です。"
                );
            }

            return null;
        }

        VolumeStack stack = VolumeManager.instance.stack;

        if (stack == null)
        {
            if (logWarnings)
            {
                Debug.LogWarning(
                    "[VolumeComponentUtility] VolumeStack が取得できません。"
                );
            }

            return null;
        }

        Type targetType =
            FindVolumeComponentType(componentName, logWarnings);

        if (targetType == null)
        {
            if (logWarnings)
            {
                Debug.LogWarning(
                    $"[VolumeComponentUtility] VolumeComponent型が見つかりません: {componentName}"
                );
            }

            return null;
        }

        MethodInfo getComponentMethod =
            GetVolumeStackGenericGetComponentMethod();

        if (getComponentMethod == null)
        {
            if (logWarnings)
            {
                Debug.LogWarning(
                    "[VolumeComponentUtility] VolumeStack.GetComponent<T>() が見つかりません。"
                );
            }

            return null;
        }

        MethodInfo typedMethod =
            getComponentMethod.MakeGenericMethod(targetType);

        object result = typedMethod.Invoke(stack, null);

        return result as VolumeComponent;
    }

    private static Type FindVolumeComponentType(
        string componentName,
        bool logWarnings
    )
    {
        if (s_VolumeComponentTypeCache.TryGetValue(
                componentName,
                out Type cachedType
            ))
        {
            return cachedType;
        }

        Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();

        for (int assemblyIndex = 0; assemblyIndex < assemblies.Length; assemblyIndex++)
        {
            Assembly assembly = assemblies[assemblyIndex];

            Type[] types;

            try
            {
                types = assembly.GetTypes();
            }
            catch
            {
                continue;
            }

            for (int typeIndex = 0; typeIndex < types.Length; typeIndex++)
            {
                Type type = types[typeIndex];

                if (type == null)
                {
                    continue;
                }

                if (!typeof(VolumeComponent).IsAssignableFrom(type))
                {
                    continue;
                }

                if (type.Name == componentName || type.FullName == componentName)
                {
                    s_VolumeComponentTypeCache.Add(componentName, type);
                    return type;
                }
            }
        }

        if (logWarnings)
        {
            Debug.LogWarning(
                $"[VolumeComponentUtility] VolumeComponent型検索に失敗しました: {componentName}"
            );
        }

        return null;
    }

    private static MethodInfo GetVolumeStackGenericGetComponentMethod()
    {
        if (s_GetComponentGenericMethod != null)
        {
            return s_GetComponentGenericMethod;
        }

        MethodInfo[] methods = typeof(VolumeStack).GetMethods(
            BindingFlags.Instance |
            BindingFlags.Public
        );

        for (int i = 0; i < methods.Length; i++)
        {
            MethodInfo method = methods[i];

            if (method.Name != "GetComponent")
            {
                continue;
            }

            if (!method.IsGenericMethodDefinition)
            {
                continue;
            }

            if (method.GetParameters().Length != 0)
            {
                continue;
            }

            s_GetComponentGenericMethod = method;
            return s_GetComponentGenericMethod;
        }

        return null;
    }

    #endregion

    #region ACTIVE_CHECK

    public static bool IsVolumeComponentActive(VolumeComponent component)
    {
        if (component == null)
        {
            return false;
        }

        if (!component.active)
        {
            return false;
        }

        if (component is IPostProcessComponent postProcessComponent)
        {
            return postProcessComponent.IsActive();
        }

        return true;
    }

    #endregion

    #region APPLY_PARAMETERS

    public static void ApplyVolumeParametersToMaterial(
        VolumeComponent component,
        Material material,
        bool logWarnings
    )
    {
        if (component == null || material == null)
        {
            return;
        }

        Type componentType = component.GetType();

        FieldInfo[] fields = componentType.GetFields(
            BindingFlags.Instance |
            BindingFlags.Public |
            BindingFlags.NonPublic
        );

        for (int i = 0; i < fields.Length; i++)
        {
            FieldInfo field = fields[i];

            if (!typeof(VolumeParameter).IsAssignableFrom(field.FieldType))
            {
                continue;
            }

            object fieldValue = field.GetValue(component);

            if (!(fieldValue is VolumeParameter parameter))
            {
                continue;
            }

            string shaderPropertyName =
                ConvertFieldNameToShaderPropertyName(field.Name);

            ApplyParameterToMaterial(
                material,
                shaderPropertyName,
                parameter,
                logWarnings
            );
        }
    }

    private static string ConvertFieldNameToShaderPropertyName(
        string fieldName
    )
    {
        if (string.IsNullOrEmpty(fieldName))
        {
            return string.Empty;
        }

        char firstChar = char.ToUpperInvariant(fieldName[0]);

        if (fieldName.Length == 1)
        {
            return "_" + firstChar;
        }

        return "_" + firstChar + fieldName.Substring(1);
    }

    private static void ApplyParameterToMaterial(
        Material material,
        string propertyName,
        VolumeParameter parameter,
        bool logWarnings
    )
    {
        if (material == null || parameter == null)
        {
            return;
        }

        int propertyId = Shader.PropertyToID(propertyName);

        if (!material.HasProperty(propertyId))
        {
            if (logWarnings)
            {
                Debug.LogWarning(
                    $"[VolumeComponentUtility] MaterialにShaderPropertyがありません: {propertyName}"
                );
            }

            return;
        }

        if (parameter is FloatParameter floatParameter)
        {
            material.SetFloat(propertyId, floatParameter.value);
            return;
        }

        if (parameter is IntParameter intParameter)
        {
            material.SetInt(propertyId, intParameter.value);
            return;
        }

        if (parameter is BoolParameter boolParameter)
        {
            material.SetFloat(
                propertyId,
                boolParameter.value ? 1.0f : 0.0f
            );

            return;
        }

        if (parameter is ColorParameter colorParameter)
        {
            material.SetColor(propertyId, colorParameter.value);
            return;
        }

        if (parameter is Vector2Parameter vector2Parameter)
        {
            material.SetVector(propertyId, vector2Parameter.value);
            return;
        }

        if (parameter is Vector3Parameter vector3Parameter)
        {
            material.SetVector(propertyId, vector3Parameter.value);
            return;
        }

        if (parameter is Vector4Parameter vector4Parameter)
        {
            material.SetVector(propertyId, vector4Parameter.value);
            return;
        }

        if (parameter is TextureParameter textureParameter)
        {
            Texture texture = textureParameter.value;

            if (texture == null)
            {
                texture = Texture2D.whiteTexture;
            }

            material.SetTexture(propertyId, texture);
            return;
        }

        if (logWarnings)
        {
            Debug.LogWarning(
                $"[VolumeComponentUtility] 未対応のVolumeParameter型です: {parameter.GetType().Name}"
            );
        }
    }

    #endregion
}