attribute vec4 position;

//uniform mat4 mvp;
//
//void main() {
//  gl_Position = mvp * position;
//}

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 vertexTexCoord;

//uniform mat4 mvp;

void main() {
	//gl_Position = mvp * position;
//	gl_Position = vec4(inputPosition.x, inputPosition.y, inputPosition.z, 1.0);
	gl_Position = vec4(position, 1.0);

	vertexTexCoord = texCoord;
}

