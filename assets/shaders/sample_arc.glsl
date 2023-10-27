uniform vec3 SELF_center_radii[MAX_ELEMENTS];
uniform vec3 SELF_sincos_rotation[MAX_ELEMENTS];

void SELF_single(in uint index, inout vec2 uv, inout float d, inout vec4 color) {
    vec3 center_radius = SELF_center_radii[index];
    vec3 sincos_rotation = SELF_sincos_rotation[index];

    vec2 point = rotate(uv - center_radius.xy, sincos_rotation.z);

    // MIT License - https://iquilezles.org/articles/distfunctions2d/
    point.x = abs(point.x);
    float self = sincos_rotation.y * point.x > sincos_rotation.x * point.y
        ? length(point - sincos_rotation.xy * center_radius.z)
        : abs(length(point) - center_radius.z);

    d = min(d, self);
}

REPEAT(SELF)