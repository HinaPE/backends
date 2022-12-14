

/*
	Please see https://github.com/GPUOpen-LibrariesAndSDKs/RadeonRays_SDK for the original code
	Code is modfied for this project
*/

#include "BvhTranslator.h"

#include <cassert>
#include <stack>
#include <iostream>

#include "math/Bounds3D.h"

namespace RadeonRays
{
	int BvhTranslator::ProcessBLASNodes(const Bvh::Node* node)
	{
		Bounds3D bbox = node->bounds;

		bboxmin[curNode] = bbox.min;
		bboxmax[curNode] = bbox.max;
		nodes[curNode].leaf = 0;

		int index = curNode;

		if (node->type == RadeonRays::Bvh::NodeType::kLeaf)
		{
			nodes[curNode].leftIndex  = curTriIndex + node->startidx;
			nodes[curNode].rightIndex = node->numprims;
			nodes[curNode].leaf = 1;
		}
		else
		{
			curNode++;
			nodes[index].leftIndex = ProcessBLASNodes(node->lc);
			nodes[index].leftIndex = ((nodes[index].leftIndex % nodeTexWidth) << 12) | (nodes[index].leftIndex / nodeTexWidth);
			curNode++;
			nodes[index].rightIndex = ProcessBLASNodes(node->rc);
			nodes[index].rightIndex = ((nodes[index].rightIndex % nodeTexWidth) << 12) | (nodes[index].rightIndex / nodeTexWidth);
		}

		return index;
	}

	int BvhTranslator::ProcessTLASNodes(const Bvh::Node* node)
	{
		Bounds3D bbox = node->bounds;

		bboxmin[curNode] = bbox.min;
		bboxmax[curNode] = bbox.max;
		nodes[curNode].leaf = 0;

		int index = curNode;

		if (node->type == RadeonRays::Bvh::NodeType::kLeaf)
		{
			int instanceIndex = TLBvh->m_PackedIndices[node->startidx];
			int meshIndex  = meshInstances[instanceIndex].meshID;
			int materialID = meshInstances[instanceIndex].materialID;

			nodes[curNode].leftIndex  = (bvhRootStartIndices[meshIndex] % nodeTexWidth) << 12 | (bvhRootStartIndices[meshIndex] / nodeTexWidth);
			nodes[curNode].rightIndex = materialID;
			nodes[curNode].leaf = -instanceIndex - 1;
		}
		else
		{
			curNode++;
			nodes[index].leftIndex = ProcessTLASNodes(node->lc);
			nodes[index].leftIndex = ((nodes[index].leftIndex % nodeTexWidth) << 12) | (nodes[index].leftIndex / nodeTexWidth);
			curNode++;
			nodes[index].rightIndex = ProcessTLASNodes(node->rc);
			nodes[index].rightIndex = ((nodes[index].rightIndex % nodeTexWidth) << 12) | (nodes[index].rightIndex / nodeTexWidth);
		}
		return index;
	}
	
	void BvhTranslator::ProcessBLAS()
	{
		int nodeCnt = 0;

		for (int i = 0; i < meshes.size(); ++i) {
			nodeCnt += meshes[i]->bvh->m_Nodecnt;
		}
		
		topLevelIndex = nodeCnt;

		// reserve space for top level nodes
		nodeCnt += 2 * meshInstances.size();
		nodeTexWidth = (int)(sqrt(nodeCnt) + 1);

		// Resize to power of 2
		bboxmin.resize(nodeTexWidth * nodeTexWidth);
		bboxmax.resize(nodeTexWidth * nodeTexWidth);
		nodes.resize(nodeTexWidth * nodeTexWidth);

		int bvhRootIndex = 0;
		curTriIndex = 0;

		for (int i = 0; i < meshes.size(); i++)
		{
			GLSLPT::Mesh *mesh = meshes[i];
			curNode = bvhRootIndex;

			bvhRootStartIndices.push_back(bvhRootIndex);
			bvhRootIndex += mesh->bvh->m_Nodecnt;
			
			ProcessBLASNodes(mesh->bvh->m_Root);
			curTriIndex += mesh->bvh->GetNumIndices();
		}
	}

	void BvhTranslator::ProcessTLAS()
	{
		curNode = topLevelIndex;
		topLevelIndexPackedXY = ((topLevelIndex % nodeTexWidth) << 12) | (topLevelIndex / nodeTexWidth);
		ProcessTLASNodes(TLBvh->m_Root);
	}

	void BvhTranslator::UpdateTLAS(const Bvh* topLevelBvh, const std::vector<GLSLPT::MeshInstance>& sceneInstances)
	{
		TLBvh = topLevelBvh;
		curNode = topLevelIndex;
		meshInstances = sceneInstances;
		ProcessTLASNodes(TLBvh->m_Root);
	}

	void BvhTranslator::Process(const Bvh* topLevelBvh, const std::vector<GLSLPT::Mesh*>& sceneMeshes, const std::vector<GLSLPT::MeshInstance>& sceneInstances)
	{
		TLBvh = topLevelBvh;
		meshes = sceneMeshes;
		meshInstances = sceneInstances;
		ProcessBLAS();
		ProcessTLAS();
	}
}
