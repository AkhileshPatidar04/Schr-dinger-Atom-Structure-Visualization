#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aSign;

uniform mat4 view;
uniform mat4 projection;
uniform float basePointSize;
uniform vec3 viewPos;

out float vSign;

void main()
{
    vec4 worldPos = vec4(aPos, 1.0);
    gl_Position = projection * view * worldPos;

    float dist = length(viewPos - aPos);
    gl_PointSize = clamp(basePointSize * (8.0 / (dist + 1.0)), 1.0, 14.0);

    vSign = aSign;
}
