#include "integrator.h"
#include "scene.h"

class Normals : public Integrator {
public:
    Normals(const PropertyList &props) {
        
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {
        Hit hitShape = Hit();
        scene->intersect(ray, hitShape);
        if(hitShape.foundIntersection()){
            Normal3f normal = hitShape.normal(); 
            Color3f color = {abs(normal[0]), abs(normal[1]), abs(normal[2])};
            return color;
        } else {
            return Color3f(0.f);
        }
    }

    std::string toString() const {
        return "Normals[]";
    }
};

REGISTER_CLASS(Normals, "normals");