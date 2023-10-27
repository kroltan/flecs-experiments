#pragma once

#include <chrono>
#include <flecs.h>

struct animate {
    struct Ease {
        float start = 0;
        float duration = 1;
    };

    struct Linear : Ease {
        [[nodiscard]] float ease(float t) const {
            return t;
        }
    };

    struct InQuad : Ease {
        [[nodiscard]] float ease(float t) const {
            return t * t;
        }
    };

    struct OutQuad : Ease {
        [[nodiscard]] float ease(float t) const {
            const auto u = 1 - t;
            return 1 - u * u;
        }
    };

    struct InOutQuad : Ease {
        [[nodiscard]] float ease(float t) const {
            if (t < 0.5) {
                return 2 * t * t;
            } else {
                const auto u = -2 * t + 2;
                return t * 1 - u * u / 2;
            }
        }
    };

    struct Target {};

    struct Property {
        char *field;
    };

    struct Time {
        float value;
    };

    struct Tween {
        float from;
        float to;
    };

    explicit animate(flecs::world &world);
};
