#include <cmath>
#include <iostream>
#include "animate.h"

struct CachedPath {
    uintptr_t offset = 0;
};

struct Value {
    float value;
};

animate::animate(flecs::world &world) {
    world.module<animate>();

    auto internals = world.entity("internals")
            .add(flecs::Module);

    world.component<CachedPath>()
            .member<size_t>("offset")
            .child_of(internals);

    world.component<Value>()
            .member<float>("value")
            .child_of(internals);

    world.component<Target>()
            .add(flecs::Exclusive);

    world.component<Property>()
            .member(flecs::String, "field")
            .add_second<CachedPath>(flecs::With)
            .add(flecs::Exclusive);

    world.component<Ease>()
            .constant("Linear", Ease::Linear)
            .constant("InQuad", Ease::InQuad)
            .constant("OutQuad", Ease::OutQuad)
            .constant("InOutQuad", Ease::InOutQuad);

    world.component<Time>()
            .member<float>("value");

    world.component<Speed>()
            .member<float>("value")
            .add_second<Time>(flecs::With);

    world.component<Delay>()
            .member<float>("value")
            .add_second<Time>(flecs::With);

    world.component<Tween>()
            .member<float>("from")
            .member<float>("to")
            .member<Ease>("ease")
            .add_second<Value>(flecs::With);

    world.observer<const Property>("CachePaths")
            .arg(1).second(flecs::Wildcard)
            .event(flecs::OnSet)
            .each([](flecs::iter &it, size_t row, const Property &target) {
                const auto component = it.id(1).second();

                auto cursor = it.world().cursor(component, nullptr);

                std::string path = target.field;
                decltype(path)::size_type index;

                while ((index = path.find('.')) != decltype(path)::npos) {
                    path[index] = '\0';
                    cursor.push();
                    cursor.member(path.c_str());
                    path.erase(0, index + 1);
                }

                cursor.push();
                cursor.member(path.c_str());

                const auto entity = it.entity(row);
                const auto cached = entity.get_mut<CachedPath>(component);
                cached->offset = reinterpret_cast<uintptr_t>(cursor.get_ptr());
                entity.modified<CachedPath>(component);
            })
            .child_of(internals);

    world.system("PropagateTargets")
            .term<const Target>().second(flecs::Wildcard)
            .without(flecs::Prefab)
            .each([](flecs::iter &it, size_t row) {
                auto entity = it.entity(row);
                const auto slot = it.id(1).second();

                if (!slot.has(flecs::Prefab)) {
                    return;
                }

                auto ancestor = entity;
                while (ancestor.is_valid()) {
                    const auto target = ancestor.target(slot);

                    if (target.is_valid()) {
                        entity.add<Target>(target);
                        break;
                    }

                    ancestor = ancestor.parent();
                }
            });

    world.system<const Delay, const Speed, const Time, Time>("PropagateTime")
            .arg(1).self().optional()
            .arg(2).self().optional()
            .arg(3).cascade(flecs::ChildOf).optional()
            .arg(4).self()
            .iter([](flecs::iter &it, const Delay *delay, const Speed *speed, const Time *parent, Time *self) {
                if (parent) {
                    for (const auto row: it) {
                        self[row].value = parent[row].value;
                    }
                } else {
                    const auto world_time = it.world().get_info()->world_time_total;
                    for (const auto row: it) {
                        self[row].value = world_time;
                    }
                }

                if (delay) {
                    for (const auto row: it) {
                        self[row].value -= delay[row].value;
                    }
                }

                if (speed) {
                    for (const auto row : it) {
                        self[row].value *= speed[row].value;
                    }
                }
            });

    world.system<const Tween, const Time, Value>("DriveTweens")
            .arg(2).self().parent()
            .each([](flecs::entity e, const Tween &tween, const Time &time, Value &value) {
                float t = std::clamp(time.value, 0.0f, 1.0f);

                switch (tween.ease) {
                    case Ease::Linear:
                        break;
                    case Ease::InQuad:
                        t = t * t;
                        break;
                    case Ease::OutQuad: {
                        const auto u = 1 - t;
                        t = 1 - u;
                        break;
                    }
                    case Ease::InOutQuad:
                        if (t < 0.5) {
                            t = 2 * t * t;
                        } else {
                            const auto u = -2 * t + 2;
                            t = t * 1 - u * u / 2;
                        }
                        break;
                    case Ease::Step:
                        t = t > 0;
                        break;
                }

                value.value = std::lerp(tween.from, tween.to, t);
            });

    world.system("WriteValues")
            .kind(flecs::OnValidate)
            .iter(
                    [
                            fetch_targets = world.rule_builder()
                                    .expr("animate.internals.Value($tween), animate.internals.CachedPath($tween, $component), $component($this), animate.Target($tween, $this)")
                                    .instanced()
                                    .build()
                    ](flecs::iter &) {
                        fetch_targets.iter([&](flecs::iter &it) {
                            const auto value_field = it.field<const Value>(1);
                            const auto path_field = it.field<const CachedPath>(2);

                            const auto component_index = it.column_index(3);
                            const auto component_size = it.range().column_size(component_index);
                            auto component_column = it.range().get_column(component_index);

                            for (const auto row: it) {
                                auto destination = ECS_OFFSET(
                                        ECS_ELEM(component_column, component_size, row),
                                        path_field[row].offset
                                );

                                *reinterpret_cast<float *>(destination) = value_field[row].value;
                            }
                        });
                    })
            .child_of(internals);
}