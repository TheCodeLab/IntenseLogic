#version 140

in vec3 in_Position;

flat out int instanceID;

//uniform MVPBlock {
    uniform mat4 mvp;//[1024];
    uniform float radius;
//};

void main()
{
    vec4 pos = mvp/*[gl_InstanceID]*/ * vec4(in_Position * radius, 1.0);
    gl_Position = vec4(pos.xyz/pos.w, 1.0);
    instanceID = gl_InstanceID;
}

