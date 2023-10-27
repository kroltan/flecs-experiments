#version 440

#define REPEAT(func) \
    uniform int func##_count;\
    void func(inout vec2 uv, inout float d, inout vec4 color) {\
        uint count = uint(func##_count);\
        for (uint index = 0u; index < count; ++index) {\
            func##_single(index, uv, d, color);\
        }\
    }

#define MAX_ELEMENTS 32

in vec2 vertexTexCoord;
in vec4 fragColor;

void setup(out vec2 uv, out float d, out vec4 color) {
    uv = gl_FragCoord.xy;
    d = 1e32;
    color = fragColor;
    color.a = 0;
}

vec2 rotate(in vec2 point, float radians) {
    float s = sin(radians);
    float c = cos(radians);
    return mat2(c, s, -s, c) * point;
}