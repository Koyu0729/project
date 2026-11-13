#include "directx.h"
#include "game.h"
#include "model.h"
#include "block.h"
#include "field.h"

FIELD::FIELD(int width, int depth, float spacing)
{
    if (width <= 0 || depth <= 0 || spacing <= 0.0f)
    {
        return;
    }
    const float floorY = -2.0f;
    const XMFLOAT3 towerPosition(5.0f, 0.0f, 5.0f);

    // BLOCK のコンストラクタで、生成した床をシーンに登録する。
    for (int x = 0; x < width; ++x)
    {
        const float floorX = spacing * x;
        for (int z = 0; z < depth; ++z)
        {
            new BLOCK(XMFLOAT3(floorX, floorY, spacing * z), MODEL_NULL);
        }
    }
    new BLOCK(towerPosition, MODEL_TOWER);
}

FIELD::~FIELD() = default;
