#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 in_position;
in float in_colour;

// Convert HSV to RGB, assume all values are within range of 0.0 to 1.0
vec3 hsvToRgb(float hue, float sat, float val)
{
    float chroma = val * sat;
    int section = int(floor(hue * 6));
    if(section >= 0 && section <= 5)
    {
        float x = chroma * (1. - abs(section % 2 - 1));
        vec3 rgbs[6] = vec3[](
            vec3(chroma, x, 0.0),
            vec3(x, chroma, 0.0),
            vec3(0.0, chroma, x),
            vec3(0.0, x, chroma),
            vec3(x, 0.0, chroma),
            vec3(chroma, 0.0, x)
        );
        return rgbs[section];
    }
    else
    {
        return vec3(0.);
    }
}

out vec3 colour;

void main()
{
    vec4 position = vec4(in_position, 1.0);
    colour = hsvToRgb(in_colour, 1., 1.);
    gl_Position = projection * view * model * position;
}
