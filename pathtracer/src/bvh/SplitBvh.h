
#pragma once

#include <list>

#include "Bvh.h"

namespace RadeonRays
{
    class SplitBvh : public Bvh
    {
    public:
        SplitBvh(float traversalCost, int numBins, int maxSplitDepth,  float minOverlap, float extraRefsBudget)
			: Bvh(traversalCost, numBins, true)
			, m_MaxSplitDepth(maxSplitDepth)
			, m_MinOverlap(minOverlap)
			, m_ExtraRefsBudget(extraRefsBudget)
			, m_NumNodesRequired(0)
			, m_NumNodesForRegular(0)
			, m_NumNodesArchived(0)
        {

        }

        ~SplitBvh() = default;

    protected:

		struct PrimRef
		{
			// Prim bounds
			Bounds3D bounds;
			Vector3 center;
			int idx;
		};

        using PrimRefArray = std::vector<PrimRef>;
        
        enum class SplitType
        {
            kObject,
            kSpatial
        };

        // Build function
        void BuildImpl(const Bounds3D* bounds, int numbounds) override;
        void BuildNode(SplitRequest& req, PrimRefArray& primrefs);
        
        SahSplit FindObjectSahSplit(const SplitRequest& req, const PrimRefArray& refs) const;
        SahSplit FindSpatialSahSplit(const SplitRequest& req, const PrimRefArray& refs) const;
        
        void SplitPrimRefs(const SahSplit& split, const SplitRequest& req, PrimRefArray& refs, int& extra_refs);
        bool SplitPrimRef(const PrimRef& ref, int axis, float split, PrimRef& leftref, PrimRef& rightref) const;

    protected:

        Node* AllocateNode() override;

        void InitNodeAllocator(size_t maxnum) override;

    private:

        int m_MaxSplitDepth;
        float m_MinOverlap;
        float m_ExtraRefsBudget;
        int m_NumNodesRequired;
        int m_NumNodesForRegular;

        // Node archive for memory management
        // As m_nodes fills up we archive it into m_node_archive
        // allocate new chunk and work there.

        // How many nodes have been archived so far
        int m_NumNodesArchived;
        // Container for archived chunks
        std::list<std::vector<Node>> m_NodeArchive;

	private:
        SplitBvh(const SplitBvh& bvh) = delete;

        SplitBvh& operator = (const SplitBvh& bvh) = delete;
    };
    
}