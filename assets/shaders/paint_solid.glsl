uniform vec4 SELF_color;

void SELF(inout vec2 uv, inout float d, inout vec4 color) {
    if (d > 0) {
        color = SELF_color;
    }

    color = vec4(d / 100, 0, 0, 0.5);
}