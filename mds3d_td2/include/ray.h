#ifndef RAY
#define RAY

#include "common.h"
#include "vector.h"

class Shape;

class Ray
{
public:
    Ray(const Point3f& o, const Vector3f& d, float shadow = false)
        : origin(o), direction(d), recursionLevel(0), shadowRay(shadow)
    {}
    Ray() : recursionLevel(0), shadowRay(false) {}

    Point3f origin;
    Vector3f direction;

    Point3f at(float t) const { return origin + t*direction; }

    int recursionLevel;   ///< recursion level (used as a stoping critera)
    bool shadowRay;       ///< tag for shadow rays
};

class Hit
{
public:
    Hit() : m_texcoord(0,0), mp_shape(nullptr), m_t(std::numeric_limits<float>::max()) {}

    bool foundIntersection() const { return m_t < std::numeric_limits<float>::max(); }

    void setT(float t) { m_t = t; }
    float t() const { return m_t; }

    void setShape(const Shape* shape) { mp_shape = shape; }
    const Shape* shape() const { return mp_shape; }

    void setNormal(const Normal3f& n) { m_normal = n; }
    const Normal3f& normal() const { return m_normal; }

    void setTexcoord(const Vector2f& uv) { m_texcoord = uv; }
    const Vector2f& texcoord() const { return m_texcoord; }

private:
    Normal3f m_normal;
    Vector2f m_texcoord;
    const Shape* mp_shape;
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

/**
 * Computed the refracted direction @param refracted of an @param incident direction according to the @param normal \
 * and the material refraction coefficients @param etaI et @param etaT
 * @return false in case of total internal reflection for transmission
 */
static inline bool refract(const Normal3f& normal, const Vector3f& incident, float etaI, float etaT, Vector3f& refracted)
{
    float eta = etaI / etaT;
    float cosThetaI = -normal.dot(incident);
    float sin2ThetaI = std::max(0.f, 1.f - cosThetaI * cosThetaI);
    float sin2ThetaT = eta * eta * sin2ThetaI;
    // Handle total internal reflection for transmission
    if (sin2ThetaT > 1)
        return false;
    float cosThetaT = std::sqrt(1 - sin2ThetaT);
    refracted = eta * incident + (eta * cosThetaI - cosThetaT) * normal;
    return true;
}

#endif
