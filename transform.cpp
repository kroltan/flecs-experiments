#include "transform.h"

transform::transform(flecs::world &world) {
    world.module<transform>();

    world.component<Transform>()
            .member<float2, flecs::units::length::Pixels>("position")
            .member<float, flecs::units::angle::Radians>("rotation")
            .member<float>("scale");

    world.component<Local>().is_a<Transform>()
            .add_second<Global>(flecs::With);

    world.component<Global>()
            .is_a<Transform>()
            .add(EcsAlwaysOverride);

    world.system<const Local, const Global, Global>("propagate_global")
            .arg(1)
            .arg(2).parent().cascade().optional()
            .arg(3).self()
            .kind(flecs::OnValidate)
            .instanced()
            .iter([](
                    flecs::iter &it,
                    const Local *const locals,
                    const Global *const parents,
                    Global *const each
            ) {
                if (parents && it.is_self(2)) {
                    for (const auto row: it) {
                        const auto &parent = parents[row];
                        const auto &local = locals[row];

                        auto &self = each[row];
                        self.position = parent.apply(local.position);
                        self.rotation = parent.rotation + local.rotation;
                        self.scale = parent.scale * local.scale;
                    }
                } else if (parents) {
                    const auto &parent = *parents;

                    for (const auto row: it) {
                        const auto &local = locals[row];

                        auto &self = each[row];
                        self.position = parent.apply(local.position);
                        self.rotation = parent.rotation + local.rotation;
                        self.scale = parent.scale * local.scale;
                    }
                } else {
                    for (const auto row: it) {
                        const auto &local = locals[row];

                        auto &self = each[row];
                        self.position = local.position;
                        self.rotation = local.rotation;
                        self.scale = local.scale;
                    }
                }
            });

    world.system<const Global, Global>("inherit_global")
            .arg(1).parent().cascade().optional()
            .arg(2).self()
            .without<Local>().self()
            .kind(flecs::OnValidate)
            .instanced()
            .iter([](flecs::iter &it, const Global *const parents, Global *const each){
                if (parents && it.is_self(1)) {
                    for (const auto row : it) {
                        each[row] = parents[row];
                    }
                } else if (parents) {
                    const auto &parent = *parents;

                    for (const auto row : it) {
                        each[row] = parent;
                    }
                } else {
                    for (const auto row : it) {
                        each[row] = {};
                    }
                }
            });
}

float2 transform::Transform::apply(const float2 local) const {
    float2 down = float2::direction(rotation);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ArgumentSelectionDefects"
    const float2 right{-down.y, down.x};
#pragma clang diagnostic pop

    return position + (right * local.y - down * local.x) * scale;
}