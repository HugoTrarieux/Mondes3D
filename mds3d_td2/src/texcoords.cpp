#include "integrator.h"
#include "scene.h"

class TexcoordsIntegrator : public Integrator {
public:
    TexcoordsIntegrator(const PropertyList &props) {
        /* No parameters this time */
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {
        /* Find the surface that is visible in the requested direction */
        Hit hit;
        scene->intersect(ray, hit);
        if (!hit.foundIntersection())
            return Color3f(0.0f);

        /* Return the component-wise absolute
           value of the shading Texcoords as a color */
        Eigen::Vector2f uv = hit.texcoord();
        return Color3f(uv[0],uv[1],0);
    }

    std::string toString() const {
        return "TexcoordsIntegrator[]";
    }
};

REGISTER_CLASS(TexcoordsIntegrator, "texcoords")
