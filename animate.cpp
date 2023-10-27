#include <cmath>
#include <iostream>
#include <type_traits>
#include "animate.h"

struct CachedPath {
    uintptr_t offset = 0;
};

struct Progress {
    float t;
};

template<typename T>
concept ease = requires(const T self, float t) {
    requires std::is_base_of<animate::Ease, T>::value;
    { self.ease(t) } -> std::same_as<decltype(t)>;
};

template<ease T>
flecs::entity add_ease_type(flecs::world & world) {
    world.component<T>()
            .template is_a<animate::Ease>()
            .template add_second<Progress>(flecs::With);

    return world.system<const animate::Time, const T, Progress>()
            .arg(1).self().up(flecs::ChildOf)
            .each([](const animate::Time &time, const T &self, Progress &progress) {
                const auto t = (time.value - self.start) / self.duration;

                if (t < 0 || t > 1) {
                    return;
                }

                progress.t = self.ease(t);
            });
}

animate::animate(flecs::world &world) {
    world.module<animate>();

    auto internals = world.entity("internals")
            .add(flecs::Module);

    world.component<CachedPath>()
            .member<size_t>("offset")
            .child_of(internals);

    world.component<Progress>()
            .member<float>("t")
            .child_of(internals);

    world.component<animate::Target>();

    world.component<animate::Property>()
            .member(flecs::String, "field")
            .add(flecs::Exclusive)
            .add_second<CachedPath>(flecs::With);

    world.component<animate::Ease>()
            .member<float>("start")
            .member<float>("duration")
            .add(flecs::Exclusive);

    world.component<animate::Time>()
            .member<float>("value");

    world.component<animate::Tween>()
            .member<float>("from")
            .member<float>("to");

    add_ease_type<animate::Linear>(world).child_of(internals);
    add_ease_type<animate::InQuad>(world).child_of(internals);
    add_ease_type<animate::OutQuad>(world).child_of(internals);
    add_ease_type<animate::InOutQuad>(world).child_of(internals);

    world.observer<const animate::Property>("CachePaths")
            .arg(1).second(flecs::Wildcard)
            .event(flecs::OnSet)
            .each([](flecs::iter &it, size_t row, const animate::Property &target) {
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

    world.system<animate::Time>("TestDrive").each([](flecs::entity entity, animate::Time &p) {
        const auto time = entity.world().get_info()->world_time_total;
        p.value = std::abs(std::sin(time));
        entity.modified<animate::Time>();
    });

    auto fetch_targets = world.rule_builder()
            .expr("animate.Tween($tween), animate.internals.CachedPath($tween, $component), $component($this), animate.Target($tween, $this)")
            .instanced()
            .build();

    auto fetch_progress = world.query_builder<const Progress>()
            .arg(1).self().cascade(flecs::ChildOf)
            .instanced()
            .build();

    world.system("Update").iter(
                    [
                            fetch_targets,
                            target_tween_var = fetch_targets.find_var("tween"),
                            fetch_progress,
                            progress_this_var = ecs_filter_find_this_var(fetch_progress.filter())
                    ](flecs::iter &) {
                        fetch_targets.iter([&](flecs::iter &target_it) {
                            const auto tween_field = target_it.field<const animate::Tween>(1);
                            const auto path_field = target_it.field<const CachedPath>(2);

                            const auto component_index = target_it.column_index(3);
                            const auto component_size = target_it.table().column_size(component_index);
                            auto component_column = target_it.table().get_column(component_index);

                            for (const auto row: target_it) {
                                const auto &tween = tween_field[row];

                                auto raw = ECS_OFFSET(ECS_ELEM(component_column, component_size, row),
                                                      path_field[row].offset);
                                auto destination = reinterpret_cast<float *>(raw);

                                fetch_progress.iter()
                                        .set_var(progress_this_var, target_it.get_var(target_tween_var))
                                        .each([&](const Progress &progress) {
                                            const auto value = std::lerp(tween.from, tween.to, progress.t);
                                            *destination = value;
                                        });
                            }
                        });
                    }
            )
            .child_of(internals);
}