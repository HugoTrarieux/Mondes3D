#include "material.h"

Phong::Phong(const PropertyList &propList)
    : Diffuse(propList.getColor("diffuse",Color3f(0.2)))
{
    m_specularColor = propList.getColor("specular",Color3f(0.9));
    m_exponent = propList.getFloat("exponent",0.2);
    m_reflectivity = propList.getColor("reflectivity",Color3f(0.0));
    m_transmissivness = propList.getColor("transmissivness",Color3f(0.0));
    m_etaA = propList.getFloat("etaA",1);
    m_etaB = propList.getFloat("etaB",1);

    std::string texturePath = propList.getString("texture","");
    if(texturePath.size()>0){
        filesystem::path filepath = getFileResolver()->resolve(texturePath);
        loadTextureFromFile(filepath.str());
        setTextureScale(propList.getFloat("scale",1));
        setTextureMode(TextureMode(propList.getInteger("mode",0)));
    }
}

Color3f Phong::brdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const
{
    Vector3f reflected = -lightDir + 2.f*(lightDir.dot(normal))*normal;
    float alpha = viewDir.dot(reflected);

    Color3f color = diffuseColor(uv);// * M_1_PI;

    if (alpha > 0.0f) {
        //color +=  m_specularColor * ((m_exponent + 2.f) * std::pow(alpha, m_exponent) / (2.f* M_PI));
        color +=  m_specularColor * std::pow(alpha, m_exponent);
    }

    return color;
}

std::string Phong::toString() const {
    return tfm::format(
        "Phong[\n"
        "  diffuse color = %s\n"
        "  specular color = %s\n"
        "  exponent = %f\n"
        "]", m_diffuseColor.toString(),
             m_specularColor.toString(),
             m_exponent);
}

REGISTER_CLASS(Phong, "phong")
