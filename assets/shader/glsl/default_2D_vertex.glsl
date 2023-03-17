#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 Color;

uniform vec2 pos;
uniform float rot;
uniform vec2 scl;

uniform vec2 iResolution;

void main()
{
    vec2 pos_scaled = aPos * scl;
    vec2 pos_rotated = vec2(pos_scaled.x * cos(rot) - pos_scaled.y * sin(rot), pos_scaled.x * sin(rot) + pos_scaled.y * cos(rot));
    vec2 pos_world = pos_rotated + pos;
    gl_Position = vec4(pos_world.x / iResolution.x * 2, pos_world.y / iResolution.y * 2, 0.0, 1.0);
    Color = aColor;
}