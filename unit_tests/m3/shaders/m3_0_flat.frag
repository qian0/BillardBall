#version 330 core

in vec3 vWorldPos;

uniform vec3 uLightDir;
uniform vec3 uColor;

out vec4 FragColor;

void main()
{
    // Derive geometric face normal from screen-space derivatives — no normals in VBO.
    vec3 N = normalize(cross(dFdx(vWorldPos), dFdy(vWorldPos)));
    vec3 L = normalize(uLightDir);

    float ambient = 0.30;
    float diffuse = max(dot(N, L), 0.0);

    vec3 color = uColor * (ambient + diffuse * 0.85);
    FragColor = vec4(color, 1.0);
}
