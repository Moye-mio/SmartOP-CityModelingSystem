#pragma once
#include "MeshSuture.h"

namespace hiveObliquePhotography
{
	namespace SceneReconstruction
	{
		class CBasicMeshSuture : public IMeshSuture
		{
		public:
			CBasicMeshSuture() = default;
			~CBasicMeshSuture() = default;

			virtual void sutureMeshes() override;
			void dumpMeshes(CMesh& voLHSMesh, CMesh& voRHSMesh);
		
		private:
			Eigen::Vector4f __calcSegmentPlane(pcl::PointCloud<pcl::PointXYZ>::Ptr vCloudOne, pcl::PointCloud<pcl::PointXYZ>::Ptr vCloudTwo);
			void __executeIntersection(CMesh& vioMesh, const Eigen::Vector4f& vPlane, std::vector<int>& voDissociatedIndices, std::vector<SVertex>& voIntersectionPoints);
			void __generatePublicVertices(const std::vector<SVertex>& vLHSIntersectionPoints, const std::vector<SVertex>& vRHSIntersectionPoints, std::vector<SVertex>& voPublicVertices);
			void __connectVerticesWithMesh(CMesh& vioMesh, std::vector<int>& vDissociatedIndices, std::vector<SVertex>& vPublicVertices);
			void __removeUnreferencedVertex(CMesh& vioMesh);
			double __computeDistance(const SVertex& vLHSVertex, const SVertex& vRHSVertex);
			SVertex __interpolatePoint(const SVertex& vLHSVertex, const SVertex& vRHSVertex);
			SVertex __findNearestPoint(const std::vector<SVertex>& vVectexSet, const SVertex& vVertex);

			std::vector<SFace> __genConnectionFace(IndexType vNumLeft, IndexType vNumRight, bool vLeftBeforeRight, bool vIsClockwise = true);
		};
	}
}

