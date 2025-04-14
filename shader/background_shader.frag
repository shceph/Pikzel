#version 330 core

const vec4 kBgColors[2] = vec4[2](
        vec4(131.0 / 255.0, 131.0 / 255.0, 131.0 / 255.0, 1.0),
        vec4(201.0 / 255.0, 201.0 / 255.0, 201.0 / 255.0, 1.0)
    );

layout(location = 0) out vec4 color;

in vec2 v_UV;

uniform float u_CanvasHeightOverWidth;

void main()
{
    float pattern_size = 55.0;
    vec2 uv_scaled = vec2(0);
    uv_scaled.x = v_UV.x * pattern_size;
    uv_scaled.y = v_UV.y * pattern_size * u_CanvasHeightOverWidth;
    int checker_square_color_id = int(mod(floor(uv_scaled.x) + floor(uv_scaled.y), 2.0F));
    color = kBgColors[checker_square_color_id];
}
