uniform vec4 SELF_color;

void SELF(inout vec2 uv, inout float d, inout vec4 color) {
    float blend = clamp(0.25 - d * 0.5, 0.0, 1.0);
    color = mix(color, SELF_color, blend * SELF_color.a);

//    float t = cos(d) / 2.0 + 0.5;
//    vec3 outside = mix(vec3(0.4, 0.3, 0.3), vec3(0.8, 0.6, 0.6), t);
//    vec3 inside = mix(vec3(0.3, 0.4, 0.3), vec3(0.6, 0.8, 0.6), t);
//    color = vec4((d > 0.0 ? outside : inside) / pow(abs(d), 0.), 1);
}