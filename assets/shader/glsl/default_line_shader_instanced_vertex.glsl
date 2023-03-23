#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in mat4 aInstanceMatrix;
layout (location = 6) in vec4 aInstColor;

uniform mat4 projection;
uniform mat4 view;

out vec3 Color;

out VS_OUT {
    vec3 FragPos;
    vec3 Color;
} vs_out;

void main()
{
    Color = aColor;
    vs_out.FragPos = aPos;
    vs_out.Color = aInstColor.xyz;
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
