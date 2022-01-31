#version 450

layout(location = 0) in vec3 vertexPos;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main() {
    gl_Position = mvp * vec4(vertexPos, 1);
}