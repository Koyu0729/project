using System.Collections;
using UnityEngine;
using UnityEngine.UI;

[RequireComponent(typeof(Animator))]
public class ScreenAnimation : MonoBehaviour
{
    // AnimationClipファイル名と同じ列挙名にすること！
    public enum AnimationType
    {
        // Fade
        FadeIn = 0,
        FadeOut,
        ScrollStart,
        ScrollEnd,
        HalfFadeIn,
        HalfFadeOut,






        // ScreenEffect
        Flash = 10,

        







    }

    public static ScreenAnimation instance;

    public event System.Action onAnimationStart;
    public event System.Action onAnimationEnd;

    [SerializeField] private Animator m_animator;
    [SerializeField] private Canvas m_canvas;

    private void Reset()
    {
        m_animator = GetComponent<Animator>();
        m_canvas = GetComponent<Canvas>();
    }

    private void Awake()
    {
        if (instance == null)
        {
            instance = this;
            DontDestroyOnLoad(gameObject);
        }
        else
        {
            Destroy(gameObject);
        }
    }

    public void StartAnimation(AnimationType _type, float _duration, int _layer = 100)
    {
        StopAllCoroutines();

        m_canvas.sortingOrder = _layer;    // ソートレイヤーの値を設定

        StartCoroutine(IStartAnimation(_type, _duration));
    }

    private IEnumerator IStartAnimation(AnimationType _type, float _duration)
    {
        var handler = onAnimationStart;
        onAnimationStart = null;    // イベントの初期化
        handler?.Invoke();

        m_animator.speed = 1.0f / _duration;    // アニメーションの再生速度を調整

        string stateName = _type.ToString();
        m_animator.Play(stateName, 0, 0.0f);

        yield return null;

        var stateInfo = m_animator.GetCurrentAnimatorStateInfo(0);
        yield return new WaitForSecondsRealtime(stateInfo.length);

        m_animator.speed = 1.0f;    // アニメーションの再生速度を元に戻す
        
        handler = onAnimationEnd;
        onAnimationEnd = null;    // イベントの初期化
        handler?.Invoke();
    }
}
