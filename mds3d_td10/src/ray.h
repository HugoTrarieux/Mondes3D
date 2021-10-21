#ifndef RAY
#define RAY

#include <Eigen/Geometry>

class Mesh;
class AreaLight;

typedef Eigen::Vector3f Point3f;
typedef Eigen::Vector3f Normal3f;
using Eigen::Vector3f;
using Eigen::Vector2f;

class Ray
{
public:
    Ray(const Point3f& o, const Vector3f& d)
        : origin(o), direction(d)
    {}
    Ray() {}

    Point3f origin;
    Vector3f direction;

    Point3f at(float t) const { return origin + t*direction; }
};

class Hit
{
public:
    Hit() : m_uvw(0,0,0), m_shape(0), m_faceId(0), m_t(std::numeric_limits<float>::max()) {}

    bool foundIntersection() const { return m_t < std::numeric_limits<float>::max(); }

    void setT(float t) { m_t = t; }
    float t() const { return m_t; }

    void setShape(const Mesh* shape) { m_shape = shape; }
    const Mesh* shape() const { return m_shape; }

    void setFaceId(int id) { m_faceId = id; }
    int faceId() const { return m_faceId; }

    void setBaryCoords(const Vector3f& uvw) { m_uvw = uvw; }
    const Vector3f& baryCoords() const { return m_uvw; }

    void setIntersectionPoint(const Vector3f& p)  { m_intersectionPoint = p; }
    const Vector3f& intersectionPoint() const { return m_intersectionPoint; }

private:
    Vector3f m_uvw;
    Vector3f m_intersectionPoint;
    const Mesh* m_shape;
    int m_faceId;
    float m_t;
};

/** Compute the intersection between a ray and an aligned box
  * \returns true if an intersection is found
  * The ranges are returned in tMin,tMax
  */
static inline bool intersect(const Ray& ray, const Eigen::AlignedBox3f& box, float& tMin, float& tMax, Normal3f& normal)
{
    Eigen::Array3f t1, t2;
    t1 = (box.min()-ray.origin).cwiseQuotient(ray.direction);
    t2 = (box.max()-ray.origin).cwiseQuotient(ray.direction);
    Eigen::Array3f::Index maxIdx, minIdx;
    tMin = t1.min(t2).maxCoeff(&maxIdx);
    tMax = t1.max(t2).minCoeff(&minIdx);
    normal = Normal3f::Zero();
    normal[maxIdx] = -1;
    return tMax>0 && tMin<=tMax;
}

#endif
