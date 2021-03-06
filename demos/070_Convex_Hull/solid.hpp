#ifndef _solid_included_1689072258736476253476523746253762891197615035283652353 
#define _solid_included_1689072258736476253476523746253762891197615035283652353

#include <vector>              
#include <GL/glew.h>
#include <glm/glm.hpp>

// ======================================================================================================================================================================================================================
// structure that stores both geometric and OpenGL representation of a convex solid
// ======================================================================================================================================================================================================================
struct solid
{
    GLuint V, E, F;

	// ==================================================================================================================================================================================================================
	// vertices, normals and indices
	// ==================================================================================================================================================================================================================
    std::vector<glm::dvec3> vertices;
    std::vector<glm::dvec3> normals;
    std::vector<glm::ivec3> triangles;
    std::vector<glm::ivec3> adjacency;

	// ==================================================================================================================================================================================================================
	// zero, first, and second-order momenta of the polyhedra
	// ==================================================================================================================================================================================================================
    double volume;
    glm::dvec3 mass_center;
	double mxx, mxy, mxz, myy, myz, mzz;

	// ==================================================================================================================================================================================================================
	// OpenGL buffers identificators
	// ==================================================================================================================================================================================================================
    GLuint vao_id, vbo_id, nbo_id, ibo_id;

	solid() {};

	// ==================================================================================================================================================================================================================
	// builds convex hull of a set of points : points vector is modified -- sorted lexicografically in z, y, x
	// ==================================================================================================================================================================================================================
	void convex_hull(std::vector<glm::dvec3>& points);

	// ==================================================================================================================================================================================================================
	// two methods of computation of the solid support function
	// * brute force   -- O(V) time
	// * hill climbing -- O(logV) time, face adjacency information used
	// ==================================================================================================================================================================================================================
	double support_bf(const glm::dvec3& direction, int& vertex_index);
	double support_hc(const glm::dvec3& direction, int& vertex_index, int& triangle_index);

	// ==================================================================================================================================================================================================================
	// vertices, normals and triangles assumed to be filled prior to calling this function
	// ==================================================================================================================================================================================================================
	void fill_buffers();

	// ==================================================================================================================================================================================================================
	// indexed render function
	// ==================================================================================================================================================================================================================
    void render();

	// ==================================================================================================================================================================================================================
	// destructor : frees up video memory buffers
	// ==================================================================================================================================================================================================================
    ~solid();
};

#endif // _solid_included_1689072258736476253476523746253762891197615035283652353
