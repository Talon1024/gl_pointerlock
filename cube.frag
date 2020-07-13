#version 330 core

in vec3 colour;
out vec4 gl_FragColor;

void main()
{
    gl_FragColor = vec4(colour, 1.0);
}
