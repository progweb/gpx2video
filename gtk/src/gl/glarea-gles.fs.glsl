precision highp float;

varying vec2 vertexTexCoord;

uniform sampler2D inputTexture;

void main() {
	gl_FragColor = texture2D(inputTexture, vertexTexCoord);
}

