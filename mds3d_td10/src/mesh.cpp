#include "mesh.h"
#include "bvh.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include "shader.h"
#include <Eigen/Geometry>
#include <limits>

using namespace std;
using namespace Eigen;

Mesh::~Mesh()
{
  if(mIsInitialized)
  {
    glDeleteBuffers(1,&mVertexBufferId);
    glDeleteBuffers(1,&mIndexBufferId);
    glDeleteVertexArrays(1,&mVertexArrayId);
  }
}


bool Mesh::load(const std::string& filename)
{
    std::cout << "Loading: " << filename << std::endl;

    std::string ext = filename.substr(filename.size()-3,3);
    if(ext=="off" || ext=="OFF")
      return loadOFF(filename);
    else if(ext=="obj" || ext=="OBJ")
      return loadOBJ(filename);

    std::cerr << "Mesh: extension \'" << ext << "\' not supported." << std::endl;
    return false;
}

void Mesh::init()
{
    updateBoundingBox();
    updateBVH();

    glGenVertexArrays(1,&mVertexArrayId);
    glGenBuffers(1,&mVertexBufferId);
    glGenBuffers(1,&mIndexBufferId);

    updateVBO();

    mIsInitialized = true;
}

void Mesh::updateNormals()
{
    // pass 1: set the normal to 0
    for(std::vector<Vertex>::iterator v_iter = mVertices.begin() ; v_iter!=mVertices.end() ; ++v_iter)
        v_iter->normal.setZero();

    // pass 2: compute face normals and accumulate
    for(std::size_t j=0; j<mFaces.size(); ++j)
    {
        Vector3f v0 = mVertices[mFaces[j][0]].position;
        Vector3f v1 = mVertices[mFaces[j][1]].position;
        Vector3f v2 = mVertices[mFaces[j][2]].position;

        Vector3f n = (v1-v0).cross(v2-v0).normalized();

        mVertices[mFaces[j][0]].normal += n;
        mVertices[mFaces[j][1]].normal += n;
        mVertices[mFaces[j][2]].normal += n;
    }

    // pass 3: normalize
    for(std::vector<Vertex>::iterator v_iter = mVertices.begin() ; v_iter!=mVertices.end() ; ++v_iter)
        v_iter->normal.normalize();
}

void Mesh::updateVBO()
{
  glBindVertexArray(mVertexArrayId);

  // activate the VBO:
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId);
  // copy the data from host's RAM to GPU's video memory:
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*mVertices.size(), mVertices[0].position.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferId);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Vector3i)*mFaces.size(), mFaces[0].data(), GL_STATIC_DRAW);


}


void Mesh::draw(const Shader& shd)
{
    if (!mIsInitialized)
      init();

      // Activate the VBO of the current mesh:
  glBindVertexArray(mVertexArrayId);
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferId);

  // Specify vertex data

  // 1 - get id of the attribute "vtx_position" as declared as "in vec3 vtx_position" in the vertex shader
  int vertex_loc = shd.getAttribLocation("vtx_position");
  if(vertex_loc>=0)
  {
    // 2 - tells OpenGL where to find the x, y, and z coefficients:
    glVertexAttribPointer(vertex_loc,     // id of the attribute
                          3,              // number of coefficients (here 3 for x, y, z)
                          GL_FLOAT,       // type of the coefficients (here float)
                          GL_FALSE,       // for fixed-point number types only
                          sizeof(Vertex), // number of bytes between the x coefficient of two vertices
                                          // (e.g. number of bytes between x_0 and x_1)
                          0);             // number of bytes to get x_0
    // 3 - activate this stream of vertex attribute
    glEnableVertexAttribArray(vertex_loc);
  }

  int normal_loc = shd.getAttribLocation("vtx_normal");
  if(normal_loc>=0)
  {
    glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(Vector3f));
    glEnableVertexAttribArray(normal_loc);
  }

  int color_loc = shd.getAttribLocation("vtx_color");
  if(color_loc>=0)
  {
    glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(Vector3f)));
    glEnableVertexAttribArray(color_loc);
  }

  int texcoord_loc = shd.getAttribLocation("vtx_texcoord");
  if(texcoord_loc>=0)
  {
    glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(Vector3f)+sizeof(Vector4f)));
    glEnableVertexAttribArray(texcoord_loc);
  }

  // send the geometry
  glDrawElements(GL_TRIANGLES, 3*mFaces.size(), GL_UNSIGNED_INT, 0);

  // at this point the mesh has been drawn and raserized into the framebuffer!
  if(vertex_loc>=0)   glDisableVertexAttribArray(vertex_loc);
  if(normal_loc>=0)   glDisableVertexAttribArray(normal_loc);
  if(color_loc>=0)    glDisableVertexAttribArray(color_loc);
  if(texcoord_loc>=0) glDisableVertexAttribArray(texcoord_loc);

  checkError();
}


void Mesh::updateBoundingBox()
{
  mBBox.setNull();
  for(std::vector<Vertex>::iterator v_iter = mVertices.begin() ; v_iter!=mVertices.end() ; ++v_iter)
      mBBox.extend(v_iter->position);
}

void Mesh::updateBVH()
{
    if(mBVH)
      delete mBVH;
    mBVH = new BVH;
    mBVH->build(this, 10, 100);
}


bool Mesh::intersectFace(const Ray& ray, Hit& hit, int faceId) const
{
    Vector3f v0 = mVertices[mFaces[faceId][0]].position;
    Vector3f v1 = mVertices[mFaces[faceId][1]].position;
    Vector3f v2 = mVertices[mFaces[faceId][2]].position;
    Vector3f e1 = v1 - v0;
    Vector3f e2 = v2 - v0;
    Eigen::Matrix3f M;
    M << -ray.direction, e1, e2;
    Vector3f tuv = M.inverse() * (ray.origin - v0);
    float t = tuv(0), u = tuv(1), v = tuv(2);
    if(t>0 && u>=0 && v>=0 && (u+v)<=1 && t<hit.t())
    {
        hit.setT(t);

        hit.setFaceId(faceId);
        hit.setBaryCoords(Vector3f(u,v,1.-u-v));
        hit.setIntersectionPoint(u*v0+v*v1+(1.f-u-v)*v2);

        return true;
    }
    return false;
}

bool Mesh::intersect(const Ray& ray, Hit& hit) const
{
    if(mBVH)
    {
        // use the BVH !!
        return mBVH->intersect(ray, hit);
    }
    else
    {
        // brute force !!
        bool ret = false;
        float tMin, tMax;
        Normal3f normal;
        if( (!::intersect(ray, mBBox, tMin, tMax,normal)) || tMin>hit.t())
            return false;

        for(int i=0; i<nbFaces(); ++i)
        {
            ret = ret | intersectFace(ray, hit, i);
        }
        return ret;
    }
}



//********************************************************************************
// Loaders...
//********************************************************************************


bool Mesh::loadOFF(const std::string& filename)
{
  std::ifstream in(filename.c_str(),std::ios::in);
  if(!in)
  {
    std::cerr << "File not found " << filename << std::endl;
    return false;
  }

  std::string header;
  in >> header;

  bool hasColor = false;

  // check the header file
  if(header != "OFF")
  {
    if(header != "COFF")
    {
      std::cerr << "Wrong header = " << header << std::endl;
      return false;
    }
    hasColor = true;
  }

  int nofVertices, nofFaces, inull;
  int nb, id0, id1, id2;
  Vector3f v;

  in >> nofVertices >> nofFaces >> inull;

  for(int i=0 ; i<nofVertices ; ++i)
  {
    in >> v[0] >> v[1] >> v[2];
    mVertices.push_back(Vertex(v));

    if(hasColor) {
      Vector4f c;
      in >> c[0] >> c[1] >> c[2] >> c[3];
      mVertices[i].color = c/255.;
    }
  }

  for(int i=0 ; i<nofFaces ; ++i)
  {
    in >> nb >> id0 >> id1 >> id2;
    assert(nb==3);
    mFaces.push_back(Vector3i(id0, id1, id2));
  }

  in.close();

  updateNormals();

  return true;
}



#include <ObjFormat/ObjFormat.h>

bool Mesh::loadOBJ(const std::string& filename)
{
  ObjMesh* pRawObjMesh = ObjMesh::LoadFromFile(filename);

  if (!pRawObjMesh)
  {
      std::cerr << "Mesh::loadObj: error loading file " << filename << "." << std::endl;
      return false;
  }

  // Makes sure we have an indexed face set
  ObjMesh* pObjMesh = pRawObjMesh->createIndexedFaceSet(Obj::Options(Obj::AllAttribs|Obj::Triangulate));
  delete pRawObjMesh;
  pRawObjMesh = 0;

  // copy vertices
  mVertices.resize(pObjMesh->positions.size());

  for (std::size_t i=0 ; i<pObjMesh->positions.size() ; ++i)
  {
    mVertices[i] = Vertex(Vector3f(pObjMesh->positions[i].x, pObjMesh->positions[i].y, pObjMesh->positions[i].z));

    if(!pObjMesh->texcoords.empty())
      mVertices[i].texcoord = Vector2f(pObjMesh->texcoords[i]);

    if(!pObjMesh->normals.empty())
      mVertices[i].normal = Vector3f(pObjMesh->normals[i]);
  }

  // copy faces
  for (std::size_t smi=0 ; smi<pObjMesh->getNofSubMeshes() ; ++smi)
  {
    const ObjSubMesh* pSrcSubMesh = pObjMesh->getSubMesh(smi);

    mFaces.reserve(pSrcSubMesh->getNofFaces());

    for (std::size_t fid = 0 ; fid<pSrcSubMesh->getNofFaces() ; ++fid)
    {
      ObjConstFaceHandle srcFace = pSrcSubMesh->getFace(fid);
      mFaces.push_back(Vector3i(srcFace.vPositionId(0), srcFace.vPositionId(1), srcFace.vPositionId(2)));
    }
  }

  if(pObjMesh->normals.empty())
    updateNormals();

  return true;
}

