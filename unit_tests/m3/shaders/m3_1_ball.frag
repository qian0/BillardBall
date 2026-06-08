#version 330 core

in vec3 vWorldPos;  // interpolated world-space position from the vertex shader
in vec3 vNormal;    // interpolated world-space normal from the vertex shader

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

    // Decal projection: map the top hemisphere (+Y axis) to UV space.
    vec2 decalUV = N.xz + 0.5;
    float facing = N.y;
    float decalAlpha = 0.0;
    vec3 decalColor = vec3(0.0);
    if (facing > 0.0 && decalUV.x >= 0.0 && decalUV.x <= 1.0 &&
                         decalUV.y >= 0.0 && decalUV.y <= 1.0) {
        vec4 texSample = texture(uTexture, decalUV);
        // Fade the decal out near the equator so there is no hard edge
        decalAlpha = texSample.a * clamp(facing * 4.0, 0.0, 1.0);
        decalColor = texSample.rgb;
    }
    vec3 baseColor = mix(uBallColor, decalColor, decalAlpha);

    vec3 color = baseColor * (ambient + diffuse * 0.75) + vec3(specular * 0.6);
    FragColor = vec4(color, 1.0);
}
