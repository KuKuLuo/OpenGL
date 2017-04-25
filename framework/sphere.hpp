#ifndef _sphere_included_235872365354939851234963462099846368123698345924091252              
#define _sphere_included_235872365354939851234963462099846368123698345924091252

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "vertex.hpp"
#include "vao.hpp"

//===================================================================================================================================================================================================================
// Spherical function types for procedural generation of surfaces, homeomorphic to sphere
//===================================================================================================================================================================================================================
typedef float (*spheric_landscape_func) (const glm::vec3& direction, int level);

//===================================================================================================================================================================================================================
// spherical indexed mesh
//===================================================================================================================================================================================================================

struct sphere_t
{
    vao_t vao;

     sphere_t() {}
    ~sphere_t() {}


    //===============================================================================================================================================================================================================
    // Single-threaded subdivision of sphere : provided function computes position-normal-texture coordinate vertex attributes
    //===============================================================================================================================================================================================================
    template<typename vertex_t> void generate_vao(typename maps<vertex_t>::spheric_func func, int level);

    //===============================================================================================================================================================================================================
    // Multithreaded subdivision of sphere : provided function computes position-normal-texture coordinate vertex attributes
    //===============================================================================================================================================================================================================
    template<typename vertex_t, int threads = 8> void generate_vao_mt(typename maps<vertex_t>::spheric_func func, int level);


    //===============================================================================================================================================================================================================
    // Rendering functions
    //===============================================================================================================================================================================================================
    void render();
    void render(GLsizei count, const GLvoid* offset);
    void instanced_render(GLsizei primcount);


  private:
    //===============================================================================================================================================================================================================
    // Auxiliary structure for multithreaded buffer filling
    //===============================================================================================================================================================================================================
    template<typename vertex_t> struct compute_data
    {
        //===========================================================================================================================================================================================================
        // Computation parameters : barycentric subdivision level, vertices, edges and faces of the initial mesh.
        //===========================================================================================================================================================================================================
        int level;                                                      // the level of barycentric subdivision
        int V, E, Q;                                                    // the number of vertices, edges and quads in the initial mesh
        const glm::uvec2* edges;                                        // array of edges (vertex-vertex pairs)
        const GLuint* edge_indices;                                     // edge (vertex-vertex pair) to edge index array 
        const glm::uvec4* quads;                                        // array of quads in the initial mesh
        int vertices_per_quad;                                          // the number of new vertices inside a quad, the number of new vertices on any edge is simply = level - 1
        int indices_per_quad;                                         
        int quad_vertices_base_index;                                   

        //===========================================================================================================================================================================================================
        // dynamically allocated attribute and index buffers for the functions below to fill
        //===========================================================================================================================================================================================================
        vertex_t* vertices;
        GLuint* indices;

    };

    //===============================================================================================================================================================================================================
    // Auxiliary function that populates its own chunk of vertex and index buffers
    //===============================================================================================================================================================================================================
    template<typename vertex_t> static void fill_vao_chunk(typename maps<vertex_t>::spheric_func func, const compute_data<vertex_t>& data, GLuint edge_start, GLuint edge_end, GLuint quad_start, GLuint quad_end);

};

#endif // _sphere_included_235872365354939851234963462099846368123698345924091252