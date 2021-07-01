#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 passPosIn[3];
layout(location = 1) in vec2 passUVIn[3];
layout(location = 2) in vec3 passNIn[3];

layout(location = 0) out vec3 passPos;
layout(location = 1) out vec2 passUV;
layout(location = 2) out vec3 passN;

void main()	{
    // compute geometric normal, no normalization necessary
    vec3 N = cross(passPosIn[0] - passPosIn[1], passPosIn[0] - passPosIn[2]);
    N = abs(N); // only interested in the magnitude
    
    for(int i = 0; i < 3; i++){
        // swizzle position, so biggest side is rasterized
        if(N.z > N.x && N.z > N.y){
            gl_Position = gl_in[i].gl_Position.xyzw;
        }
        else if(N.x > N.y){
            gl_Position = gl_in[i].gl_Position.yzxw;
        }
        else{
            gl_Position = gl_in[i].gl_Position.xzyw;
        }
        gl_Position.z = gl_Position.z * 0.5 + 0.5;  // xyz are kept in NDC range [-1, 1] so swizzling works, but vulkan needs final z in range [0, 1]
        passPos = passPosIn[i];
        passUV  = passUVIn[i];
        passN   = passNIn[i];
        EmitVertex();
    }
    EndPrimitive();
}