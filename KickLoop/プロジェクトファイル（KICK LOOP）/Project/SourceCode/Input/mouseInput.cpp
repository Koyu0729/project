#include "main.h"
#include "mouseInput.h"
//クリック間隔の計測用
#include <chrono>

// static 変数の定義
LONG Mouse::mouseX = 0;
LONG Mouse::mouseY = 0;

float Mouse::wheel = 0.0f;

bool Mouse::left = false;
bool Mouse::right = false;
bool Mouse::middle = false;

bool Mouse::leftDown = false;
bool Mouse::rightDown = false;
bool Mouse::middleDown = false;

bool Mouse::leftUp = false;
bool Mouse::rightUp = false;
bool Mouse::middleUp = false;

bool Mouse::prevLeft = false;
bool Mouse::prevRight = false;
bool Mouse::prevMiddle = false;

void Mouse::SetMouseMove(int x, int y)
{
    mouseX += x;
    mouseY += y;
}

void Mouse::LeftDown() { left = true; }
void Mouse::LeftUp() { left = false; }

void Mouse::RightDown() { right = true; }
void Mouse::RightUp() { right = false; }

void Mouse::MiddleDown() { middle = true; }
void Mouse::MiddleUp() { middle = false; }

void Mouse::Wheel(float w)
{
    wheel = w * 30.0f;
}

void Mouse::Update()
{
    leftDown = (!prevLeft && left);
    leftUp = (prevLeft && !left);

    rightDown = (!prevRight && right);
    rightUp = (prevRight && !right);

    middleDown = (!prevMiddle && middle);
    middleUp = (prevMiddle && !middle);

    prevLeft = left;
    prevRight = right;
    prevMiddle = middle;
}

void Mouse::Clear()
{
    mouseX = 0;
    mouseY = 0;
    wheel = 0.0f;
}

bool Mouse::LeftClick() { return leftDown; }
bool Mouse::LeftHold() { return left; }
bool Mouse::LeftRelease() { return leftUp; }

//https://takap-tech.com/entry/2019/05/13/235416
bool Mouse::LeftDoubleClick()
{
    static bool waiting = false;
    static auto firstTime = std::chrono::system_clock::now();

    // 他ボタンが来たらキャンセル
    if (waiting && (RightClick() || MiddleClick()))
    {
        waiting = false;
        return false;
    }

    // 左クリックされた
    if (LeftClick())
    {
        if (!waiting)
        {
            waiting = true;
            firstTime = std::chrono::system_clock::now();
        }
        else
        {
            auto now = std::chrono::system_clock::now();
            //ミリ秒取得
            auto difference = std::chrono::duration_cast<std::chrono::milliseconds>(now - firstTime).count();

            waiting = false;
            return difference <= 300; // 300ms以内ならダブルクリックと判定する
        }
    }
    return false;
}

bool Mouse::RightClick() { return rightDown; }
bool Mouse::RightHold() { return right; }
bool Mouse::RightRelease() { return rightUp; }

bool Mouse::MiddleClick() { return middleDown; }
bool Mouse::MiddleHold() { return middle; }
bool Mouse::MiddleRelease() { return middleUp; }

Vector3 Mouse::Pos()
{
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(GetWindow(), &p);
    return Vector3((float)p.x, 0.0f, (float)p.y);
}

float Mouse::Wheel()
{
    return wheel;
}

LONG Mouse::DeltaX()
{
    return mouseX;
}

LONG Mouse::DeltaY()
{
    return mouseY;
}