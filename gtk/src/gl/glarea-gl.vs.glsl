#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 vertexTexCoord;

uniform mat4 transform;

void main() {
	gl_Position = transform * vec4(position, 1.0);

	vertexTexCoord = texCoord;
}

