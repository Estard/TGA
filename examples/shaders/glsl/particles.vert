#version 450
#extension GL_ARB_separate_shader_objects : enable


struct Particle{
    vec4 position; //position.w -> scale of particle;
    vec4 color;
    uint index;
};

layout(set = 0, binding = 0) uniform Camera{
    mat4 view;
    mat4 projection;
}camera;

layout(set = 0, binding = 1) readonly buffer Particles{
    Particle at[];
}particles;


layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;


vec3 triangulation[6] = vec3[](
    vec3(-.5,.5,0),
    vec3(.5,.5,0),
    vec3(-.5,-.5,0),

    vec3(-.5,-.5,0),
    vec3(.5,.5,0),
    vec3(.5,-.5,0)
);

vec2 uvs[6] = vec2[](
    vec2(0,0),
    vec2(1,0),
    vec2(0,1),

    vec2(0,1),
    vec2(1,0),
    vec2(1,1)
);

void main() 
{
    uint pIndex = gl_VertexIndex/6;
    uint vIndex = gl_VertexIndex%6;
    Particle p = particles.at[pIndex];
    vec3 vertex = triangulation[vIndex];
    outUV = uvs[vIndex];
    outColor = p.color;

    vec3 camRight = vec3(camera.view[0][0],camera.view[1][0],camera.view[2][0]);
    vec3 camUp = vec3(camera.view[0][1],camera.view[1][1],camera.view[2][1]);

    vec3 position = p.position.xyz+camRight*vertex.x*p.position.w+camUp*vertex.y*p.position.w;

    gl_Position = camera.projection*camera.view*vec4(position,1);
}
