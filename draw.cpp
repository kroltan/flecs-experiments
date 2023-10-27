#include <iostream>
#include "draw.h"

void setup_draw(flecs::world & world) {
    world.component<Color>()
            .member<unsigned char>("r")
            .member<unsigned char>("g")
            .member<unsigned char>("b")
            .member<unsigned char>("a");

    auto post_draw = world.entity("Draw")
            .add(flecs::Phase)
            .depends_on(flecs::OnStore);

    world.system()
            .kind(flecs::PreStore)
            .iter([](flecs::iter &it) {
                BeginDrawing();
                ClearBackground(WHITE);
            });

    world.system()
            .kind(post_draw)
            .iter([](flecs::iter &it) {
                EndDrawing();

                if (WindowShouldClose()) {
                    it.world().quit();
                }
            });
}