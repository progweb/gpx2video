#version 330 core

in vec3 vertexColor;

out vec4 fragColor;

void main()
{
//   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
   fragColor = vec4(vertexColor, 1.0f);
}

