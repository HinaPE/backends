#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aColor; // deprecated
layout (location = 4) in vec3 aTengent;
layout (location = 5) in vec3 aBiTengent;
layout (location = 6) in uint id;
layout (location = 7) in mat4 aInstanceMatrix;
layout (location = 11) in vec4 aInstColor;

uniform mat4 projection;
uniform mat4 view;
//uniform mat4 model; we don't need this

uniform int inst_id;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Color;
} vs_out;

flat out int instanceID;

void main()
{
    vs_out.FragPos = aPos;
    vs_out.Normal = mat3(transpose(inverse(aInstanceMatrix))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.Color = aColor;
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);

    instanceID = gl_InstanceID;

    if (inst_id == gl_InstanceID)
    {
        vs_out.Color = vec3(1.0, 0.3, 0.3);
    }
}
