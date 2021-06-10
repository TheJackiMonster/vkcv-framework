#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(points) in;
layout (triangle_strip, max_vertices = 24) out;

layout( push_constant ) uniform constants{
    mat4 viewProjection;
};

layout(location = 0) in float passCubeHalf[1];

layout(location = 0) out vec3 passColor;

void main()	{
    float cubeHalf = passCubeHalf[0];
    // right
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1, 1, 1), 1);
    passColor = vec3(1, 0, 0);
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1, 1, -1), 1);
    passColor = vec3(1, 0, 0);
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1, -1, 1), 1);
    passColor = vec3(1, 0, 0);
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1, -1, -1), 1);
    passColor = vec3(1, 0, 0);
    EmitVertex();
    EndPrimitive();
    // left
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, 1, 1), 1);
    passColor = vec3(1, 0, 0);
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, 1, -1), 1);
    passColor = vec3(1, 0, 0);
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, -1, 1), 1);
    passColor = vec3(1, 0, 0);
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, -1, -1), 1);
    passColor = vec3(1, 0, 0);
    EmitVertex();
    EndPrimitive();
    // back
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,   1, -1), 1);
    passColor = vec3(1, 0, 0);
    EmitVertex();                                                                  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,  -1, -1), 1);
    passColor = vec3(1, 0, 0);
    EmitVertex();                                                                  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1,  1, -1), 1);
    passColor = vec3(1, 0, 0);
    EmitVertex();                                                                  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, -1, -1), 1);
    passColor = vec3(0, 0, 1);
    EmitVertex();
    EndPrimitive();
    // front
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,   1, 1), 1);
    passColor = vec3(0, 0, 1);
    EmitVertex();                                                                  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,  -1, 1), 1);
    passColor = vec3(0, 0, 1);
    EmitVertex();                                                                  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1,  1, 1), 1);
    passColor = vec3(0, 0, 1);
    EmitVertex();                                                                  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, -1, 1), 1);
    passColor = vec3(0, 0, 1);
    EmitVertex();
    EndPrimitive();
    // bot
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,  1,  1), 1);
    passColor = vec3(0, 1, 0);
    EmitVertex(); 
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,  1, -1), 1);
    passColor = vec3(0, 1, 0);
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, 1,  1), 1);
    passColor = vec3(0, 1, 0);
    EmitVertex();  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, 1, -1), 1);
    passColor = vec3(0, 1, 0);
    passColor = vec3(0, 1, 0);
    EmitVertex();
    EndPrimitive();
    // top
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,  -1,  1), 1);
    passColor = vec3(0, 1, 0);
    EmitVertex();   
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,  -1, -1), 1);
    passColor = vec3(0, 1, 0);
    EmitVertex();  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, -1,  1), 1);
    passColor = vec3(0, 1, 0);
    EmitVertex();   
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, -1, -1), 1);
    passColor = vec3(0, 1, 0);
    passColor = vec3(0, 1, 0);
    EmitVertex();
    EndPrimitive();
}