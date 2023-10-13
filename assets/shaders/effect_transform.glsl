uniform float SELF_offset;

void SELF(inout vec2 uv, inout float d, inout vec4 color) {
    d += SELF_offset;
}