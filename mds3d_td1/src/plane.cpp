#include "plane.h"

Plane::Plane()
{
}

Plane::Plane(const PropertyList &propList)
{
    m_position = propList.getPoint("position",Point3f(0,0,0));
    m_normal = propList.getVector("normal",Point3f(0,0,1));
}

Plane::~Plane()
{
}

bool Plane::intersect(const Ray& ray, Hit& hit) const
{
    Vector3f d = ray.direction;
    d.normalize();
    Vector3f n = m_normal;

    float denominateur = d.dot(n);
    if(denominateur == 0)
        return false;

    float numerateur = n.dot(-ray.origin + m_position);

    float t = numerateur/denominateur;

    if(t == INFINITY)
        return false;
    if(t > 0){
        hit.setT(t);
        hit.setShape(this);
        hit.setNormal(m_normal);
        return true;
    } else{
        return false;
    }
}

REGISTER_CLASS(Plane, "plane")
