#include "integrator.h"
#include "scene.h"

class FlatIntegrator : public Integrator {
public:
    FlatIntegrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {
        /** TODO : Find the surface that is visible in the requested direction
                   Return its ambiant color */
        Hit hitShape = Hit();
        scene->intersect(ray, hitShape);
        if(hitShape.foundIntersection()){
            Color3f color = hitShape.shape()->material()->ambientColor();
            return color;
        } else {
            return Color3f(0.f);
        }
    }

    std::string toString() const {
        return "FlatIntegrator[]";
    }
};

REGISTER_CLASS(FlatIntegrator, "flat")
