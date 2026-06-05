#version 330 core

// Outputs a solid yellow-orange colour — confirms the fragment stage runs and the pipeline is wired correctly.
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.8, 0.2, 1.0);
}
