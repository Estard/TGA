#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 vertex_position;
layout (location = 1) in vec3 vertex_color;


layout (location = 0) out FragData{
    vec3 color;
} fragData;

void main(){
    fragData.color = vertex_color;
    gl_Position = vec4(vertex_position,0,1);
}