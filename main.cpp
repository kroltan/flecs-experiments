#include <flecs.h>
#include <raylib.h>
#include "sdf.h"
#include "draw.h"
#include "animate.h"
#include "transform.h"


int main(int argc, char *argv[]) {
    flecs::world world(argc, argv);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(800, 450, "Chillworld");

    world.component<float2>()
            .member<float>("x")
            .member<float>("y");

    setup_draw(world);

    world.import<flecs::units>();
    world.import<transform>();
    world.import<sdf>();
    world.import<animate>();

    world.plecs_from_file("assets/init.plecs");
    world.app()
            .enable_rest()
            .enable_monitor()
            .run();

    CloseWindow();
    return 0;
}
