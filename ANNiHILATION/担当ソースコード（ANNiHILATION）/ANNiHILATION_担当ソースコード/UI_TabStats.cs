// 【ファイルの役割】インベントリ入力でステータスUIの開閉状態を切り替え、位置を補間する。
// 【依存・注意点】GameInputManagerの入力イベントを購読・解除する。OnEnable時のManager初期化順序に注意。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

using UnityEngine;
using UnityEngine.InputSystem;

public class UI_TabStats : UI_Base
{
	[Header("HUD")]
	[SerializeField] private RectTransform m_Pos;

	[Header("Hide Offset")]
	[SerializeField] private Vector2 m_showOffset;

	[Header("Animation")]
	[SerializeField] private float m_LerpSpeed = 10.0f;

	[SerializeField, ReadOnly] private bool m_push = false;
	[SerializeField, ReadOnly] private Vector2 m_default;

	private void Awake()
	{
		base.Awake();

        if (m_Pos != null)
		{
			m_default = m_Pos.anchoredPosition;
		}
	}

	private void OnEnable()
	{
		GameInputManager.Instance.InputActions.UI.OpenInventory.started += OnPush; 
	}

	private void Update()
	{
		UpdateHUDPosition();
	}

	private void UpdateHUDPosition()
	{
		if (m_Pos == null)
		{
			return;
		}

		Vector2 hudTargetPosition = m_default;

		if (m_push)
		{
			hudTargetPosition += m_showOffset;
		}

		m_Pos.anchoredPosition = Vector2.Lerp(
			m_Pos.anchoredPosition,
			hudTargetPosition,
			Time.deltaTime * m_LerpSpeed
		);
	}

	private void OnPush(InputAction.CallbackContext context)
	{
		m_push = !m_push;
	}

	private void OnDisable()
	{
		if (GameInputManager.Instance == null)
		{
			return;
		}

		GameInputManager.Instance.InputActions.UI.OpenInventory.started -= OnPush;
	}
}