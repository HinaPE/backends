#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 2) in vec3 aColor;
layout (location = 4) in vec3 aTengent;
layout (location = 5) in vec3 aBiTengent;
layout (location = 6) in uint id;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoords;
out vec3 Color;

void main()
{
    TexCoords = aTexCoords;
    Color = aColor;
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
