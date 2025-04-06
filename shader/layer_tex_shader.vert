#version 330 core

layout(location = 0) in vec2 a_Pos;
layout(location = 1) in vec2 a_UV;

out vec2 v_UV;

uniform mat4 u_ViewProjection;

void main()
{
    gl_Position = u_ViewProjection * vec4(a_Pos, 0.0F, 1.0F);
    v_UV = a_UV;
}
