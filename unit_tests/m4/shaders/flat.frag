#version 330 core

in vec3 vWorldPos;

uniform vec3 uLightDir;  // world-space direction toward the light source
uniform vec3 uColor; // surface colour (felt green, cushion green, etc.)

out vec4 FragColor;

void main()
{
    // Derive the geometric face normal from screen-space position derivatives —
    // no normal data needed in the VBO; every triangle gets its exact flat normal.
    vec3 N = normalize(cross(dFdx(vWorldPos), dFdy(vWorldPos)));
    vec3 L = normalize(uLightDir);

    float ambient = 0.30;
    float diffuse = max(dot(N, L), 0.0);

    // No specular — felt and cushion rubber are matte surfaces
    vec3 color = uColor * (ambient + diffuse * 0.85);
    FragColor = vec4(color, 1.0);
}
