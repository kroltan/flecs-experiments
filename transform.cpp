#include "transform.h"

transform::transform(flecs::world &world) {
    world.module<transform>();

    world.component<Global>()
            .member<float2, flecs::units::length::Pixels>("position")
            .member<float, flecs::units::angle::Radians>("rotation_radians")
            .member<float>("scale")
            .add(EcsAlwaysOverride);

    world.component<Position>()
            .member<float2, flecs::units::length::Pixels>("value")
            .add_second<Global>(flecs::With);

    world.component<Rotation>()
            .member<float, flecs::units::angle::Radians>("radians")
            .add_second<Global>(flecs::With);

    world.component<Scale>()
            .member<float>("value")
            .add_second<Global>(flecs::With);

    world.system<const Position, const Rotation, const Scale, const Global, Global>("ApplyGlobal")
            .arg(1).optional()
            .arg(2).optional()
            .arg(3).optional()
            .arg(4).parent().cascade().optional()
            .arg(5).self()
            .instanced()
            .iter([](
                    flecs::iter &it,
                    const Position *const positions,
                    const Rotation *const rotations,
                    const Scale *const scales,
                    const Global *const parents,
                    Global *const each
            ) {
                for (const auto row: it) {
                    const auto &parent = parents ? *parents : Global{};
                    auto &self = each[row];

                    self.position =
                            positions
                            ? parent.local_to_global(positions[row].value)
                            : parent.position;

                    self.rotation_radians =
                            rotations
                            ? parent.rotation_radians + rotations[row].radians
                            : parent.rotation_radians;

                    self.scale =
                            scales
                            ? parent.scale * scales[row].value
                            : parent.scale;
                }
            });
}

float2 transform::Global::local_to_global(const float2 local) const {
    float2 down = float2::direction(rotation_radians);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ArgumentSelectionDefects"
    const float2 right{-down.y, down.x};
#pragma clang diagnostic pop

    return position + (right * local.y - down * local.x) * scale;
}