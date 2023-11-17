#version 460

layout (location = 0) in vec2 vertex_position;
layout (location = 1) in vec3 vertex_color;


layout (location = 0) out FragData{
    vec3 color;
    float alpha;
} fragData;

float alphas[3] = float[3](
    0.3,
    0.1,
    1.0
);

void main(){
    fragData.color = vertex_color;
    fragData.alpha = alphas[gl_DrawID];
    gl_Position = vec4(vertex_position,0,1);
}