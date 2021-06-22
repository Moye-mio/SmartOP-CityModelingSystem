#include "pch.h"
#include "PointCloudPCDSaver.h"

using namespace hiveObliquePhotography;

_REGISTER_EXCLUSIVE_PRODUCT(CPointCloudPCDSaver, PCD_SAVER)

//*****************************************************************
//FUNCTION: 
void CPointCloudPCDSaver::saveDataToFile(const PointCloud_t& vPointCloud, const std::string& vFilePath)
{
	if (pcl::io::savePCDFile<PointCloud_t::PointType>(vFilePath, vPointCloud) == -1)
		_HIVE_OUTPUT_WARNING(_FORMAT_STR1("Fail to write file [%1%] due to inexistent file.", vFilePath));
}
