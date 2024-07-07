#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aColor;
layout (location = 4) in vec2 aCoords;
  
out vec4 vertexColor; // specify a color output to the fragment shader

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor
    vertexColor = vec4(0.5, 0.0, 0.0, 1.0); // set the output variable to a dark-red color
}