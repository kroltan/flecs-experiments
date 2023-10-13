#version 330

#define REPEAT(func) \
    uniform uint func##_count;\
    void func(inout vec2 uv, inout float d, inout vec4 color) {\
        for (uint index = 0u; index < func##_count; ++index) {\
            func##_single(index, uv, d, color);\
        }\
    }

#define MAX_ELEMENTS 128

in vec2 vertexTexCoord;
in vec4 fragColor;

void setup(out vec2 uv, out float d, out vec4 color) {
    uv = gl_FragCoord.xy;
    d = 1e32;
    color = fragColor;
}