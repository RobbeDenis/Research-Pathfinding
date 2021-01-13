#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class Dijkstra
	{
	public:
		Dijkstra(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnectionToPrevNode = nullptr;
			float costSoFar = FLT_MAX;

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode;
			};

			bool operator<(const NodeRecord& other) const
			{
				return costSoFar < other.costSoFar;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, vector<T_NodeType*>& searchedNodes);
	private:
		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template <class T_NodeType, class T_ConnectionType>
	Dijkstra<T_NodeType, T_ConnectionType>::Dijkstra(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> Dijkstra<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, vector<T_NodeType*>& searchedNodes)
	{
		vector<T_NodeType*> path;
		vector<NodeRecord> openList;
		vector<NodeRecord> visited;
		NodeRecord currentNode;

		// set pStartNode as currentNode
		currentNode.costSoFar = 0.f;
		currentNode.pNode = pStartNode;
		openList.push_back(currentNode);

		while (!openList.empty())
		{
			// loop over all the connections from the currentNode
			for (T_ConnectionType* pConnection : m_pGraph->GetNodeConnections(currentNode.pNode->GetIndex()))
			{
				// setup nextNode
				NodeRecord nextNode;
				nextNode.costSoFar = pConnection->GetCost() + currentNode.costSoFar;
				nextNode.pNode = m_pGraph->GetNode(pConnection->GetTo());
				nextNode.pConnectionToPrevNode = pConnection;

				// check if this node has already been visited, if so skip
				auto visitedNodeIt = std::find(begin(visited), end(visited), nextNode);
				if (visitedNodeIt != end(visited))
					continue;

				// check if the costSoFar is smaller then the one that exists, if so replace the best connection 
				auto nextNodeIt = std::find(begin(openList), end(openList), nextNode);
				if (nextNodeIt != end(openList))
				{
					if (nextNodeIt->costSoFar > nextNode.costSoFar)
					{
						nextNodeIt->costSoFar = nextNode.costSoFar;
						nextNodeIt->pConnectionToPrevNode = pConnection;
					}
				}
				else
				{
					// push the node on the openList if it didn't exist yet
					openList.push_back(nextNode);
					searchedNodes.push_back(nextNode.pNode);
				}
			}

			// add currentNode to visited and remove from the openList
			visited.push_back(currentNode);
			openList.erase(std::remove(begin(openList), end(openList), currentNode));

			// look for the new currenNode (smallest cost in openList)
			auto smallestNodeIt = std::min_element(begin(openList), end(openList));
			if (smallestNodeIt == end(openList))
				return path;
			currentNode = *smallestNodeIt;

			// check if the destination has been found
			if (currentNode.pNode == pDestinationNode)
			{
				// track back until you find the startNode and fill in the path
				while (currentNode.pNode != pStartNode)
				{
					path.push_back(currentNode.pNode);
					int fromNodeIdx = currentNode.pConnectionToPrevNode->GetFrom();
					currentNode = *std::find_if(begin(visited), end(visited), [fromNodeIdx](NodeRecord& other)
						{
							return fromNodeIdx == other.pNode->GetIndex();
						});
				}
				path.push_back(pStartNode);
				break;
			}
		}
		// reverse the path because the path is in the wrong order
		std::reverse(begin(path), end(path));
		return path;
	}
}
