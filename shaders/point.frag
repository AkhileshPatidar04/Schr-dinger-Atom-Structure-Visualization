#version 330 core
in float vSign;
out vec4 FragColor;

void main()
{
    vec2 c = gl_PointCoord - vec2(0.5);
    float d = length(c);
    if (d > 0.5) discard;

    float alpha = smoothstep(0.5, 0.0, d) * 0.85;

    vec3 posColor = vec3(0.35, 0.65, 1.0); // positive lobe - blue
    vec3 negColor = vec3(1.0, 0.45, 0.35); // negative lobe - red
    vec3 color = vSign >= 0.0 ? posColor : negColor;

    FragColor = vec4(color, alpha);
}
