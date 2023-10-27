#pragma once

#include <flecs.h>
#include "vectors.h"

struct transform {
    struct Global {
        float2 position {0, 0};
        float rotation_radians = 0;
        float scale = 1;

        [[nodiscard]] float2 local_to_global(float2 local) const;
    };

    struct Position {
        float2 value;
    };

    struct Rotation {
        float radians;
    };

    struct Scale {
        float value;
    };

    explicit transform(flecs::world & world);
};