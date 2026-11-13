#pragma once

// ノードを生成するときに使う、各種類のヘッダー一覧

// 開始
#include "start.h"

// 条件分岐
#include "ifFindObject.h"
#include "ifPlayerStats.h"
#include "ifRangeIO.h"
#include "targetCondition.h"
#include "tacticalCondition.h"

// 近接攻撃
#include "attackMelee.h"

// 遠距離攻撃
#include "attackGun.h"

// 時間指定の行動と防御
#include "timedAction.h"
#include "guard.h"

// 移動
#include "moveForward.h"
#include "moveBack.h"
#include "moveEscape.h"
#include "moveToTarget.h"
#include "moveStop.h"
#include "sideMove.h"
#include "orbitMovement.h"
#include "tacticalMovement.h"

// 回転
#include "faceLeft.h"
#include "faceRight.h"
#include "turnLeft.h"
#include "turnRight.h"
#include "targetRotation.h"
#include "spinRotation.h"
