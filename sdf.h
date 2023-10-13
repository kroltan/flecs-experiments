#pragma once

#include <flecs.h>
#include <raylib.h>
#include "vectors.h"

struct sdf {
    struct Line {
        float2 start, end;
    };

    struct Circle {
        float2 center;
        float radius;
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