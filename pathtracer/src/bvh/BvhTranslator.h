

/* 
	Code is not the original code from RadeonRays. It is modfied slightly for this project 
	Please refer to https://github.com/GPUOpen-LibrariesAndSDKs/RadeonRays_SDK for the original code
*/

#pragma once

#ifndef BVH_TRANSLATOR_H
#define BVH_TRANSLATOR_H

#include <map>

#include "Bvh.h"
#include "core/Mesh.h"

namespace RadeonRays
{
    /// This class translates pointer based BVH representation into
    /// index based one suitable for feeding to GPU or any other accelerator
    //
    class BvhTranslator
    {
    public:

        // Constructor
        BvhTranslator() = default;

		struct Node
		{
			int leftIndex;
			int rightIndex;
			int leaf;
		};

		void ProcessBLAS();
		void ProcessTLAS();
		void UpdateTLAS(const Bvh* topLevelBvh, const std::vector<GLSLPT::MeshInstance>& instances);
		void Process(const Bvh* topLevelBvh, const std::vector<GLSLPT::Mesh*>& meshes, const std::vector<GLSLPT::MeshInstance>& instances);
		
	private:
		int ProcessBLASNodes(const Bvh::Node* root);
		int ProcessTLASNodes(const Bvh::Node* root);

	public:
		std::vector<Vector3> bboxmin;
		std::vector<Vector3> bboxmax;
		std::vector<Node> nodes;
		int nodeTexWidth;
		int topLevelIndexPackedXY = 0;
		int topLevelIndex = 0;

    private:
		int curNode = 0;
		int curTriIndex = 0;
		const Bvh* TLBvh;
		std::vector<int> bvhRootStartIndices;
		std::vector<GLSLPT::MeshInstance> meshInstances;
		std::vector<GLSLPT::Mesh*> meshes;
    };
}

#endif // BVH_TRANSLATOR_H
