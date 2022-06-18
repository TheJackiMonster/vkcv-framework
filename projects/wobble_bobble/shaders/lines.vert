#version 450

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main() {
    const vec3 positions[8] = {
        vec3(0.0f, 0.0f, 0.0f),
        vec3(1.0f, 0.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f),
        vec3(1.0f, 1.0f, 0.0f),
        vec3(0.0f, 0.0f, 1.0f),
        vec3(1.0f, 0.0f, +1.0f),
        vec3(0.0f, 1.0f, 1.0f),
        vec3(1.0f, 1.0f, 1.0f)
    };

    gl_Position = mvp * vec4(positions[gl_VertexIndex], 1);
}