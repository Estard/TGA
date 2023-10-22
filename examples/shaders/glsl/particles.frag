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


layout (location = 0) in vec2 uv;
layout (location = 1) in vec4 pColor;
layout (location = 0) out vec4 color;

void main(){
    color = pColor*vec4(uv,1,1);
}