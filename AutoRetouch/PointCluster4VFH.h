#pragma once
#include "PointCluster.h"

namespace hiveObliquePhotography
{
	namespace AutoRetouch
	{
		class CPointCluster4VFH : public IPointCluster
		{
		public:
			CPointCluster4VFH(const pcl::IndicesPtr& vPointIndices, EPointLabel vLabel);
			~CPointCluster4VFH() override = default;

			double computeDistanceV(pcl::index_t vPointIndex) const override;

		private:
			void __computeVFHDescriptor(const pcl::Indices& vPointIndices, Eigen::Matrix<float, 308, 1>& voVFHDescriptor) const;

			Eigen::Matrix<float, 308, 1> m_VFHDescriptor;
		};
	}
}