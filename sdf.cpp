#include <unordered_map>
#include <functional>
#include <ranges>
#include <sstream>
#include <memory>
#include <iostream>
#include "sdf.h"
#include "draw.h"
#include "util.h"
#include "sdf_fragments.h"

struct PendingMaterial {
};

struct Shared {
};

struct Fragment {
    std::string file_name;
    std::function<std::unique_ptr<shader_driver>(flecs::entity self, Shader shader,
                                                 const std::string_view &identifier)> factory;
};

struct ShaderFragment {
    flecs::entity subject;
    std::string code;
    std::string identifier;
    decltype(Fragment::factory) factory;
};

struct PendingFragment {
    ShaderFragment fragment;
};

struct SdfMaterial {
    Shader shader{.id = 0};
    std::vector<std::shared_ptr<shader_driver>> fragments{};
};

ShaderFragment transform_shader(flecs::entity instance, flecs::entity component) {
    const auto fragment = component.get<Fragment>();

    std::ostringstream identifier;
    identifier << component.name() << "_" << instance.raw_id();

    std::string content = read_file_to_end(fragment->file_name);
    replace_all(content, "SELF", identifier.view());

    std::ostringstream code;

    code << "// " << component.name() << ": " << fragment->file_name << "\n"
         << content;

    return {
            .subject = instance,
            .code {code.str()},
            .identifier {identifier.str()},
            .factory {fragment->factory},
    };
}

std::vector<ShaderFragment> collect_preamble(const flecs::world &world) {
    std::vector<ShaderFragment> result;
    result.emplace_back(ShaderFragment{
            .subject = world.entity("Preamble"),
            .code = read_file_to_end("assets/shaders/preamble.glsl"),
            .identifier = "setup",
            .factory = nullptr,
    });

    world.filter_builder()
            .term<const Shared>()
            .term<const Fragment>()
            .each([&](flecs::entity e) {
                result.push_back(transform_shader(e, e));
            });

    return result;
}

void stitch(
        std::ostringstream &code,
        std::ostringstream &main,
        const ShaderFragment &fragment,
        std::vector<ShaderFragment> &out_create
) {
    code << "\n\n" << fragment.code;
    main << "\t" << fragment.identifier << "(uv, d, color);\n";

    if (fragment.factory) {
        out_create.emplace_back(fragment);
    }
}

std::string stitch_shader(
        const flecs::rule<const PendingFragment, const sdf::Order> &pending_fragments,
        const std::vector<ShaderFragment> &preamble,
        flecs::entity material,
        std::vector<ShaderFragment> &out_create
) {
    std::vector<std::tuple<int16_t, ShaderFragment>> fragments;

    material.remove<PendingMaterial>();

    pending_fragments.iter()
            .set_var("material", material)
            .each([&](flecs::entity child, const PendingFragment &fragment, const sdf::Order &order) {
                const auto index = std::find_if(
                        fragments.begin(),
                        fragments.end(),
                        [&](const std::tuple<int16_t, ShaderFragment> &it) {
                            return std::get<0>(it) >= order.order;
                        }
                );

                fragments.emplace_back(order.order, fragment.fragment);
                child.remove<PendingFragment>();
            });

    std::sort(fragments.begin(), fragments.end(), [](auto &a, auto &b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    std::ostringstream code;
    std::ostringstream main;
    main << "\n\n// Generated\n"
         << "out vec4 finalColor;\n"
         << "void main() {\n"
         << "\tvec2 uv;\n"
         << "\tfloat d;\n"
         << "\tvec4 color;\n\n";

    for (const auto &fragment: preamble) {
        stitch(code, main, fragment, out_create);
    }

    for (const auto &[_, fragment]: fragments) {
        stitch(code, main, fragment, out_create);
    }

    main << "\n\tfinalColor = color;\n"
         << "}\n";
    code << main.view();

    return code.str();
}

sdf::sdf(flecs::world &world) {
    world.component<Line>()
            .member<float2, flecs::units::length::Pixels>("start")
            .member<float2, flecs::units::length::Pixels>("end")
            .add_second<transform::Global>(flecs::With)
            .set(Fragment{
                    .file_name = "assets/shaders/sample_line.glsl",
                    .factory = [&](auto self, auto shader, const auto &identifier) {
                        return std::make_unique<sample_line_driver>(world, shader, identifier);
                    },
            })
            .add<Shared>();

    world.component<Arc>()
            .member<float2, flecs::units::length::Pixels>("center")
            .member<float, flecs::units::length::Pixels>("radius")
            .member<float, flecs::units::angle::Radians>("start")
            .member<float, flecs::units::angle::Radians>("end")
            .add_second<transform::Global>(flecs::With)
            .set(Fragment{
                    .file_name = "assets/shaders/sample_arc.glsl",
                    .factory = [&](auto self, auto shader, const auto &identifier) {
                        return std::make_unique<sample_arc_driver>(world, shader, identifier);
                    },
            })
            .add<Shared>();

    world.component<Solid>()
            .member<Color>("color")
            .set(Fragment{
                    .file_name = "assets/shaders/paint_solid.glsl",
                    .factory = [](auto self, auto shader, const auto &identifier) {
                        return std::make_unique<paint_solid_driver>(self, shader, identifier);
                    },
            });

    world.component<Onion>()
            .set(Fragment{
                    .file_name = "assets/shaders/effect_onion.glsl",
                    .factory = [&](auto self, auto shader, const auto &identifier) {
                        return std::make_unique<effect_onion_driver>();
                    },
            });

    world.component<Offset>()
            .member<float, flecs::units::length::Pixels>("offset")
            .set(Fragment{
                    .file_name = "assets/shaders/effect_transform.glsl",
                    .factory = [&](auto self, auto shader, const auto &identifier) {
                        return std::make_unique<effect_offset_driver>(self, shader, identifier);
                    },
            });

    world.component<Order>()
            .member<int16_t>("order");

    world.component<SdfMaterial>();
    world.component<Shared>();
    world.component<Fragment>();

    world.component<PendingMaterial>().add(flecs::DontInherit);
    world.component<PendingFragment>().add(flecs::DontInherit);

    const auto preamble = collect_preamble(world);

    world.observer<Fragment>()
            .event(flecs::OnAdd)
            .yield_existing()
            .each([](flecs::entity component, const Fragment &fragment) {
                component.world()
                        .observer()
                        .event(flecs::OnSet)
                        .with(component)
                        .with(flecs::ChildOf, flecs::Wildcard)
                        .term(flecs::Prefab).filter()
                        .each([=](flecs::iter &it, size_t row) {
                            auto entity = it.entity(row);
                            const auto component = it.world().component(it.id(1));
                            auto parent = it.id(2).second();

                            entity.set(PendingFragment{
                                    .fragment = transform_shader(entity, component),
                            });

                            parent.add<PendingMaterial>();
                        });
            });

    world.system("compile")
            .term<const PendingMaterial>()
            .term(flecs::Prefab)
            .no_readonly()
            .each(
                    [
                            preamble,
                            pending_fragments = world.rule_builder<const PendingFragment, const Order>()
                                    .term(flecs::ChildOf).second("$material")
                                    .term(flecs::Prefab)
                                    .instanced()
                                    .build()
                    ](flecs::entity material) {
                        {
                            const auto render = material.get<SdfMaterial>();
                            if (render) {
                                UnloadShader(render->shader);
                            }
                        }

                        std::vector<ShaderFragment> create;
                        const auto code = stitch_shader(pending_fragments, preamble, material, create);
                        const auto shader = LoadShaderFromMemory(nullptr, code.c_str());

                        decltype(SdfMaterial::fragments) update;
                        for (const auto &fragment: create) {
                            update.emplace_back(fragment.factory(fragment.subject, shader, fragment.identifier));
                        }

                        auto render = material.get_mut<SdfMaterial>();
                        render->fragments = std::move(update);
                        render->shader = shader;
                    }
            );

    world.system<const SdfMaterial>("draw")
            .term(flecs::Prefab)
            .kind(flecs::OnStore)
            .each([](flecs::entity material, const SdfMaterial &material_data) {
                rlEnableShader(material_data.shader.id);
                for (auto &fragment: material_data.fragments) {
                    fragment->update(material);
                }

                BeginShaderMode(material_data.shader);
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
                EndShaderMode();
            });
}

