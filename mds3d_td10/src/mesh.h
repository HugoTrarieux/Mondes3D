#ifndef MESH_H
#define MESH_H

#include "ray.h"
#include <string>
#include <vector>

class Shader;
class BVH;

class Mesh
{
    typedef Eigen::Matrix<float,2,1,Eigen::DontAlign> Vector2f;
    typedef Eigen::Vector3f Vector3f;
    typedef Eigen::Matrix<float,4,1,Eigen::DontAlign> Vector4f;
    typedef Eigen::Vector3i Vector3i;

    struct Vertex
    {
      Vertex(const Vector3f& pos = Vector3f::Zero(),
             const Vector3f& n = Vector3f::Zero(),
             const Vector4f& c = Vector4f(0.6,0.6,0.6,1.0),
             const Vector2f& uv = Vector2f::Zero()
            )
        : position(pos), normal(n), color(c), texcoord(uv)
      {}

      Vector3f position;
      Vector3f normal;
      Vector4f color;
      Vector2f texcoord;
    };

public:
    Mesh() : mBVH(0) {}
    ~Mesh();

    /** load a triangular mesh from the file \a filename (.off or .obj) */
    bool load(const std::string& filename);

    /** initialize OpenGL's Vertex Buffer Array (must be called once before calling draw()) */
    void init();

    /** Send the mesh to OpenGL for drawing using shader \a shd */
    void draw(const Shader& shd);

    /// Re-compute vertex normals (needs to be called after editing vertex positions)
    void updateNormals();

    /// Copy vertex attributes from the CPU to GPU memory (needs to be called after editing any vertex attributes: positions, normals, texcoords, masks, etc.)
    void updateVBO();

    // For ray-casting:

    /// Re-compute the aligned bounding box (needs to be called after editing vertex positions)
    void updateBoundingBox();

    const Eigen::AlignedBox3f& boundingBox() const { return mBBox; }

    /// Re-compute the BVH for fast ray-mesh intersections (needs to be called after editing vertex positions)
    void updateBVH();

    /// computes the first intersection between the ray and the mesh in hit (if any)
    bool intersect(const Ray& ray, Hit& hit) const;

    /// \returns  the number of faces
    int nbFaces() const { return mFaces.size(); }

    /// \returns a const references to the \a vertexId -th vertex of the \a faceId -th face. vertexId must be between 0 and 2 !!
    const Vertex& vertexOfFace(int faceId, int vertexId) const { return mVertices[mFaces[faceId][vertexId]]; }

    /** compute the intersection between a ray and a given triangular face */
    bool intersectFace(const Ray& ray, Hit& hit, int faceId) const;

private:

    /** Loads a triangular mesh in the OFF format */
    bool loadOFF(const std::string& filename);
    bool loadOBJ(const std::string& filename);

    /** The list of vertices */
    std::vector<Vertex> mVertices;

    /** The list of face indices */
    std::vector<Vector3i> mFaces;

    unsigned int mVertexArrayId;
    unsigned int mVertexBufferId; ///< the id of the BufferObject storing the vertex attributes
    unsigned int mIndexBufferId;  ///< the id of the BufferObject storing the faces indices
    bool mIsInitialized;

    Eigen::AlignedBox3f mBBox;

    BVH *mBVH;
};


#endif // MESH_H
