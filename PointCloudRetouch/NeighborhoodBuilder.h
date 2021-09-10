#pragma once

namespace hiveObliquePhotography
{
	namespace PointCloudRetouch
	{
		class CNeighborhood;
		class CPointLabelSet;

		class INeighborhoodBuilder : public hiveDesignPattern::IProduct
		{
		public:
			INeighborhoodBuilder() = default;
			virtual ~INeighborhoodBuilder();

			virtual bool onProductCreatedV(const hiveConfig::CHiveConfig* vConfig, PointCloud_t::Ptr vPointCloudScene, const CPointLabelSet* vPointLabelSet, const std::vector<PointCloud_t::Ptr>& vTileSet);

			std::vector<pcl::index_t> buildNeighborhood(pcl::index_t vSeed, std::string& vType, float vPara) const;
			std::vector<pcl::index_t> buildNeighborhood(pcl::index_t vSeed) const;
			void reset();

		protected:
			std::vector<std::size_t> m_OffsetSet;
			std::vector<PointCloud_t::Ptr> m_TileSet;

			PointCloud_t::Ptr m_pPointCloudScene = nullptr;

		private:
			const CPointLabelSet* m_pPointLabelSet = nullptr;

			virtual void __extraInitV(const hiveConfig::CHiveConfig* vConfig) {}
			virtual std::vector<pcl::index_t> __buildNeighborhoodV(pcl::index_t vSeed, std::string& vType, float vPara) const = 0;
			virtual std::vector<pcl::index_t> __buildNeighborhoodV(pcl::index_t vSeed) const = 0;
		};
	}
}