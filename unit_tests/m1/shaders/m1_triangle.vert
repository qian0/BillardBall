#version 330 core

// Passes the vertex position straight through to clip space — no transform needed for an NDC triangle.
layout(location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
