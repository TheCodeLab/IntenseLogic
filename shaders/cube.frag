#version 140

out vec4 gl_FragColor;
 
void main(void) {
    // Pass through our original color with full opacity.
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
