#include "integrator.h"
#include "scene.h"
#include "material.h"

class Direct : public Integrator
{
public:
    Direct(const PropertyList &props) {}

    Color3f Li(const Scene *scene, const Ray &ray) const {
        /* Find the surface that is visible in the requested direction */
        Hit hit;
        scene->intersect(ray, hit);
        if (!hit.foundIntersection())
            return scene->backgroundColor();

        Color3f radiance = Color3f::Zero();
        const Material* material = hit.shape()->material();

        Normal3f normal = hit.normal();
        Point3f pos = ray.at(hit.t());

        const LightList &lights = scene->lightList();
        for(LightList::const_iterator it=lights.begin(); it!=lights.end(); ++it)
        {
            float dist;
            Vector3f lightDir = (*it)->direction(pos, &dist);
            Ray shadowRay(pos + normal*Epsilon, lightDir,true);
            Hit shadowHit;
            scene->intersect(shadowRay,shadowHit);
            if(!shadowHit.foundIntersection() || shadowHit.t() > dist) {
                float cos_term = std::max(0.f,lightDir.dot(normal));
                Color3f brdf = material->brdf(-ray.direction, lightDir, normal, hit.texcoord());
                radiance += (*it)->intensity(pos) * cos_term * brdf;
            }
        }

        return radiance;
    }

    std::string toString() const {
        return "Direct[]";
    }
};

REGISTER_CLASS(Direct, "direct")
