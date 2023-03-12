#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 Color;

uniform bool is_colored;
uniform bool is_textured;
uniform bool is_framebuffer;

uniform int diffuse_texture_num;
uniform int specular_texture_num;
uniform int normal_texture_num;
uniform int height_texture_num;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Color;
} fs_in;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool highlight_mode;
flat in int instanceID;

void main()
{
    vec3 out_color = vec3(0.0f, 0.0f, 0.0f);
    if (is_colored)
    {
        out_color += fs_in.Color;
    }

    if (is_textured)
    {
        out_color += texture(texture_diffuse1, fs_in.TexCoords).rgb;// diffuse map
    }

    // blinn-phong lighting
    vec3 ambient = 0.1 * out_color;// ambient

    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 norm = normalize(fs_in.Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * out_color;// diffuse

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * vec3(0.5, 0.5, 0.5);// specular

    out_color = ambient + diffuse + specular;

    float alpha = 1.0f;
    if (is_framebuffer) alpha = 0.5;

    if (highlight_mode) out_color *= 2;

    FragColor = vec4(out_color, alpha);
}
