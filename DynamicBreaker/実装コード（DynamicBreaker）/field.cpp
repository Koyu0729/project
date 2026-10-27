#include "directx.h"
#include "game.h"
#include "model.h"
#include "block.h"
#include "field.h"

FIELD::FIELD(int width, int depth, float spacing)
{
    // サイズや間隔が0以下なら、床は作らない。
    if (width <= 0 || depth <= 0 || spacing <= 0.0f)
    {
        return;
    }
    for (int x = 0; x < width; ++x)
    {
        for (int z = 0; z < depth; ++z)
        {
            // 作った床は、BLOCK側で管理リストに追加される。
            new BLOCK(XMFLOAT3(spacing * x, -2.0f, spacing * z), MODEL_NULL);
        }
    }
    new BLOCK(XMFLOAT3(5.0f, 0.0f, 5.0f), MODEL_TOWER);
}

FIELD::~FIELD()
{
}
