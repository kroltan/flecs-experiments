uniform vec3 SELF_center_radii[MAX_ELEMENTS];

void SELF_single(in uint index, inout vec2 uv, inout float d, inout vec4 color) {
    vec3 circle = SELF_center_radii[index];
    d = min(d, distance(uv, circle.xy) - circle.z);
}

REPEAT(SELF)