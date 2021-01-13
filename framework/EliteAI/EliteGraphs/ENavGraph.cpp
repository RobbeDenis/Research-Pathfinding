#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	int index = 0;
	for (const Elite::Line* line : m_pNavMeshPolygon->GetLines())
	{
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(line->index).size() > 1)
		{
			Elite::Vector2 pos = (line->p1 + line->p2) / 2;
			AddNode(new NavGraphNode(index, line->index, pos));
			index++;
		}
	}

	//2. Create connections now that every node is created
	for (const Elite::Triangle* triangle : m_pNavMeshPolygon->GetTriangles())
	{
		int amount = 0;
		NavGraphNode* node0 = nullptr;
		NavGraphNode* node1 = nullptr;
		NavGraphNode* node2 = nullptr;

		for (int lineIndex : triangle->metaData.IndexLines)
		{
			auto it = std::find_if(GetAllNodes().begin(), GetAllNodes().end(), [lineIndex](NavGraphNode* node)
				{
					return node->GetLineIndex() == lineIndex;
				}
			);

			if (it != GetAllNodes().end())
			{
				amount++;
				if (amount == 1)
				{
					node0 = *it;
				}
				else if (amount == 2)
				{
					node1 = *it;
				}
				else if (amount == 3)
				{
					node2 = *it;
				}
			}

			if (amount == 2)
			{
				AddConnection(new GraphConnection2D(node0->GetIndex(), node1->GetIndex(), Elite::Distance(node0->GetPosition(), node1->GetPosition())));
			}
			else if (amount == 3)
			{
				AddConnection(new GraphConnection2D(node1->GetIndex(), node2->GetIndex(), Elite::Distance(node1->GetPosition(), node2->GetPosition())));
				AddConnection(new GraphConnection2D(node2->GetIndex(), node0->GetIndex(), Elite::Distance(node2->GetPosition(), node0->GetPosition())));
			}
		}
	}
}

