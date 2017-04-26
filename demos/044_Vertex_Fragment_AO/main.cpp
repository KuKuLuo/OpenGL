//========================================================================================================================================================================================================================
// DEMO 044 : Vertex-Fragment Ambient Occlusion
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/noise.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "glfw_window.hpp"
#include "gl_info.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "image.hpp"
#include "vao.hpp"
#include "fbo1.hpp"
#include "attribute.hpp"
#include "plato.hpp"
#include "vertex.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;
    int draw_mode = 0;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================

    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);

        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE))
            draw_mode = (draw_mode + 1) % 6;        
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};

float noise_func(const glm::vec3& v, const glm::vec3& n, GLuint l)
{
    float magnitude = 0.77f;
    float s = 0.0f;
    glm::vec3 p = v;
    float frequency = 15.0f;

    for (int l0 = 0; l0 < 6; ++l0)
    {
        s += magnitude * glm::simplex(frequency * n);
        p.x += 2.0f;
        magnitude *= 0.77;
        frequency *= 2.31;
    }

    static float magnitudes[] = {0.25138f, 0.093573f, 0.042512f, 0.027535f, 0.014214f, 0.009539f, 0.004125f, 0.001525f, 0.000825f, 0.000225f};
    return -magnitudes[l] * s;
}

template<typename vertex_t> vao_t generate_fractal_vbo(const glm::vec3* positions, GLuint V, const glm::uvec3* triangles, GLuint F, GLuint levels)
{
    //===================================================================================================================================================================================================================
    // preliminary step 1 :: calculate final number of vertices, edges and faces
    //===================================================================================================================================================================================================================

    GLuint pow4 = 1 << (2 * levels);
    GLuint nV = pow4 * (V - 2) + 2;
    GLuint nF = pow4 * F;
    GLuint nE = nF + nF + nF;

    struct edge_t
    {
        GLuint a, b;
        GLuint face;
        GLuint midpoint; 
        edge_t(GLuint a, GLuint b, GLuint face, GLuint midpoint) : a(a), b(b), face(face), midpoint(midpoint) {}      
    };

    GLuint v = 0, e = 0, f = 0;

    //===================================================================================================================================================================================================================
    // preliminary step 2 :: allocate memory for surface construction, copy and set up initial data
    //===================================================================================================================================================================================================================
    edge_t* edges = (edge_t*) malloc(nE * sizeof(edge_t));
    glm::uvec3* faces = (glm::uvec3*) malloc(nF * sizeof(glm::uvec3));
    vertex_t* vertices = (vertex_t*) malloc(nV * sizeof(vertex_t));
    
    while(v < V)
    {
        vertices[v].position = positions[v];
        vertices[v].normal = glm::vec3(0.0f);
        ++v;
    }

    while(f < F)                                                                                                                                                                                      
    {                                                
        faces[f] = triangles[f];
        edges[e++] = edge_t(faces[f].x, faces[f].y, f, -1);                                                                                                                                                                        
        edges[e++] = edge_t(faces[f].y, faces[f].z, f, -1);                                                                                                                                                                        
        edges[e++] = edge_t(faces[f].z, faces[f].x, f, -1);
        ++f;                                                                                                                                                                        
    }

    //===================================================================================================================================================================================================================
    // main loop
    //===================================================================================================================================================================================================================
    for(int l = 0; l < levels; ++l)
    {
        //================================================================================================================================================================================================================
        // 1 :: quick sort edges lexicographically
        //================================================================================================================================================================================================================
        const unsigned int STACK_SIZE = 32;                                                         // variables to emulate stack of sorting requests
        
        struct                                                                                                                                                                                                                
        {                                                                                                                                                                                                                     
            edge_t* l;                                                                              // left index of the sub-array that needs to be sorted                                                                    
            edge_t* r;                                                                              // right index of the sub-array to sort                                                                                   
        } _stack[STACK_SIZE];                                                                                                                                                                                                 
                                                                                                                                                                                                                              
        int sp = 0;                                                                                 // stack pointer, stack grows up not down                                                                                 
        _stack[sp].l = edges;                                                                                                                                                                                                 
        _stack[sp].r = edges + e - 1;                                                                                                                                                                                         
                                                                                                                                                                                                                              
        do                                                                                                                                                                                                                    
        {                                                                                                                                                                                                                     
            edge_t* l = _stack[sp].l;                                                                                                                                                                                
            edge_t* r = _stack[sp].r;                                                                                                                                                                                
            --sp;                                                                                                                                                                                                             
            do                                                                                                                                                                                                                
            {                                                                                                                                                                                                                 
                edge_t* i = l;                                                                                                                                                                                       
                edge_t* j = r;                                                                                                                                                                                       
                edge_t* m = i + (j - i) / 2;                                                                                                                                                                         
                GLuint a = m->a;                                                                                                                                                                                             
                GLuint b = m->b;                                                                                                                                                                                             
                do                                                                                                                                                                                                            
                {                                                                                                                                                                                                             
                    while ((i->b < b) || ((i->b == b) && (i->a < a))) i++;                          // lexicographic compare and proceed forward if less                                                                      
                    while ((j->b > b) || ((j->b == b) && (j->a > a))) j--;                          // lexicographic compare and proceed backward if greater                                                                    
                                                                                                                                                                                                                              
                    if (i <= j)                                                                                                                                                                                               
                    {                                                                                                                                                                                                         
                        std::swap(i->a, j->a);                                                                                                                                                                                
                        std::swap(i->b, j->b);                                                                                                                                                                                
                        std::swap(i->face, j->face);                                                                                                                                                                                
                        i++;                                                                                                                                                                                                  
                        j--;                                                                                                                                                                                                  
                    }                                                                                                                                                                                                         
                }                                                                                                                                                                                                             
                while (i <= j);                                                                                                                                                                                               
                                                                                                                                                                                                                              
                if (j - l < r - i)                                                                  // push the larger interval to stack and continue sorting the smaller one                                                      
                {                                                                                                                                                                                                             
                    if (i < r)                                                                                                                                                                                                
                    {                                                                                                                                                                                                         
                        ++sp;                                                                                                                                                                                                 
                        _stack[sp].l = i;                                                                                                                                                                                     
                        _stack[sp].r = r;                                                                                                                                                                                     
                    }                                                                                                                                                                                                         
                    r = j;                                                                                                                                                                                                    
                }                                                                                                                                                                                                             
                else                                                                                                                                                                                                          
                {                                                                                                                                                                                                             
                    if (l < j)                                                                                                                                                                                                
                    {                                                                                                                                                                                                         
                        ++sp;                                                                                                                                                                                                 
                        _stack[sp].l = l;                                                                                                                                                                                     
                        _stack[sp].r = j;                                                                                                                                                                                     
                    }                                                                                                                                                                                                         
                    l = i;                                                                                                                                                                                                    
                }                                                                                                                                                                                                             
            }                                                                                                                                                                                                                 
            while(l < r);                                                                                                                                                                                                     
        }                                                                                                                                                                                                                     
        while (sp >= 0); 
        
        //================================================================================================================================================================================================================
        // 2 :: go over all the edges and assign indices to the new vertices, filling their positions
        //================================================================================================================================================================================================================
        for(GLuint ee = 0; ee < e; ++ee)
        {
            edge_t& edge = edges[ee];
            if (edge.midpoint != -1) continue;

            //============================================================================================================================================================================================================
            // 2.1 :: find edge ba 
            //============================================================================================================================================================================================================
            GLint l = 0;                                                                                                                                                                                                        
            GLint r = e - 1;
            GLint m;
            while (l <= r)                                                                                                                                                                                                
            {                                                                                                                                                                                                             
                m = (r + l) / 2;                                                                                                                                                                                  
                                                                                                                                                                                                                      
                if (edges[m].b < edge.a) { l = m + 1; continue; }                                                                                                                                                              
                if (edges[m].b > edge.a) { r = m - 1; continue; }                                                                                                                                                              
                if (edges[m].a < edge.b) { l = m + 1; continue; }                                                                                                                                                              
                if (edges[m].a > edge.b) { r = m - 1; continue; }                                                                                                                                                              
                break;
            }

            //============================================================================================================================================================================================================
            // 2.2 :: update midpoint index for both [ab] and [ba] 
            //============================================================================================================================================================================================================
            edge.midpoint = v++;
            edges[m].midpoint = edge.midpoint;
            glm::uvec3& abc = faces[edge.face];
            glm::uvec3& bad = faces[edges[m].face];

            GLuint a = edge.a;
            GLuint b = edge.b;
            GLuint s = a + b;
            GLuint c = abc.x + abc.y + abc.z - s;
            GLuint d = bad.x + bad.y + bad.z - s;

            // glm::vec3 predicted_A = 


            glm::vec3 mid_ab = 0.5f * (vertices[edge.a].position + vertices[edge.b].position);
            glm::vec3 mid_cd = 0.5f * (vertices[c].position + vertices[d].position);

            vertices[edge.midpoint].position = 0.85f * mid_ab + 0.15f * mid_cd;
            //vertices[edge.midpoint].position = 1.125f * mid_ab - 0.125f * mid_cd;
            vertices[edge.midpoint].normal = glm::vec3(0.0f);
        }

        //================================================================================================================================================================================================================
        // 3 :: rebuild faces array and generate new edges at the same time
        //================================================================================================================================================================================================================
        GLuint F = f;

        for(GLuint ff = 0; ff < F; ++ff)
        {
            glm::uvec3 face = faces[ff];
            GLuint a = face.x;
            GLuint b = face.y;
            GLuint c = face.z;
            GLint ll, rr;
            GLint e_ab, e_bc, e_ca;

            //============================================================================================================================================================================================================
            // 3.1 :: find edge [ab]
            //============================================================================================================================================================================================================
            ll = 0; rr = e - 1;
            while (ll <= rr)                                                                                                                                                                                                
            {                                                                                                                                                                                                             
                e_ab = (rr + ll) / 2;                                                                                                                                                                                  
                if (edges[e_ab].b < b) { ll = e_ab + 1; continue; }                                                                                                                                                              
                if (edges[e_ab].b > b) { rr = e_ab - 1; continue; }                                                                                                                                                              
                if (edges[e_ab].a < a) { ll = e_ab + 1; continue; }                                                                                                                                                              
                if (edges[e_ab].a > a) { rr = e_ab - 1; continue; }                                                                                                                                                              
                break;
            }

            //============================================================================================================================================================================================================
            // 3.2 :: find edge [bc]
            //============================================================================================================================================================================================================
            ll = 0; rr = e - 1;
            while (ll <= rr)                                                                                                                                                                                                
            {                                                                                                                                                                                                             
                e_bc = (rr + ll) / 2;                                                                                                                                                                                  
                if (edges[e_bc].b < c) { ll = e_bc + 1; continue; }                                                                                                                                                              
                if (edges[e_bc].b > c) { rr = e_bc - 1; continue; }                                                                                                                                                              
                if (edges[e_bc].a < b) { ll = e_bc + 1; continue; }                                                                                                                                                              
                if (edges[e_bc].a > b) { rr = e_bc - 1; continue; }                                                                                                                                                              
                break;
            }

            //============================================================================================================================================================================================================
            // 3.3 :: find edge [ca]
            //============================================================================================================================================================================================================
            ll = 0; rr = e - 1;
            while (ll <= rr)                                                                                                                                                                                                
            {                                                                                                                                                                                                             
                e_ca = (rr + ll) / 2;                                                                                                                                                                                  
                if (edges[e_ca].b < a) { ll = e_ca + 1; continue; }                                                                                                                                                              
                if (edges[e_ca].b > a) { rr = e_ca - 1; continue; }                                                                                                                                                              
                if (edges[e_ca].a < c) { ll = e_ca + 1; continue; }                                                                                                                                                              
                if (edges[e_ca].a > c) { rr = e_ca - 1; continue; }                                                                                                                                                              
                break;
            }
            
            GLuint p = edges[e_bc].midpoint;
            GLuint q = edges[e_ca].midpoint;
            GLuint r = edges[e_ab].midpoint;
                             

            faces[ff] = glm::uvec3(p, q, r);
            GLuint f_arq = f++; faces[f_arq] = glm::uvec3(a, r, q);
            GLuint f_bpr = f++; faces[f_bpr] = glm::uvec3(b, p, r);
            GLuint f_cqp = f++; faces[f_cqp] = glm::uvec3(c, q, p);
        }

        //================================================================================================================================================================================================================
        // 4 :: fill edge array
        //================================================================================================================================================================================================================
        e = 0;
        for(GLuint ff = 0; ff < f; ++ff)
        {
            glm::uvec3& face = faces[ff];
            edges[e++] = edge_t(face.x, face.y, ff, -1);
            edges[e++] = edge_t(face.y, face.z, ff, -1);
            edges[e++] = edge_t(face.z, face.x, ff, -1);
        }

        //================================================================================================================================================================================================================
        // 5 :: recalculate normals
        //================================================================================================================================================================================================================
        for(GLuint ff = 0; ff < f; ++ff)
        {
            glm::uvec3& face = faces[ff];
            glm::vec3 normal = glm::cross(vertices[face.y].position - vertices[face.x].position, vertices[face.z].position - vertices[face.x].position);
            vertices[face.x].normal += normal;
            vertices[face.y].normal += normal;
            vertices[face.z].normal += normal;
        }
        
        //================================================================================================================================================================================================================
        // 6 :: displace vertices along normals, zero the normals
        //================================================================================================================================================================================================================
        for(GLuint vv = 0; vv < v; ++vv)
        {
            glm::vec3 normal = glm::normalize(vertices[vv].normal);
            vertices[vv].position = vertices[vv].position + noise_func(vertices[vv].position, normal, l) * normal;
            vertices[vv].normal = glm::vec3(0.0f);
        }
    }

    //====================================================================================================================================================================================================================
    // mesh construction is over --- recalculate normals
    //====================================================================================================================================================================================================================
    for(GLuint f = 0; f < nF; ++f)
    {
        glm::uvec3& face = faces[f];
        glm::vec3 normal = glm::cross(vertices[face.y].position - vertices[face.x].position, vertices[face.z].position - vertices[face.x].position);
        vertices[face.x].normal += normal;
        vertices[face.y].normal += normal;
        vertices[face.z].normal += normal;
    }

    for(GLuint vv = 0; vv < v; ++vv)
        vertices[vv].normal = glm::normalize(vertices[vv].normal);

    free(edges);


    attribute_data_t<vertex_t, GLuint, GL_TRIANGLES> attribute_data(nV, nF * 3, vertices, (GLuint*) faces);
    calculate_vertex_occlusion_in_place<vertex_t, GLuint, GL_TRIANGLES>(attribute_data);

    //====================================================================================================================================================================================================================
    // create OpenGL position-normal-occlusion VAO and release the memory allocated                                                                                       
    //====================================================================================================================================================================================================================
    vao_t vao;
    vao.init(GL_TRIANGLES, vertices, nV, glm::value_ptr(faces[0]), nF * 3);
    
    return vao;
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Vertex-Fragment Ambient Occlusion", 4, 3, 3, 1920, 1080, true);

    //===================================================================================================================================================================================================================
    // geometry pass :: fills depth texture only
    //===================================================================================================================================================================================================================
    glsl_program_t geometry_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/geometry_pass.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/geometry_pass.fs"));
    geometry_pass.enable();
    uniform_t uni_gp_mvp_matrix = geometry_pass["mvp_matrix"];

    // Texture unit usages ::
    // geometry depth output = ssao_compute depth texture input :: unit0
    // ssao_compute output = ssao_blur input                    :: unit1
    // ssao_blur output = lighting_pass input                   :: unit2
    // tb_tex lighting_pass texture input                       :: unit3

    //===================================================================================================================================================================================================================
    // shader program for screen-space ambient occlusion pass
    //===================================================================================================================================================================================================================
    const unsigned int KERNEL_SIZE = 64;
    glm::vec3 ssao_kernel[KERNEL_SIZE];
    for (int i = 0; i < KERNEL_SIZE; ++i)
        ssao_kernel[i] = glm::sphericalRand(1.0f);

    glsl_program_t ssao_compute(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_compute.fs"));
    ssao_compute.enable();
    uniform_t uni_sc_camera_matrix = ssao_compute["camera_matrix"];
    uniform_t uni_sc_pv_matrix = ssao_compute["projection_view_matrix"];
    ssao_compute["depth_tex"] = 0;
    ssao_compute["ssao_kernel"] = ssao_kernel;

    //===================================================================================================================================================================================================================
    // shader program for ssao image blur 
    //===================================================================================================================================================================================================================
    glsl_program_t ssao_blur(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/quad.vs"),
                             glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ssao_blur.fs"));

    ssao_blur.enable();
    ssao_blur["ssao_input"] = 1;

    //===================================================================================================================================================================================================================
    // lighting shader program 
    //===================================================================================================================================================================================================================
    glsl_program_t lighting_pass(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/lighting_pass.vs"),
                                 glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/lighting_pass.fs"));
    lighting_pass.enable();
    uniform_t uni_lp_model_matrix = lighting_pass["model_matrix"];
    uniform_t uni_lp_pv_matrix    = lighting_pass["projection_view_matrix"];
    uniform_t uni_lp_camera_ws    = lighting_pass["camera_ws"];
    uniform_t uni_lp_light_ws     = lighting_pass["light_ws"];
    uniform_t uni_lp_draw_mode    = lighting_pass["draw_mode"]; 

    lighting_pass["ssao_blurred_tex"] = 2;
    lighting_pass["tb_tex"] = 3;

    //===================================================================================================================================================================================================================
    // Generate the surface
    //===================================================================================================================================================================================================================
    vao_t deformed_icosahedron = generate_fractal_vbo<vertex_pno_t>(plato::icosahedron::vertices, plato::icosahedron::V, plato::icosahedron::triangles, plato::icosahedron::F, 7);

    depth_map_t zbuffer_fbo(window.res_x, window.res_y, GL_TEXTURE0, GL_DEPTH_COMPONENT32F);
    color_map_t ssao_compute_fbo(window.res_x, window.res_y, GL_TEXTURE1, GL_R32F);
    color_map_t ssao_blur_fbo(window.res_x, window.res_y, GL_TEXTURE2, GL_R32F);

    //===================================================================================================================================================================================================================
    // generate fake vao for full-screen quad
    //===================================================================================================================================================================================================================
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    //===================================================================================================================================================================================================================
    // load 2D texture for trilinear blending in lighting shader
    //===================================================================================================================================================================================================================
    glActiveTexture(GL_TEXTURE3);
    GLuint tb_tex_id = image::png::texture2d("../../../resources/tex2d/crystalline.png", 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, false);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        window.new_frame();

        float time = window.frame_ts;
        glm::mat4 projection_view_matrix = window.camera.projection_view_matrix();
        glm::mat4 model_matrix = glm::mat4(1.0f);
        glm::mat4 camera_matrix = window.camera.camera_matrix();
        glm::mat4 mvp_matrix = projection_view_matrix * model_matrix;
        glm::vec3 camera_ws = window.camera.position();
        glm::vec3 light_ws = glm::vec3(2.0f * glm::cos(time), 4.0f, -2.0f * glm::sin(time));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        //===============================================================================================================================================================================================================
        // geometry pass
        //===============================================================================================================================================================================================================
        zbuffer_fbo.bind();
        glClear(GL_DEPTH_BUFFER_BIT);

        geometry_pass.enable();
        uni_gp_mvp_matrix = mvp_matrix;
        deformed_icosahedron.render();

        //===============================================================================================================================================================================================================
        // ssao compute pass + ssao blur pass
        //===============================================================================================================================================================================================================
        glBindVertexArray(vao_id);
        glDisable(GL_DEPTH_TEST);

        ssao_compute_fbo.bind();
        ssao_compute.enable();
        uni_sc_pv_matrix = projection_view_matrix;
        uni_sc_camera_matrix = camera_matrix;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        ssao_blur_fbo.bind();
        ssao_blur.enable();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //===============================================================================================================================================================================================================
        // lighting pass
        //===============================================================================================================================================================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        lighting_pass.enable();

        uni_lp_model_matrix = model_matrix;
        uni_lp_pv_matrix    = projection_view_matrix;
        uni_lp_camera_ws    = camera_ws;
        uni_lp_light_ws     = light_ws;
        uni_lp_draw_mode    = window.draw_mode;

        deformed_icosahedron.render();           

        window.end_frame();
    }
    
    //===================================================================================================================================================================================================================
    // close OpenGL window and terminate GLFW
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}