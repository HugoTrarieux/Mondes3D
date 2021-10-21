#include "bvh.h"
#include "mesh.h"
// #include "glPrimitives.h"
#include <iostream>
#include <algorithm>

void BVH::build(const Mesh* pMesh, int targetCellSize, int maxDepth)
{
    m_splitMethod = SPLIT_MIDDLE;
    m_pMesh = pMesh;
    m_nodes.resize(1);
    if(m_pMesh->nbFaces() <= targetCellSize) {
        m_nodes[0].box = pMesh->boundingBox();
        m_nodes[0].first_face_id = 0;
        m_nodes[0].is_leaf = true;
        m_nodes[0].nb_faces = m_pMesh->nbFaces();
        m_faces.resize(m_pMesh->nbFaces());
        for(int i=0; i<m_pMesh->nbFaces(); ++i)
        {
            m_faces[i] = i;
        }
    }else{
        m_nodes.reserve( std::min<int>(2<<maxDepth, std::log(m_pMesh->nbFaces()/targetCellSize) ) );
        // compute centroids and initialize the face list
        m_centroids.resize(m_pMesh->nbFaces());
        m_faces.resize(m_pMesh->nbFaces());
        for(int i=0; i<m_pMesh->nbFaces(); ++i)
        {
            m_centroids[i] = (m_pMesh->vertexOfFace(i, 0).position + m_pMesh->vertexOfFace(i, 1).position + m_pMesh->vertexOfFace(i, 2).position)/3.f;
            m_faces[i] = i;
        }
        buildNode(0, 0, m_pMesh->nbFaces(), 0, targetCellSize, maxDepth);
    }
}

bool BVH::intersect(const Ray& ray, Hit& hit) const
{
    float tMin, tMax;
    Normal3f normal;
    ::intersect(ray, m_nodes[0].box, tMin, tMax, normal);
    if(tMax>0 && tMax>=tMin && tMin<hit.t())
        return intersectNode(0, ray, hit);
    return false;
}


bool BVH::intersectNode(int nodeId, const Ray& ray, Hit& hit) const
{
    const Node& node = m_nodes[nodeId];

    if(node.is_leaf)
    {
        int end = node.first_child_id+node.nb_faces;
        bool found = false;
        for(int i=node.first_child_id; i<end; ++i)
        {
            found = found | m_pMesh->intersectFace(ray, hit, m_faces[i]);
        }
        return found;
    }
    else
    {
        float tMin1, tMax1, tMin2, tMax2;
        int child_id1 = node.first_child_id;
        int child_id2 = node.first_child_id+1;
        Normal3f normal;
        ::intersect(ray, m_nodes[child_id1].box, tMin1, tMax1, normal);
        ::intersect(ray, m_nodes[child_id2].box, tMin2, tMax2, normal);
        if(tMin1 > tMin2)
        {
            std::swap(tMin1, tMin2);
            std::swap(tMax1, tMax2);
            std::swap(child_id1, child_id2);
        }

        bool found = false;
        if(tMin1 < hit.t() && tMin1<=tMax1 && tMax1>0)
        {
            found = intersectNode(child_id1, ray, hit);
        }
        if(tMin2 < hit.t() && tMin2<=tMax2 && tMax2>0)
        {
            found = found | intersectNode(child_id2, ray, hit);
        }
        return found;
    }

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

// box surface area = 2lw + 2lh + 2wh
float surfaceArea(Eigen::AlignedBox3f& aabb)
{
    Vector3f diag = aabb.max() - aabb.min();
    return 2.f*(diag[0]*diag[1] + diag[0]*diag[2] + diag[1]*diag[2]);
}

void BVH::buildNode(int nodeId, int start, int end, int level, int targetCellSize, int maxDepth)
{
    Node& node = m_nodes[nodeId];

    // compute bounding box
    Eigen::AlignedBox3f aabb;
    aabb.setNull();
    for(int i=start; i<end; ++i)
    {
        // Attention pas m_pMesh->vertexOfFace(i
        aabb.extend(m_pMesh->vertexOfFace(m_faces[i], 0).position);
        aabb.extend(m_pMesh->vertexOfFace(m_faces[i], 1).position);
        aabb.extend(m_pMesh->vertexOfFace(m_faces[i], 2).position);
    }
    node.box = aabb;

    // stopping criteria
    if(end-start <= targetCellSize || level>=maxDepth)
    {
        // we got a leaf !
        node.is_leaf = true;
        node.first_face_id = start;
        node.nb_faces = end-start;
        assert(node.nb_faces < m_faces.size());
        return;
    }
    node.is_leaf = false;

    // Split along the largest dimension
    Vector3f diag = aabb.max() - aabb.min();
    int dim;
    diag.maxCoeff(&dim);

    float split_value;

    if(m_splitMethod == SPLIT_EQUAL_COUNTS)
    {
        /// TODO: Split at the mediane
        split_value = 0.5 * (aabb.max()[dim] + aabb.min()[dim]);
    }
    else if(m_splitMethod == SPLIT_SAH)
    {
        // Split using SAH
        // Init buckets
        for(int i = 0; i < nBuckets; ++i) {
            m_buckets[i].count = 0;
            m_buckets[i].bounds.setNull();
        }
        for(int i = start; i < end; ++i) {
            int b = nBuckets * ((m_centroids[i](dim) - aabb.min()(dim)) / diag(dim));
            if (b == nBuckets) b = nBuckets-1;
            m_buckets[b].count++;
            m_buckets[b].bounds.extend(m_pMesh->vertexOfFace(m_faces[i], 0).position);
            m_buckets[b].bounds.extend(m_pMesh->vertexOfFace(m_faces[i], 1).position);
            m_buckets[b].bounds.extend(m_pMesh->vertexOfFace(m_faces[i], 2).position);
        }

        // Compute cost per bucket
        float cost[nBuckets-1];
        float mainSA = surfaceArea(aabb);
        for(int i = 0; i < nBuckets-1; ++i) {
               Eigen::AlignedBox3f b0, b1;
               b0.setNull();
               b1.setNull();
               int count0 = 0, count1 = 0;
               for (int j = 0; j <= i; ++j) {
                   b0.extend(m_buckets[j].bounds);
                   count0 += m_buckets[j].count;
               }
               for (int j = i+1; j < nBuckets; ++j) {
                   b1.extend(m_buckets[j].bounds);
                   count1 += m_buckets[j].count;
               }
               cost[i] = .125f;
               if(count0 > 0)
                    cost[i] += count0 * surfaceArea(b0) / mainSA;
                if(count1 > 0)
                    cost[i] += count1 * surfaceArea(b1) / mainSA;
        }
        // Find bucket to split at that minimizes SAH metric
        float minCost = cost[0];
        int minCostSplit = 0;
        for(int i = 1; i < nBuckets-1; ++i) {
            if (cost[i] < minCost) {
                minCost = cost[i];
                minCostSplit  = i;
            }
        }
        split_value = aabb.min()[dim] + (minCostSplit+1) * diag(dim) / nBuckets;
        //m_buckets[minCostSplit].bounds.max()[dim];
        //0.5 * (m_buckets[minCostSplit].bounds.max()[dim] + m_buckets[minCostSplit].bounds.min()[dim]);
    }else{
        // Split at the middle
        split_value = 0.5 * (aabb.max()[dim] + aabb.min()[dim]);
    }

    // Sort the faces according to the split plane
    int mid_id = split(start, end, dim, split_value);

    // second stopping criteria
    if(mid_id==start || mid_id>=end)
    {
        // no improvement
        node.is_leaf = true;
        node.first_face_id = start;
        node.nb_faces = end-start;
        assert(node.nb_faces < m_faces.size());
        return;
    }

    // create the children
    int child_id = node.first_child_id = m_nodes.size();
    m_nodes.resize(m_nodes.size()+2);
    // node is not a valid reference anymore !

    buildNode(child_id  , start, mid_id, level+1, targetCellSize, maxDepth);
    buildNode(child_id+1, mid_id, end, level+1, targetCellSize, maxDepth);
}

