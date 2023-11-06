#pragma once

#include <chrono>
#include <flecs.h>

struct animate {
    enum class Ease {
        Linear,
        InQuad,
        OutQuad,
        InOutQuad,
        Step,
    };

    struct Target {};

    struct Property {
        char *field;
    };

    struct Time {
        float value;
    };

    struct Tween {
        float from = 0;
        float to = 0;
        Ease ease = Ease::Linear;
    };

    struct Delay {
        float value = 0;
    };

    struct Speed {
        float value = 1;
    };

    explicit animate(flecs::world &tween);
};
