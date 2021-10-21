#include "integrator.h"
#include "scene.h"

class NormalsIntegrator : public Integrator {
public:
    NormalsIntegrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {
        /* Find the surface that is visible in the requested direction */
        Hit hit;
        scene->intersect(ray, hit);
        if (!hit.foundIntersection())
            return Color3f(0.0f);

        /* Return the component-wise absolute
           value of the shading normal as a color */
        Normal3f n = hit.normal().cwiseAbs();
        return Color3f(n[0],n[1],n[2]);
    }

    std::string toString() const {
        return "NormalsIntegrator[]";
    }
};

REGISTER_CLASS(NormalsIntegrator, "normals")
