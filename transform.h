#pragma once

#include <flecs.h>
#include "vectors.h"

struct transform {
    struct Transform {
        float2 position {0, 0};
        float rotation = 0;
        float scale = 1;

        [[nodiscard]] float2 apply(float2 local) const;
    };

    struct Local : public Transform {};
    struct Global : public Transform {};

    explicit transform(flecs::world & world);
};