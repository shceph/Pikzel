#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_UV;

uniform sampler2D u_Texture;
uniform int u_Opacity;

void main()
{
    color = texture(u_Texture, v_UV);
    color.a *= float(u_Opacity) / 255.0;
}
