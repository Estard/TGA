#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out FragData{
    vec3 color;
}fragData;


vec3[3] colors = vec3[](
    vec3(1,0,0),
    vec3(0,1,0),
    vec3(0,0,1)
);

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragData.color = colors[gl_VertexIndex];
}