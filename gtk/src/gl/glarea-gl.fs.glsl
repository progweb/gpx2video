#version 330 core

in vec2 vertexTexCoord;

out vec4 fragColor;

uniform sampler2D inputTexture1;
uniform sampler2D inputTexture2;

void main() {
	fragColor = texture(inputTexture1, vertexTexCoord);
}
