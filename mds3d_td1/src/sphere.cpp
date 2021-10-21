#include "sphere.h"
#include <iostream>

Sphere::Sphere(float radius)
    : m_radius(radius)
{
}

Sphere::Sphere(const PropertyList &propList)
{
    m_radius = propList.getFloat("radius",1.f);
    m_center = propList.getPoint("center",Point3f(0,0,0));
}

Sphere::~Sphere()
{
}

bool Sphere::intersect(const Ray& ray, Hit& hit) const
{
    /// DONE: compute ray-sphere intersection
    Point3f     o = ray.origin;
    Point3f     c = m_center;
    Vector3f    d = ray.direction;
    Vector3f    v = o - c;
    float       r = m_radius;
    Point3f     pHit;

    float A = 1;                    //d.squaredNorm() = 1
    float B = 2 * v.dot(d);
    float C = v.squaredNorm() - r*r;
    float delta = B*B - 4 * A * C;
    
    float t1 = (-B + sqrt(delta)) / 2;
    float t2 = (-B - sqrt(delta)) / 2;

    if(t1<0 && t2<0)
        return false;

    hit.setShape(this);
    if(t1 <= t2){
        hit.setT(t1);
    } else {
        hit.setT(t2);
    }
    pHit = ray.at(hit.t());
    Normal3f normal = pHit - c;
    normal.normalize();
    hit.setNormal(normal);

    return hit.foundIntersection();
}

REGISTER_CLASS(Sphere, "sphere")
