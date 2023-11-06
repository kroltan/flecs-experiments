#pragma once

#include <sstream>
#include <vector>
#include <cmath>
#include <raylib.h>
#include "sdf.h"
#include "rlgl.h"
#include "transform.h"

inline int get_shader_location(
        Shader shader,
        std::ostringstream &temp,
        const std::string_view &prefix,
        const std::string_view &suffix
) {
    temp.str("");
    temp.clear();
    temp << prefix << '_' << suffix << '\0';
    auto location = GetShaderLocation(shader, temp.view().data());
    return location;
}

class shader_driver {
public:
    virtual void update(flecs::entity material) = 0;

    virtual ~shader_driver() = default;
};

class sample_line_driver : public shader_driver {
    flecs::rule<const sdf::Line, const transform::Global> get_lines;
    std::vector<sdf::Line> buffer{};
    int start_ends_location;
    int count_location;

public:
    sample_line_driver(flecs::world &world, Shader shader, const std::string_view &identifier) {
        get_lines = world.rule_builder<const sdf::Line, const transform::Global>()
                .term(flecs::IsA).second("$material")
                .without(flecs::Prefab)
                .build();

        std::ostringstream temp;
        start_ends_location = get_shader_location(shader, temp, identifier, "start_ends");
        count_location = get_shader_location(shader, temp, identifier, "count");
    }

    ~sample_line_driver() override = default;

    void update(flecs::entity material) override {
        const auto lines = get_lines.iter()
                .set_var("material", material);

        buffer.clear();

        lines.each([this](const sdf::Line &line, const transform::Global &transform) {
            buffer.emplace_back(sdf::Line {
                    .start = transform.apply(line.start),
                    .end = transform.apply(line.end)
            });
        });

        const auto count = static_cast<int>(buffer.size());
        rlSetUniform(count_location, &count, SHADER_UNIFORM_INT, 1);
        rlSetUniform(start_ends_location, buffer.data(), SHADER_UNIFORM_VEC4, count);
    }
};

class sample_arc_driver : public shader_driver {
    flecs::rule<const sdf::Arc, const transform::Global> get_arcs;
    std::vector<float> center_radii_buffer{};
    std::vector<float> sincos_rotation_buffer{};
    int center_radii_location;
    int sincos_rotation_location;
    int count_location;

public:
    sample_arc_driver(flecs::world &world, Shader shader, const std::string_view &identifier) {
        get_arcs = world.rule_builder<const sdf::Arc, const transform::Global>()
                .term(flecs::IsA).second("$material")
                .without(flecs::Prefab)
                .build();

        std::ostringstream temp;
        center_radii_location = get_shader_location(shader, temp, identifier, "center_radii");
        sincos_rotation_location = get_shader_location(shader, temp, identifier, "sincos_rotation");
        count_location = get_shader_location(shader, temp, identifier, "count");
    }

    ~sample_arc_driver() override = default;

    void update(flecs::entity material) override {
        const auto arcs = get_arcs.iter()
                .set_var("material", material);

        center_radii_buffer.clear();
        sincos_rotation_buffer.clear();

        arcs.each([this](const sdf::Arc &arc, const transform::Global &transform) {
            const auto half_end = arc.end / 2;
            const auto center = transform.apply(arc.center);

            center_radii_buffer.push_back(center.x);
            center_radii_buffer.push_back(center.y);
            center_radii_buffer.push_back(arc.radius * transform.scale);
            sincos_rotation_buffer.push_back(std::sin(half_end));
            sincos_rotation_buffer.push_back(std::cos(half_end));
            sincos_rotation_buffer.push_back(arc.start + half_end - transform.rotation);
        });

        assert(center_radii_buffer.size() == sincos_rotation_buffer.size());

        const auto count = static_cast<int>(center_radii_buffer.size() / 3);
        rlSetUniform(count_location, &count, SHADER_UNIFORM_INT, 1);
        rlSetUniform(center_radii_location, center_radii_buffer.data(), SHADER_UNIFORM_VEC3, count);
        rlSetUniform(sincos_rotation_location, sincos_rotation_buffer.data(), SHADER_UNIFORM_VEC3, count);
    }
};

class paint_solid_driver : public shader_driver {
    flecs::entity self;
    int color_location;

public:
    paint_solid_driver(flecs::entity self, Shader shader, const std::string_view &identifier) : self(self) {
        std::ostringstream temp;
        color_location = get_shader_location(shader, temp, identifier, "color");
    }

    ~paint_solid_driver() override = default;

    void update(flecs::entity material) override {

        const auto color = &self.get<const sdf::Solid>()->color;

        float floats[4]{};
        floats[0] = static_cast<float>(color->r) / 255;
        floats[1] = static_cast<float>(color->g) / 255;
        floats[2] = static_cast<float>(color->b) / 255;
        floats[3] = static_cast<float>(color->a) / 255;

        rlSetUniform(color_location, &floats, SHADER_UNIFORM_VEC4, 1);
    }
};

class effect_onion_driver : public shader_driver {
public:

    ~effect_onion_driver() override = default;

    void update(flecs::entity material) override {
    }
};

class effect_offset_driver : public shader_driver {
    flecs::entity self;
    int offset_location;

public:
    effect_offset_driver(flecs::entity self, Shader shader, const std::string_view &identifier) : self(self) {
        std::ostringstream temp;
        offset_location = get_shader_location(shader, temp, identifier, "offset");
    }

    ~effect_offset_driver() override = default;

    void update(flecs::entity material) override {
        const auto offset = self.get<const sdf::Offset>();
        rlSetUniform(offset_location, &offset->offset, SHADER_UNIFORM_FLOAT, 1);
    }
};