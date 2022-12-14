
#pragma once

#ifndef BVH_H
#define BVH_H

#include <vector>

#include "math/Bounds3D.h"

namespace RadeonRays
{
    class Bvh
    {
    public:
        Bvh(float traversalCost, int numBins = 64, bool usesah = false)
            : m_Root(nullptr)
            , m_Usesah(usesah)
            , m_Height(0)
            , m_TraversalCost(traversalCost)
            , m_NumBins(numBins)
        {
            
        }

        virtual ~Bvh()
        {
            
        }

		// Build function
		// bounds is an array of bounding boxes
		void Build(const Bounds3D* bounds, int numbounds);

        // World space bounding box
		const Bounds3D& Bounds() const
		{
			return m_Bounds;
		}

		// Get tree height
		int GetHeight() const
		{
			return m_Height;
		}

        // Get reordered prim indices Nodes are pointing to
		virtual const int* GetIndices() const
		{
			return &m_PackedIndices[0];
		}

        // Get number of indices.
        // This number can differ from numbounds passed to Build function for
        // some BVH implementations (like SBVH)
		virtual size_t GetNumIndices() const
		{
			return m_PackedIndices.size();
		}

	protected:

		// Enum for node type
		enum NodeType
		{
			kInternal,
			kLeaf
		};

		// BVH node
		struct Node
		{
			// Node bounds in world space
			Bounds3D bounds;
			// Type of the node
			NodeType type;
			// Node index in a complete tree
			int index;

			union
			{
				// For internal nodes: left and right children
				struct
				{
					Node* lc;
					Node* rc;
				};

				// For leaves: starting primitive index and number of primitives
				struct
				{
					int startidx;
					int numprims;
				};
			};
		};

		struct SplitRequest
		{
			// Starting index of a request
			int startidx;
			// Number of primitives
			int numprims;
			// Root node
			Node** ptr;
			// Bounding box
			Bounds3D bounds;
			// Centroid bounds
			Bounds3D centroidBounds;
			// Level
			int level;
			// Node index
			int index;
		};

		struct SahSplit
		{
			int dim;
			float split;
			float sah;
			float overlap;
		};

    protected:

        // Build function
        virtual void BuildImpl(const Bounds3D* bounds, int numbounds);

        // Node allocation
        virtual Node* AllocateNode();

        virtual void InitNodeAllocator(size_t maxnum);

        void BuildNode(const SplitRequest& req, const Bounds3D* bounds, const Vector3* centroids, int* primindices);

        SahSplit FindSahSplit(const SplitRequest& req, const Bounds3D* bounds, const Vector3* centroids, int* primindices) const;

        // Bvh nodes
        std::vector<Node> m_Nodes;
        // Identifiers of leaf primitives
        std::vector<int> m_Indices;
        // Node allocator counter, atomic for thread safety
		int m_Nodecnt;
        // Identifiers of leaf primitives
        std::vector<int> m_PackedIndices;
        // Bounding box containing all primitives
		Bounds3D m_Bounds;
        // Root node
        Node* m_Root;
        // SAH flag
        bool m_Usesah;
        // Tree height
        int m_Height;
        // Node traversal cost
        float m_TraversalCost;
        // Number of spatial bins to use for SAH
        int m_NumBins;

    private:

        Bvh(const Bvh& bvh) = delete;

        Bvh& operator = (const Bvh& bvh) = delete;

		friend class BvhTranslator;
    };
}

#endif // BVH_H
