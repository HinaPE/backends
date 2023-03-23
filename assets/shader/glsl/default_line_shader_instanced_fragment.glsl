#version 330 core
out vec4 FragColor;
in vec3 Color;

uniform float opacity;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 Color;
} fs_in;

void main()
{
    FragColor = vec4(fs_in.Color, opacity);
}
