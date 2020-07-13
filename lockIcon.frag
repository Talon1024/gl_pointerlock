#version 330 core

uniform sampler2D iconTex;
in vec2 uv;
out vec4 gl_FragColor;

void main()
{
    gl_FragColor = texture(iconTex, uv);
}