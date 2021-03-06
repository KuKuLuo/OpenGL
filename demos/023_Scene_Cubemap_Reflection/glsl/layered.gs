#version 430 core

layout (points) in;
layout (triangle_strip, max_vertices = 144) out; 
 
const mat4 projection_matrix = 
    mat4(vec4( 1.0f,  0.0f,  0.0f,  0.0f), 
         vec4( 0.0f,  1.0f,  0.0f,  0.0f), 
         vec4( 0.0f,  0.0f, -1.0f, -1.0f), 
         vec4( 0.0f,  0.0f, -20.0f,  0.0f));


const mat4 view_matrices[6] = mat4[6] 
(
    mat4(vec4( 0.0f,  0.0f, -1.0f,  0.0f), 
         vec4( 0.0f, -1.0f,  0.0f,  0.0f), 
         vec4(-1.0f,  0.0f,  0.0f,  0.0f), 
         vec4( 0.0f,  0.0f,  0.0f,  1.0f)),

    mat4(vec4( 0.0f,  0.0f,  1.0f,  0.0f), 
         vec4( 0.0f, -1.0f,  0.0f,  0.0f), 
         vec4( 1.0f,  0.0f,  0.0f,  0.0f), 
         vec4( 0.0f,  0.0f,  0.0f,  1.0f)),

    mat4(vec4( 1.0f,  0.0f,  0.0f,  0.0f), 
         vec4( 0.0f,  0.0f, -1.0f,  0.0f), 
         vec4( 0.0f,  1.0f,  0.0f,  0.0f), 
         vec4( 0.0f,  0.0f,  0.0f,  1.0f)),

    mat4(vec4( 1.0f,  0.0f,  0.0f,  0.0f), 
         vec4( 0.0f,  0.0f,  1.0f,  0.0f), 
         vec4( 0.0f, -1.0f,  0.0f,  0.0f), 
         vec4( 0.0f,  0.0f,  0.0f,  1.0f)),

    mat4(vec4( 1.0f,  0.0f,  0.0f,  0.0f), 
         vec4( 0.0f, -1.0f,  0.0f,  0.0f), 
         vec4( 0.0f,  0.0f, -1.0f,  0.0f), 
         vec4( 0.0f,  0.0f,  0.0f,  1.0f)),

    mat4(vec4(-1.0f,  0.0f,  0.0f,  0.0f), 
         vec4( 0.0f, -1.0f,  0.0f,  0.0f), 
         vec4( 0.0f,  0.0f,  1.0f,  0.0f), 
         vec4( 0.0f,  0.0f,  0.0f,  1.0f))
);


const mat4 simplex_ms = mat4
(
    vec4(-1.0f, -1.0f,  1.0f, 0.0f),
    vec4( 1.0f, -1.0f, -1.0f, 0.0f),
    vec4(-1.0f,  1.0f, -1.0f, 0.0f),
    vec4( 1.0f,  1.0f,  1.0f, 0.0f)
);

const vec2[4] square = vec2[4] 
(
    vec2(0.0f, 0.0f), 
    vec2(1.0f, 0.0f),
    vec2(0.0f, 1.0f),
    vec2(1.0f, 1.0f)
);

const uint faces[24] = 
{
    1, 4, 6, 3,     // normal : x
    4, 2, 3, 5,     // normal : y
    3, 5, 6, 0,     // normal : z
    7, 2, 0, 5,     // normal : -x
    1, 7, 6, 0,     // normal : -y
    4, 2, 1, 7      // normal : -z
};

uniform float global_time;

in mat4 model_matrix[];

out vec4 position;
out vec4 normal;
out vec4 view_direction;
out vec2 texture_coord;

//mat4 rotationMatrix(vec3 axis, float angle)
//{
//    axis = normalize(axis);
//    float s = sin(angle);
//    float c = cos(angle);
//    float oc = 1.0 - c;
//    
//    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
//                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
//                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
//                0.0,                                0.0,                                0.0,                                1.0);
//}


void main()
{
    mat4 frame = model_matrix[0];

//    mat4 rotor = rotationMatrix(vec3(frame[3]), 3.0f * global_time * sin(dot(frame[3], vec4(12.9391f, 4.1417f, -16.89013, 0.0f))) );

    for (int layer = 0; layer < 6; ++layer)
    {
    
        mat4 view_matrix = view_matrices[layer];
        mat3 orientation = mat3(view_matrix) * mat3(frame);
        if (orientation[0][2] > 0.0f) frame[0] = -frame[0];
        if (orientation[1][2] > 0.0f) frame[1] = -frame[1];
        if (orientation[2][2] > 0.0f) frame[2] = -frame[2];
    
//        frame[0] = rotor * frame[0];
//        frame[1] = rotor * frame[1];
//        frame[2] = rotor * frame[2];
    
    
        mat4 simplex_ws = frame * simplex_ms;  
    
        vec4 shift = frame[3];
        vec4 view_point = view_matrix[3];
    
        vec4 vertices[] = {
                                shift + simplex_ws[0],
                                shift + simplex_ws[1],    
                                shift + simplex_ws[2],    
                                shift + simplex_ws[3],
                                shift - simplex_ws[0],
                                shift - simplex_ws[1],    
                                shift - simplex_ws[2],    
                                shift - simplex_ws[3] 
                          };
    
    
        mat4 normals_cs = view_matrix * frame;
    
        vec4 normals[6]    = {frame[0], frame[1], frame[2], -frame[0], -frame[1], -frame[2]};
    
        uint index = 0;    
    
        vec4 c = normalize(shift);
    
    
        for (uint i = 0; i < 6; ++i)
        {
            for (uint j = 0; j < 4; ++j)
            {
                gl_Layer = layer;
                position = vertices[faces[index]];
                view_direction = view_point - position;
                vec4 camera_space_position = view_matrix * position;
                gl_Position = projection_matrix * camera_space_position;
                texture_coord = square[j];
                normal = normals[i];
                EmitVertex();
                ++index;
            };
            EndPrimitive();
        };
    };
};




