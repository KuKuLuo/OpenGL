#version 330 core

layout(location = 0) in vec2 uv;  

void main()
{
    gl_Position = vec4(uv, 0.0f, 1.0f);
}


