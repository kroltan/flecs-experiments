uniform vec4[MAX_ELEMENTS] SELF_start_ends;

void SELF_single(in uint index, inout vec2 uv, inout float d, inout vec4 color) {
    vec4 line = SELF_start_ends[index];
    vec2 uv_to_start = uv - line.xy;
    vec2 end_to_start = line.zw - line.xy;
    float projection = clamp(dot(uv_to_start, end_to_start) / dot(end_to_start, end_to_start), 0.0, 1.0);
    d = min(d, length(uv_to_start - end_to_start * projection));
}

REPEAT(SELF)
