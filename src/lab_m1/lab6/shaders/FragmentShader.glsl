#version 330

// Input
in vec3 frag_color;
in vec3 frag_normal;
in vec2 frag_texture;
in vec3 frag_position;

// TODO(student): Get values from vertex shader

// Output
layout(location = 0) out vec4 out_color;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_texture;
layout(location = 3) out vec3 out_position;

void main()
{
    // TODO(student): Write pixel out color
    out_color = vec4(frag_normal, 1);
    out_normal = frag_normal;
    out_texture = frag_texture;
    out_position = frag_position;
}
