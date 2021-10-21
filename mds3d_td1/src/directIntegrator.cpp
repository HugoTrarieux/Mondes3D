#include "integrator.h"
#include "scene.h"

class Direct : public Integrator {

public:
    Direct(const PropertyList &props) {
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {
        Hit hitShape = Hit();
        scene->intersect(ray, hitShape);
        if(hitShape.foundIntersection()){
            Color3f material;
            Point3f pHit = ray.at(hitShape.t());
            float distLight;
            Vector3f lightDirection;
            Color3f lightIntensity;
            Normal3f normalV = hitShape.normal();
            Color3f color;
            float costerm;
            float epsilon = 0.0001;

            for(auto light : scene->lightList()){
                lightDirection = light->direction(pHit, &distLight);
                Hit hitShadow = Hit();
                pHit += epsilon*normalV;
                Ray rayShadow = Ray(pHit, lightDirection);
                scene->intersect(rayShadow, hitShadow);
                bool obstruction = hitShadow.foundIntersection();
                bool distIsCloser = hitShadow.t() < distLight;
                if(!obstruction && !distIsCloser){
                    material = hitShape.shape()->material()->brdf(ray.direction, lightDirection, normalV);
                    costerm = lightDirection.dot(normalV);
                    lightIntensity = light->intensity(pHit);
                    color += material * fmax(costerm, 0) * lightIntensity;
                } else{
                    color += 0; //Current Light blocked by an object, color not modified by current light
                }
            }
            return color;
        } else {
            return Color3f(0.f);
        }
    }

    std::string toString() const {
        return "Direct[]";
    }
};

REGISTER_CLASS(Direct, "direct");