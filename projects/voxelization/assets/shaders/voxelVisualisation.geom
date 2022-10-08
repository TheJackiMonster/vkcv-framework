#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(points) in;
layout (triangle_strip, max_vertices = 24) out;

layout( push_constant ) uniform constants{
    mat4 viewProjection;
};

layout(location = 0) in float passCubeHalf[1];
layout(location = 1) in vec3 passColorToGeom[1];


layout(location = 0) out vec3 passColorToFrag;

void main()	{
    float cubeHalf = passCubeHalf[0];
    // right
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1, 1, 1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1, 1, -1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1, -1, 1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1, -1, -1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();
    EndPrimitive();
    // left
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, 1, 1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, 1, -1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, -1, 1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, -1, -1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();
    EndPrimitive();
    // back
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,   1, -1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();                                                                  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,  -1, -1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();                                                                  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1,  1, -1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();                                                                  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, -1, -1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();
    EndPrimitive();
    // front
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,   1, 1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();                                                                  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,  -1, 1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();                                                                  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1,  1, 1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();                                                                  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, -1, 1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();
    EndPrimitive();
    // bot
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,  1,  1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex(); 
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,  1, -1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, 1,  1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, 1, -1), 1);
    passColorToFrag = passColorToGeom[0];
    passColorToFrag = passColorToGeom[0];
    EmitVertex();
    EndPrimitive();
    // top
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,  -1,  1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();   
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(1,  -1, -1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();  
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, -1,  1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();   
    gl_Position = viewProjection * vec4(gl_in[0].gl_Position.xyz + cubeHalf * vec3(-1, -1, -1), 1);
    passColorToFrag = passColorToGeom[0];
    EmitVertex();
    EndPrimitive();
}