#include <flecs.h>
#include <raylib.h>
#include "sdf.h"
#include "draw.h"


int main(int argc, char *argv[]) {
    flecs::world world(argc, argv);
    InitWindow(800, 450, "Chillworld");

    world.component<float2>()
            .member<float>("x")
            .member<float>("y");

    setup_draw(world);

    world.set<flecs::Rest>({});
    world.import<flecs::monitor>();
    world.import<sdf>();

    world.plecs_from_file("assets/init.plecs");
    while (world.progress()) {}

    CloseWindow();
    return 0;
}
