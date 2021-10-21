#include "integrator.h"
#include "scene.h"

class Whitted : public Integrator {
    
private:
    int maxRecursion;

public:
    Whitted(const PropertyList &props) {
        maxRecursion = props.getInteger("maxRecursion", 0);
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {
        Hit hitShape = Hit();
        scene->intersect(ray, hitShape);

        if(hitShape.foundIntersection()){
            Color3f material;
            Point3f pHit = ray.at(hitShape.t());
            float dist = hitShape.t();
            Vector3f lightDirection;
            Color3f lightIntensity;
            Normal3f normalV = hitShape.normal();
            Color3f color;
            float costerm;
            float epsilon = 0.0001;

            for(auto light : scene->lightList()){
                lightDirection = light->direction(pHit, &dist);
                Hit hitShadow = Hit();
                pHit += epsilon*normalV;
                Ray rayShadow = Ray(pHit, lightDirection);
                scene->intersect(rayShadow, hitShadow);
                bool obstruction = hitShadow.foundIntersection();
                if(!obstruction){
                    material = hitShape.shape()->material()->brdf(ray.direction, lightDirection, normalV);
                    costerm = lightDirection.dot(normalV);
                    lightIntensity = light->intensity(pHit);
                    color += material * fmax(costerm, 0) * lightIntensity;
                } else{
                    color += 0; //Current Light blocked by an object, color not modified by current light
                }
            }
            if(ray.recursionLevel <  maxRecursion){
                Vector3f dir = 2*normalV.dot(-ray.direction) * normalV + ray.direction;
                dir.normalize();
                Ray rayRebound(pHit, dir);
                rayRebound.recursionLevel = ray.recursionLevel + 1;
                color += hitShape.shape()->material()->reflectivity() * Li(scene, rayRebound);
            } 
            return color;
        } else {
            return scene->backgroundColor();
        }
    }

    std::string toString() const {
        return "Whitted[]";
    }
};

REGISTER_CLASS(Whitted, "whitted");