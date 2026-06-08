#version 330 core

in vec2 vUV;

uniform sampler2D uTexture;
uniform vec3 uColor; // text colour

out vec4 FragColor;

void main()
{
    float alpha = texture(uTexture, vUV).r;
    FragColor = vec4(uColor, alpha);
}
