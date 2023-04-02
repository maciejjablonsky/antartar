#version 450

layout(location = 0) in vec2 input_position;
layout(location = 1) in vec3 input_color;

layout(location = 0) out vec3 fragment_color;

void main()
{
    gl_Position    = vec4(input_position, 0.0, 1.0);
    fragment_color = input_color;
}
