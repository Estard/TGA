#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D cc1;
layout(set = 0, binding = 1) uniform sampler2D cc2;
layout(set = 0, binding = 2) uniform sampler2D cc3;
layout(set = 0, binding = 3) uniform sampler2D cc4;

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 color;

void main() 
{
    vec4 colCc1 = texture(cc1,uv);
    vec4 colCc2 = texture(cc2,uv);
    float colCc3 = texture(cc3,uv).r;
    float colCc4 = texture(cc4,uv).r;
    color = vec4(colCc1+colCc2)+vec4(0,0,colCc3,colCc4);
}
