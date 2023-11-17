#version 460


layout (location = 0) in FragData{
    vec3 color;
    float alpha;
} fragData;

layout(location = 0) out vec4 color;

void main(){
    color = vec4(fragData.color,fragData.alpha);
}