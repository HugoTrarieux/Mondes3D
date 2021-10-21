
#include "bvh.h"
#include "mesh.h"
#include <iostream>

void BVH::build(const Mesh* pMesh, int targetCellSize, int maxDepth)
{
    // store a pointer to the mesh
    m_pMesh = pMesh;
    // allocate the root node
    m_nodes.resize(1);

    if(m_pMesh->nbFaces() <= targetCellSize) { // only one node
        m_nodes[0].box = pMesh->AABB();
        m_nodes[0].first_face_id = 0;
        m_nodes[0].is_leaf = true;
        m_nodes[0].nb_faces = m_pMesh->nbFaces();
        m_faces.resize(m_pMesh->nbFaces());
        for(int i=0; i<m_pMesh->nbFaces(); ++i)
        {
            m_faces[i] = i;
        }
    }else{
        // reserve space for other nodes to avoid multiple memory reallocations
        m_nodes.reserve( std::min<int>(2<<maxDepth, std::log(m_pMesh->nbFaces()/targetCellSize) ) );

        // compute centroids and initialize the face list
        m_centroids.resize(m_pMesh->nbFaces());
        m_faces.resize(m_pMesh->nbFaces());
        for(int i=0; i<m_pMesh->nbFaces(); ++i)
        {
            m_centroids[i] = (m_pMesh->vertexOfFace(i, 0).position + m_pMesh->vertexOfFace(i, 1).position + m_pMesh->vertexOfFace(i, 2).position)/3.f;
            m_faces[i] = i;
        }

        // recursively build the BVH, starting from the root node and the entire list of faces
        buildNode(0, 0, m_pMesh->nbFaces(), 0, targetCellSize, maxDepth);
    }
}

bool BVH::intersect(const Ray& ray, Hit& hit) const
{
    float tMin, tMax;
    Normal3f n;
    if(::intersect(ray, m_nodes[0].box, tMin, tMax, n) && tMin > hit.t()){
        return intersectNode(0, ray, hit);  
    }
    return false;
}

bool BVH::intersectNode(int nodeId, const Ray& ray, Hit& hit) const
{
    const Node& node = m_nodes[nodeId];
    if(node.is_leaf){
      Hit hitFace = Hit();
      int nbFace = node.nb_faces;
      int firstFace = node.first_face_id;

      int i;
      for(i=firstFace; i < nbFace+firstFace; i++){
        if(m_pMesh->intersectFace(ray,hitFace,m_faces[i])){
          if(hit.t() >= hitFace.t()){
            hit.setT(hitFace.t());
            hit.setNormal(hitFace.normal());
            hit.setShape(hitFace.shape());
            hit.setTexcoord(hitFace.texcoord());
            return true;
          }
        }
      }
    }

    //Noeud Interne
    else{
        float tMin1,tMin2,tMax1,tMax2;
        int firstChildId = node.first_child_id;
        Normal3f normal;
        bool b = false;

        bool intersect_f1 = ::intersect(ray, m_nodes[firstChildId].box, tMin1, tMax1, normal);
        bool intersect_f2 = ::intersect(ray, m_nodes[firstChildId+1].box, tMin2, tMax2, normal);

        if(intersect_f1 && intersect_f2 && tMin2<tMax1){
            bool t1 = intersectNode(firstChildId,ray,hit);
            bool t2 = intersectNode(firstChildId+1,ray,hit);
            b = t1 || t2;
        } else {
            if(intersect_f1 && (intersect_f2 && !b) ){
                b = intersectNode(firstChildId+1,ray,hit);
            } else{
                b = intersectNode(firstChildId,ray,hit);
            }
            if(!intersect_f1 && intersect_f2 ){
                b = intersectNode(firstChildId+1,ray,hit);
            }
        }
        return b;
    }
    return false;
}

/** Sorts the faces with respect to their centroid along the dimension \a dim and spliting value \a split_value.
  * \returns the middle index
  */
int BVH::split(int start, int end, int dim, float split_value)
{
    int l(start), r(end-1);
    while(l<r)
    {
        // find the first on the left
        while(l<end && m_centroids[l](dim) < split_value) ++l;
        while(r>=start && m_centroids[r](dim) >= split_value) --r;
        if(l>r) break;
        std::swap(m_centroids[l], m_centroids[r]);
        std::swap(m_faces[l], m_faces[r]);
        ++l;
        --r;
    }
    return m_centroids[l][dim]<split_value ? l+1 : l;
}

void BVH::buildNode(int nodeId, int start, int end, int level, int targetCellSize, int maxDepth)
{
    Node& node = m_nodes[nodeId];

    // étape 1 : calculer la boite englobante des faces indexées de m_faces[start] à m_faces[end]
    // (Utiliser la fonction extend de Eigen::AlignedBox3f et la fonction mpMesh->vertexOfFace(int) pour obtenir les coordonnées des sommets des faces)
    Eigen::AlignedBox3f box;
    for(int i=start; i<end; i++){
        box = box.extend(m_pMesh->vertexOfFace(m_faces[i],0).position);
        box = box.extend(m_pMesh->vertexOfFace(m_faces[i],1).position);
        box = box.extend(m_pMesh->vertexOfFace(m_faces[i],2).position);
    }
    node.nb_faces = end - start;
    node.box = box;

    // étape 2 : déterminer si il s'agit d'une feuille (appliquer les critères d'arrêts)
    // Si c'est une feuille, finaliser le noeud et quitter la fonction
    if(level == maxDepth || node.nb_faces <= targetCellSize){
        node.is_leaf = true;
        node.first_face_id = start;
        return;
    }

    // Si c'est un noeud interne :
    // étape 3 : calculer l'index de la dimension (x=0, y=1, ou z=2) et la valeur du plan de coupe
    // (on découpe au milieu de la boite selon la plus grande dimension)
    else{
        Vector3f min = node.box.min();
        Vector3f max = node.box.max();
        float x = node.box.sizes().x();
        float y = node.box.sizes().y();
        float z = node.box.sizes().z();
        float split_value;
        int dim;

        if(x >= y && x >= z){
            split_value = (min.x() + max.x()) * 0.5;
            dim = 0;
        }
        else if(y >= x){
            split_value = (min.y() + max.y()) * 0.5;
            dim = 1;
        }
        else{
            split_value = (min.z() + max.z()) * 0.5;
            dim = 2;
        }

    // étape 4 : appeler la fonction split pour trier (partiellement) les faces et vérifier si le split a été utile
    int _split = split(start, end, dim, split_value);
    if(_split == start || _split == end){
        node.is_leaf = true;
        node.first_face_id = start;
        return;
    }

    // étape 5 : allouer les fils, et les construire en appelant buildNode...
    int nodeSize = m_nodes.size();
    node.first_child_id = nodeSize;
    m_nodes.resize(nodeSize + 2);

    buildNode(node.first_child_id, start, _split, level+1, targetCellSize, maxDepth);
    buildNode(node.first_child_id+1, _split, end, level+1, targetCellSize, maxDepth);
  }
}


