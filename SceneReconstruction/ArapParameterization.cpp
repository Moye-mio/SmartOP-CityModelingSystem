﻿#include "pch.h"
#include "ArapParameterization.h"

using namespace hiveObliquePhotography::SceneReconstruction;

_REGISTER_NORMAL_PRODUCT(CArapParameterization, KEYWORD::ARAP_MESH_PARAMETERIZATION)

using namespace hiveObliquePhotography::SceneReconstruction;

//*****************************************************************
//FUNCTION: 
Eigen::MatrixXd CArapParameterization::execute()
{
	auto BoundaryStatus = findBoundaryPoint();
	auto InitialUV = calcInitialUV(m_Mesh, BoundaryStatus);
	
	return InitialUV;
}

//*****************************************************************
//FUNCTION: 
void CArapParameterization::buildHalfEdge()
{
	m_VertexInfoTable.resize(m_Mesh.m_Vertices.size());
	std::vector<bool> Flag(m_Mesh.m_Vertices.size(), false);
	for(auto& Face:m_Mesh.m_Faces)
	{
		for(int i = 0; i < 3; i++)
		{
			SHalfEdge HalfEdge;
			HalfEdge.VertexRef = Face[i];
			auto Index = m_HalfEdgeTable.size();
			m_VertexInfoTable[Face[i]].push_back(Index);
			HalfEdge.Prev = Index + ((i == 0) ? (2) : (-1));
			HalfEdge.Next = Index + ((i == 2) ? (-2) : (1));
			if(Flag[Face[i]] && Flag[Face[(i + 1) % 3]])
			{
				HalfEdge.Twin = __findTwinRef(Face[i], Face[(i + 1) % 3]);
				if( HalfEdge.Twin >= 0 )
				    m_HalfEdgeTable[HalfEdge.Twin].Twin = Index;
			}
			m_HalfEdgeTable.push_back(HalfEdge);
		}
		Flag[Face[0]] = true;
		Flag[Face[1]] = true;
		Flag[Face[2]] = true;
	}
}

//*****************************************************************
//FUNCTION: 
std::vector<bool> CArapParameterization::findBoundaryPoint()
{
	std::vector<bool> OutPutSet(m_Mesh.m_Vertices.size(), false);
	for(auto& HalfEdge:m_HalfEdgeTable)
	{
		if(HalfEdge.Twin < 0)
		{
			OutPutSet[HalfEdge.VertexRef] = true;
			OutPutSet[m_HalfEdgeTable[HalfEdge.Next].VertexRef] = true;
		}
	}
	
	return OutPutSet;
}

//*****************************************************************
//FUNCTION: 
Eigen::MatrixXd CArapParameterization::calcInitialUV(const CMesh& vMesh, const std::vector<bool>& vBoundaryStatus)
{
	auto TutteMatrix = __calcTutteSolveMatrix(vMesh, vBoundaryStatus);
	Eigen::VectorXd VectorX, VectorY;
	__fillTutteSolveVectors(VectorX, VectorY, vMesh, vBoundaryStatus);
	auto X = __solveSparseMatrix(TutteMatrix, VectorX);
	auto Y = __solveSparseMatrix(TutteMatrix, VectorY);
	
	return __switch2UVMatrix(vMesh, X, Y);
}

//*****************************************************************
//FUNCTION: 
Eigen::SparseMatrix<double, Eigen::ColMajor, int> CArapParameterization::__buildTutteSolveMatrix(const std::vector<SHalfEdge>& vHalfEdgeSet)
{
	auto NumVertices = vHalfEdgeSet.size();
	Eigen::SparseMatrix<double, Eigen::ColMajor, int> TutteMatrix(NumVertices, NumVertices);
	for (size_t VertexId = 0; VertexId < NumVertices; ++VertexId)
	{
		if (vHalfEdgeSet[VertexId].Twin < 0) //boundary
			TutteMatrix.insert(VertexId, VertexId) = 1.0;
		else //interior
		{
			const auto& NeighborHalfEdgeSet = m_VertexInfoTable[VertexId];
			
			TutteMatrix.insert(VertexId, VertexId) = -NeighborHalfEdgeSet.size();
			for (auto i : NeighborHalfEdgeSet)
				if (vHalfEdgeSet[i].Twin >= 0)
					TutteMatrix.insert(VertexId, vHalfEdgeSet[i].Next) = 1.0;
		}
	}

	return TutteMatrix;
}

//*****************************************************************
//FUNCTION: 
void CArapParameterization::__fillTutteSolveVectors(Eigen::VectorXd& vVectorX, Eigen::VectorXd& vVectorY, const CMesh& vMesh, const std::vector<bool>& vBoundaryStatus)
{
	auto NumVertices = vMesh.m_Vertices.size();
	vVectorX.resize(NumVertices);
	vVectorY.resize(NumVertices);
	std::pair<int, int> XYAxis;
	int HeightAxis;
	vMesh.calcModelPlaneAxis(XYAxis, HeightAxis);
	for (int VertexId = 0; VertexId < NumVertices; VertexId++)
	{
		if (vBoundaryStatus[VertexId])
		{
			vVectorX(VertexId) = vMesh.m_Vertices[VertexId][XYAxis.first];
			vVectorY(VertexId) = vMesh.m_Vertices[VertexId][XYAxis.second];
		}
		else
		{
			vVectorX(VertexId) = 0.0;
			vVectorY(VertexId) = 0.0;
		}	
	}
}

//*****************************************************************
//FUNCTION: 
Eigen::VectorXd CArapParameterization::__solveSparseMatrix(const Eigen::SparseMatrix<double, Eigen::ColMajor, int>& vMatrix, const Eigen::VectorXd& vVector)
{
	auto CompressMatrix = vMatrix;
	CompressMatrix.makeCompressed();

	Eigen::SimplicialLDLT<Eigen::SparseMatrix<double, Eigen::ColMajor>> Solver;
	Solver.analyzePattern(CompressMatrix);
	Solver.factorize(CompressMatrix);
	_ASSERTE(Solver.info() == Eigen::Success);
	auto Solution = Solver.solve(vVector);
	_ASSERTE(Solver.info() == Eigen::Success);
	return Solution;
}

//*****************************************************************
//FUNCTION: 
Eigen::MatrixXd CArapParameterization::__switch2UVMatrix(const CMesh& vMesh, const Eigen::VectorXd& vX, const Eigen::VectorXd& vY)
{
	_ASSERTE(vX.size() == vMesh.m_Vertices.size() && vX.size() == vY.size());
	Eigen::MatrixXd UVMatrix(vMesh.m_Vertices.size(), 2);
	auto BoundingBox = vMesh.calcAABB();
	std::pair<int, int> XYAxis;
	int HeightAxis;
	vMesh.calcModelPlaneAxis(XYAxis, HeightAxis);
	float WidthU = BoundingBox.second.data()[XYAxis.first] - BoundingBox.first.data()[XYAxis.first];
	float HeightV = BoundingBox.second.data()[XYAxis.second] - BoundingBox.first.data()[XYAxis.second];
	float BeginX = BoundingBox.first.data()[XYAxis.first], BeginY = BoundingBox.first.data()[XYAxis.second];

	for (int VertexId = 0; VertexId < vX.size(); VertexId++)
	{
		float U = (vX(VertexId) - BeginX) / WidthU;
		float V = (vY(VertexId) - BeginY) / HeightV;

		UVMatrix.row(VertexId) = Eigen::Vector2d(U, V);
	}

	return UVMatrix;
}
//*****************************************************************
//FUNCTION: 
int CArapParameterization::__findTwinRef(int vStartIndex, int vEndIndex)
{
	for(auto EdgeIndex:m_VertexInfoTable[vEndIndex])
		if (m_HalfEdgeTable[m_HalfEdgeTable[EdgeIndex].Next].VertexRef == vStartIndex)
			return EdgeIndex;
	return -1;
}

//*****************************************************************
//FUNCTION: 
Eigen::MatrixXd CArapParameterization::__solveARAP(const Eigen::MatrixXd& vVertexPos, const Eigen::MatrixXi& vFaces, Eigen::MatrixXd& vInitialUV)
{
	return {};
}