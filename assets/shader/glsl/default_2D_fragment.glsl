#version 330 core
out vec4 FragColor;

uniform bool is_colored;
uniform bool is_textured;

in vec2 TexCoords;
in vec3 Color;

uniform sampler2D texture1;

void main()
{
    vec3 out_color = vec3(0.0f, 0.0f, 0.0f);

    if (is_colored)
    {
        out_color += Color;
    }

    if (is_textured)
    {
        out_color += texture(texture1, TexCoords).rgb;// diffuse map
    }

    FragColor = vec4(out_color, 1.0f);
}