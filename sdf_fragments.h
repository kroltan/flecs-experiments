#pragma once

#include <sstream>
#include <vector>
#include <raylib.h>
#include "sdf.h"
#include "rlgl.h"

inline int get_shader_location(Shader shader, std::ostringstream &temp, const std::string_view &prefix,
                               const std::string_view &suffix) {
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
    flecs::rule<const sdf::Line> get_lines;
    std::vector<sdf::Line> buffer{};
    int start_ends_location;
    int count_location;

public:
    sample_line_driver(flecs::world &world, Shader shader, const std::string_view &identifier) {
        get_lines = world.rule_builder<const sdf::Line>()
                .term(flecs::IsA).second("$material")
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

        lines.each([this](const sdf::Line &line) {
            buffer.push_back(line);
        });

        const auto count = buffer.size();
        rlSetUniform(count_location, &count, SHADER_UNIFORM_INT, 1);
        rlSetUniform(start_ends_location, buffer.data(), SHADER_UNIFORM_VEC4, static_cast<int>(count));
    }
};

class sample_circle_driver : public shader_driver {
    flecs::rule<const sdf::Circle> get_circles;
    std::vector<sdf::Circle> buffer{};
    int center_radii_location;
    int count_location;

public:
    sample_circle_driver(flecs::world &world, Shader shader, const std::string_view &identifier) {
        get_circles = world.rule_builder<const sdf::Circle>()
                .term(flecs::IsA).second("$material")
                .build();

        std::ostringstream temp;
        center_radii_location = get_shader_location(shader, temp, identifier, "center_radii");
        count_location = get_shader_location(shader, temp, identifier, "count");
    }

    ~sample_circle_driver() override = default;

    void update(flecs::entity material) override {
        const auto circles = get_circles.iter()
                .set_var("material", material);

        buffer.clear();

        circles.each([this](const sdf::Circle &circle) {
            buffer.push_back(circle);
        });

        const auto count = buffer.size();
        rlSetUniform(count_location, &count, SHADER_UNIFORM_INT, 1);
        rlSetUniform(center_radii_location, buffer.data(), SHADER_UNIFORM_VEC3, static_cast<int>(count));
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
        const auto solid = self.get<const sdf::Solid>();
        rlSetUniform(color_location, &solid->color, SHADER_UNIFORM_VEC4, 1);
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