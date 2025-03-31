#version 330 core

layout(location = 0) in vec2 a_Pos;

out vec2 v_UV;

uniform vec2 u_TopLeftInUV;

void main()
{
    gl_Position = vec4(a_Pos, 0.0, 1.0);
    v_UV = (vec2(a_Pos) + vec2(1.0)) / vec2(2.0);
    v_UV -= u_TopLeftInUV;
}
