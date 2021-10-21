#include "plane.h"

Plane::Plane()
{
}

Plane::Plane(const PropertyList &propList)
{
    m_position = propList.getPoint("position",Point3f(0,0,0));
    m_normal = propList.getVector("normal",Point3f(0,0,1)).normalized();
}

Plane::~Plane()
{
}

bool Plane::intersect(const Ray& ray, Hit& hit) const
{
    float Vd = m_normal.dot(ray.direction);

    if(Vd >= -Epsilon && Vd <= Epsilon) // dot product close to zero => ray parallel to the plane
        return false;

    float D = m_position.dot(m_normal);
    float V0 = m_normal.dot(ray.origin);
    float t = (D-V0)/Vd;

    if(t<Epsilon)
        return false;

    hit.setT(t);
    hit.setNormal(m_normal);
    Vector3f u = Vector3f( m_normal.y(), m_normal.z(), -m_normal.x() );
    Vector3f v = u.cross(m_normal);
    Point3f p = ray.at(t);
    hit.setTexcoord(Vector2f(p.dot(u), p.dot(v)));

    return true;
}

REGISTER_CLASS(Plane, "plane")
