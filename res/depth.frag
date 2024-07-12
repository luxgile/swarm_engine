#version 330 core

uniform mat4 mvp;

void main()
{             
     gl_FragDepth = gl_FragCoord.z;
}  