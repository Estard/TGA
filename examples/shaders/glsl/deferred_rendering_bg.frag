#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 cc1;
layout (location = 1) out vec4 cc2;
layout (location = 2) out float cc3;
layout (location = 3) out float cc4;

void main() 
{
    cc1 = vec4(1,0,0.0,0.0);
    cc2 = vec4(0,1,0.0,0.0);
    cc3 = 1;
    cc4 = 1;
}
