#version 140

uniform mat4 transform;

in  vec3 in_Position;

void main() {
  gl_Position = transform * vec4(in_Position, 1.0);
}
