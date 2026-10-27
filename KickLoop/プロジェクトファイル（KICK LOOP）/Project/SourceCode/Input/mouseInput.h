#pragma once
#include <windows.h>
#include "vector3.h"

class Mouse
{
private:
    static LONG mouseX, mouseY;
    static float wheel;

    //ボタンを押しているか調べる
    static bool left, right, middle;

    //左クリック、押した瞬間、離した瞬間
    static bool leftDown, leftUp;

    //右クリック、押した瞬間、離した瞬間
    static bool rightDown, rightUp;

    //マウスホイールクリック、押した瞬間、離した瞬間
    static bool middleUp, middleDown;

    //押した瞬間、離した瞬間の記録
    static bool prevLeft, prevRight, prevMiddle;
public:
    static void Update();
    static void Clear();

    //RawInput から入力を受け取る
    static void SetMouseMove(int x, int y);

    static void LeftDown();
    static void LeftUp();

    static void RightDown();
    static void RightUp();

    static void MiddleDown();
    static void MiddleUp();

    static void Wheel(float w);

    // 1フレーム分のマウス移動量を取得
    static LONG DeltaX();
    static LONG DeltaY();

    //ゲームから参照する入力状態
    static bool LeftClick();        // 押した瞬間
    static bool LeftHold();         // 押している間
    static bool LeftRelease();      // 離した瞬間
    static bool LeftDoubleClick();  //ダブルクリック用

    static bool RightClick();    // 押した瞬間
    static bool RightHold();     // 押している間
    static bool RightRelease();  // 離した瞬間

    static bool MiddleClick();    // 押した瞬間
    static bool MiddleHold();     // 押している間
    static bool MiddleRelease();  // 離した瞬間

    static Vector3 Pos();        // マウス座標
    static float Wheel();        // ホイール
};
