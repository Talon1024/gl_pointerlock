#version 330 core

uniform vec2 windowSize;
uniform sampler2D iconTex;

in vec2 in_position;
in vec2 in_uv;

out vec2 uv;

void main()
{
    ivec2 iconSize = textureSize(iconTex, 0);
    vec2 scale = iconSize / windowSize;
    vec2 position = in_position * scale;
    vec4 vposition = vec4(position, 0., 1.);
    uv = in_uv;
    gl_Position = vposition;
}