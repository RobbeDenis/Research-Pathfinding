#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, vector<T_NodeType*>& searchedNodes);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode, vector<T_NodeType*>& searchedNodes)
	{
		vector<T_NodeType*> path;
		vector<NodeRecord> openList;
		vector<NodeRecord> closedList;
		NodeRecord currentRecord;

		// StartRecord
		NodeRecord startNode;
		startNode.pNode = pStartNode;
		startNode.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);
		openList.push_back(startNode);

		while (!openList.empty())
		{
			// Find closest NodeRecord to goal
			bool first = true;
			for (const NodeRecord& nodeRecord : openList)
			{
				if (first)
				{
					currentRecord = nodeRecord;
					first = false;
				}
				else if(nodeRecord < currentRecord)
					currentRecord = nodeRecord;
			}

			// Break if currentNode is the goal
			if (currentRecord.pNode == pGoalNode)
				break;

			for (T_ConnectionType* pConnection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
			{
				// Calculate G-Cost for this connection
				float gCost = currentRecord.costSoFar + pConnection->GetCost();

				// Remove NodeRecord with the same node that has a higher cost else continue
				auto it = std::find_if(closedList.begin(), closedList.end(), [pConnection](const NodeRecord& nodeRecord)
					{
						return pConnection->GetTo() == nodeRecord.pNode->GetIndex();
					}
				);

				if (it != closedList.end())
				{
					if (gCost > it->costSoFar)
						continue;

					closedList.erase(std::remove(closedList.begin(), closedList.end(), *it));
				}

				// Remove NodeRecord with the same node that has a higher cost else continue
				it = std::find_if(openList.begin(), openList.end(), [pConnection](const NodeRecord& nodeRecord)
					{
						return pConnection->GetTo() == nodeRecord.pNode->GetIndex();
					}
				);

				if (it != openList.end())
				{
					if (gCost > it->costSoFar)
						continue;

					openList.erase(std::remove(openList.begin(), openList.end(), *it));
				}

				// Add new NodeRecord to openList
				NodeRecord newNodeRecord;
				T_NodeType* pNode = m_pGraph->GetNode(pConnection->GetTo());

				newNodeRecord.costSoFar = gCost;
				newNodeRecord.estimatedTotalCost = gCost + GetHeuristicCost(pNode, pGoalNode);
				newNodeRecord.pConnection = pConnection;
				newNodeRecord.pNode = pNode;

				openList.push_back(newNodeRecord);
				searchedNodes.push_back(newNodeRecord.pNode);
			}

			// Add the NodeRecord from openList to closedList
			closedList.push_back(currentRecord);
			openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));
		}

		// Put all nodes in reverse order in the path
		while (currentRecord.pNode != pStartNode)
		{
			T_NodeType* pNode = currentRecord.pNode;
			path.push_back(pNode);
			NodeRecord prevRecord = currentRecord;
			
			for (const NodeRecord& nodeRecord : closedList)
			{
				if (nodeRecord.pNode->GetIndex() == currentRecord.pConnection->GetFrom())
				{
					currentRecord = nodeRecord;
					break;
				}
			}
			if (prevRecord == currentRecord)
				break;
		} 
		path.push_back(pStartNode);

		// Reverse the path
		std::reverse(path.begin(), path.end());

		return path;
	}
}