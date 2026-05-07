attribute vec3 position;
attribute vec3 color;

varying vec3 vertexColor;

uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(position, 1.0);

    vertexColor = color;
}

