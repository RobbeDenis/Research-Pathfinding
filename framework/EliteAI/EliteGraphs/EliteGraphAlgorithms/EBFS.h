#pragma once

namespace Elite 
{
	template <class T_NodeType, class T_ConnectionType>
	class BFS
	{
	public:
		BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, vector<T_NodeType*>& searchedNodes);
	private:
		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template <class T_NodeType, class T_ConnectionType>
	BFS<T_NodeType, T_ConnectionType>::BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> BFS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, vector<T_NodeType*>& searchedNodes)
	{
		// Here we calculate our path using BFS
		// if start is equal to end
		vector<T_NodeType*> path;
		if (pStartNode == pDestinationNode)
		{
			path.push_back(pDestinationNode);
			return path;
		}

		std::queue<T_NodeType*> openList;
		std::map<T_NodeType*, T_NodeType*> closedList;

		openList.push(pStartNode);
		T_NodeType* pCurrentNode = nullptr;

		while (!openList.empty())
		{
			pCurrentNode = openList.front();
			openList.pop();

			if (pCurrentNode == pDestinationNode)
				break;

			for (auto connection : m_pGraph->GetNodeConnections(pCurrentNode->GetIndex()))
			{
				T_NodeType* pNextNode = m_pGraph->GetNode(connection->GetTo());
				if (closedList.find(pNextNode) == closedList.end())
				{
					openList.push(pNextNode);
					closedList[pNextNode] = pCurrentNode;
					searchedNodes.push_back(pNextNode);
				}
			}
		}

		// Track back
		// If end wasnt found
		if (pCurrentNode != pDestinationNode)
		{
			path.push_back(pStartNode);
			return path;
		}

		while (pCurrentNode != pStartNode)
		{
			path.push_back(pCurrentNode);
			pCurrentNode = closedList[pCurrentNode];
		}

		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end());

		return path;
	}
}

