// 2Dの矩形UIの重なりと、マウス操作を判定する
#pragma once
#include "vector2.h"
#include "mouseInput.h"

//https://qiita.com/mohanty_pratyush/items/ffe8a37ed275e8a19ced

class UIHitBox
{
private:
    // シーンから入力対象に選択された状態
    bool m_Select = false;

    //見えない時は当たり判定は消す
    bool m_Enable = true;

    bool m_UseOBB = true;   // 回転を考慮した矩形判定を使用するか

    bool m_LeftTop = true;   //ピボット左上のオブジェクトをセンター変換するか

    bool m_BlockSelect = false;//複数の当たり判定が重なっているとき

    Vector2 m_Center;
    float m_Rotation; // ラジアン角
    Vector2 m_Size;
public:

    //ピボットが左上のオブジェクトを変換する
    void TransCenter()
    {
        m_LeftTop = true;
    }

    //マウスと重なっている時
    bool OverlapAABB() const
    {
        if (!m_Enable) return false;

        Vector2 mouse = Mouse::Pos();

        float halfW = m_Size.x * 0.5f;
        float halfH = m_Size.y * 0.5f;

        return (mouse.x >= m_Center.x - halfW &&
            mouse.x <= m_Center.x + halfW &&
            mouse.y >= m_Center.y - halfH &&
            mouse.y <= m_Center.y + halfH);
    }

    bool OverlapOBB() const
    {
        if (!m_Enable) return false;

        Vector2 mouse = Mouse::Pos();

        // マウスをローカル座標へ（中心基準）
        Vector2 local = mouse - m_Center;

        // 逆回転して軸平行状に戻す
        float cosR = cosf(-m_Rotation);
        float sinR = sinf(-m_Rotation);

        float x = local.x * cosR - local.y * sinR;
        float y = local.x * sinR + local.y * cosR;

        float halfW = m_Size.x * 0.5f;
        float halfH = m_Size.y * 0.5f;

        return (fabsf(x) <= halfW &&
            fabsf(y) <= halfH);
    }

    bool Overlap() const
    {
        if (!m_Enable) return false;

        if (m_UseOBB)
            return OverlapOBB();
        else
            return OverlapAABB();
    }

    // マウスと重なっている時&&押した瞬間
    bool Clicked(bool right = false) const
    {
        if (!m_Enable || !m_Select) return false;

        if (right && m_Select)
        {
            return Mouse::RightClick() && Overlap();
        }
        else
        {
            return Mouse::LeftClick() && Overlap();
        }
    }

    // マウスと重なっている時&&押し続けてる状態（ドラッグなどで使用）
    bool Pressed(bool right = false) const
    {
        if (!m_Enable || !m_Select) return false;

        if (right && m_Select)
        {
            return Mouse::RightHold() && Overlap();
        }
        else
        {
            return Mouse::LeftHold() && Overlap();
        }
    }

    // マウスと重なっている時&&離した瞬間
    bool Released(bool right = false) const
    {
        if (!m_Enable || !m_Select) return false;

        if (right)
        {
            return Mouse::RightRelease() && Overlap();
        }
        else
        {
            return Mouse::LeftRelease() && Overlap();
        }
    }

    bool DoubleClicked() const
    {
        if (!m_Enable || !m_Select) return false;

        return Mouse::LeftDoubleClick() && Overlap();
    }

    bool OutsideClicked() const
    {
        if (!m_Enable) return false;

        // 当たり判定に重なっていない
        if (Overlap()) return false;

        // クリック系のどれかが起きた
        if (Mouse::LeftClick() || Mouse::RightClick() || Mouse::MiddleClick())
        {
            return true;
        }

        return false;
    }
    //pos、size、ピボット左上、当たり判定OBB、回転
    void SetBox(const Vector2& pos, const Vector2& size, bool obb = false, float rot = 0.0f)
    {
        if (m_LeftTop)
        {
            m_Center = pos + size * 0.5f;
        }
        else
        {
            m_Center = pos;
        }
        m_Size = size;
        m_Rotation = rot;
        m_UseOBB = obb;
    }

    //当たり判定を存在させるか
    void SetEnable(bool enable) { m_Enable = enable; }
    bool GetEnable() const { return m_Enable; }

    //選択
    void SetSelect(bool select) { m_Select = select; }
    bool GetSelect() const { return m_Select; }

    // 入力対象の候補から除外するかを設定する
    void SetBlockSelect(bool block) { m_BlockSelect = block; }
    bool GetBlockSelect() const { return m_BlockSelect; }
};
