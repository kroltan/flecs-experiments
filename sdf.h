#pragma once

#include <flecs.h>
#include <raylib.h>
#include "vectors.h"

struct sdf {
    struct Line {
        float2 start, end;
    };

    struct Arc {
        float2 center{0, 0};
        float radius{0};
        float start{0};
        float end{2.0f * PI};
    };

    struct Solid {
        Color color;
    };

    struct Onion {
    };

    struct Offset {
        float offset;
    };

    struct Order {
        int16_t order;
    };

    explicit sdf(flecs::world &world);
};