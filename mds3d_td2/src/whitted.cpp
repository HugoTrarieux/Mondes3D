#include "integrator.h"
#include "scene.h"
#include "material.h"

class Whitted : public Integrator
{
public:
    Whitted(const PropertyList &props) {
        m_maxRecursion = props.getInteger("maxRecursion",4);
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {

        Color3f radiance = Color3f::Zero();

        // stopping criteria:
        if(ray.recursionLevel>=m_maxRecursion) {
            return radiance;
        }

        /* Find the surface that is visible in the requested direction */
        Hit hit;
        scene->intersect(ray, hit);
        if (hit.foundIntersection())
        {
            Point3f pos = ray.at(hit.t());
            Normal3f normal = hit.normal();

            const Material* material = hit.shape()->material();

            const LightList &lights = scene->lightList();
            for(LightList::const_iterator it=lights.begin(); it!=lights.end(); ++it)
            {
                float dist;
                Vector3f lightDir = (*it)->direction(pos, &dist);
                Ray shadowRay(pos + normal*1e-4, lightDir, true);
                Hit shadowHit;
                scene->intersect(shadowRay,shadowHit);
                Color3f attenuation = Color3f(1.f);
                if(shadowHit.t()<dist){
                    attenuation = 0.5f * shadowHit.shape()->material()->transmissivness();
                    if((attenuation <= 1e-6).all())
                        continue;
                }

                float cos_term = std::max(0.f,lightDir.dot(normal));
                Color3f brdf = material->brdf(-ray.direction, lightDir, normal, hit.texcoord());
                radiance += (*it)->intensity(pos) * cos_term * brdf * attenuation;
            }

            // reflexions
            if((material->reflectivity() > 1e-6).any())
            {
                Vector3f r = (ray.direction - 2.*ray.direction.dot(hit.normal())*hit.normal()).normalized();
                Ray reflexion_ray(pos + hit.normal()*1e-4, r);
                reflexion_ray.recursionLevel = ray.recursionLevel + 1;
                float cos_term = std::max(0.f,r.dot(normal));
                radiance += material->reflectivity() * Li(scene, reflexion_ray) * cos_term;
            }

            // refraction
            if((material->transmissivness() > 1e-6).any())
            {
                float etaA = material->etaA(), etaB = material->etaB();
                bool entering = -normal.dot(ray.direction) > 0;
                if(!entering) {
                    std::swap(etaA,etaB);
                    normal = -normal;
                }
                Vector3f r;
                if(refract(normal,ray.direction,etaA,etaB,r)) {
                    Ray refraction_ray(pos - normal*1e-4, r);
                    refraction_ray.recursionLevel = ray.recursionLevel + 1;
                    float cos_term = std::max(0.f,-r.dot(normal));
                    radiance += material->transmissivness() * Li(scene, refraction_ray) * cos_term;
                }
            }

        }else if(ray.recursionLevel == 0)
            return scene->backgroundColor();

        return radiance;
    }

    std::string toString() const {
        return tfm::format("Whitted[\n"
                           "  max recursion = %f\n"
                           " ]\n",
                           m_maxRecursion);
    }
private:
    int m_maxRecursion;
};

REGISTER_CLASS(Whitted, "whitted")
