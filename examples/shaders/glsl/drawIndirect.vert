#version 460
#extension GL_EXT_nonuniform_qualifier: enable

layout (location = 0) in vec2 vertex_position;
layout (location = 1) in vec3 vertex_color;

layout (set = 0, binding = 0) uniform sampler2D textures[];

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
    fragData.color = vertex_color + texture(textures[gl_DrawID], vec2(0.5)).rgb;
    fragData.alpha = alphas[gl_DrawID];
    gl_Position = vec4(vertex_position,0,1);
}