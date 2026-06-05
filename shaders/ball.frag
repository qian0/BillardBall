#version 330 core

in vec3 vWorldPos;  // interpolated world-space position from the vertex shader
in vec3 vNormal;    // interpolated world-space normal from the vertex shader
in vec2 vTexCoord;  // interpolated UV from the vertex shader

uniform vec3 uCameraPos;    // world-space eye position — used to compute the view direction for specular reflection
uniform vec3 uLightDir;     // world-space direction pointing toward the light source (not the surface) — kept as a directional light so shadows are uniform across the table
uniform vec3 uBallColor;    // base diffuse colour of the ball, used when no texture is bound
uniform sampler2D uTexture; // ball face texture containing the number label on a white disc

out vec4 FragColor; // final RGBA colour written to the framebuffer

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 R = reflect(-L, N);

    float ambient  = 0.15;
    float diffuse  = max(dot(N, L), 0.0);
    float specular = pow(max(dot(R, V), 0.0), 64.0);

    // Sample the ball texture; if the texture alpha is zero, fall back to the base ball colour
    vec4 texSample = texture(uTexture, vTexCoord);
    vec3 baseColor = mix(uBallColor, texSample.rgb, texSample.a);

    vec3 color = baseColor * (ambient + diffuse * 0.75) + vec3(specular * 0.6);
    FragColor = vec4(color, 1.0);
}
