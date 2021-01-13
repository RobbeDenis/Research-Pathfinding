#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\App_Steering\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
	, m_CellHeight{ height / rows }
	, m_CellWidth{ width / cols }
	, m_OldSpaceSize{ width }
{
	for (int r = 0; r < m_NrOfRows; r++)
	{
		for (int c = 0; c < m_NrOfCols; c++)
		{
			m_Cells.push_back({ c * m_CellWidth, r * m_CellHeight, m_CellWidth, m_CellHeight });
		}
	}
}

CellSpace::~CellSpace()
{

}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	m_Cells[PositionToIndex(agent->GetPosition())].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, const Elite::Vector2& oldPos)
{
	if (agent != nullptr)
	{
		int newIndex = PositionToIndex(agent->GetPosition());
		int oldIndex = PositionToIndex(oldPos);

		if (newIndex != oldIndex)
		{
			auto it = std::find(m_Cells[oldIndex].agents.begin(), m_Cells[oldIndex].agents.end(), agent);
			if (it != m_Cells[oldIndex].agents.end())
			{
				m_Cells[oldIndex].agents.remove(*it);
			}
			m_Cells[newIndex].agents.push_back(agent);
		}
	}
}

void CellSpace::RegisterNeighbors(const Elite::Vector2& pos, float queryRadius, float deltaT)
{
	Elite::Rect rect = { {pos.x - queryRadius, pos.y - queryRadius}, queryRadius * 2, queryRadius * 2 };
	for (const Cell& cell : m_Cells)
	{
		if (Elite::IsOverlapping(cell.boundingBox, rect))
		{
			for (SteeringAgent* agent : cell.agents)
			{
				if (Elite::Distance(pos, agent->GetPosition()) <= queryRadius)
				{
					m_Neighbors[m_NrOfNeighbors] = agent;
					m_NrOfNeighbors++;
				}
			}
		}
	}
}

void CellSpace::RenderCells(SteeringAgent* pAgent, float queryRadius) const
{
	for (int i = 1; i < m_NrOfCols; i++)
	{
		DEBUGRENDERER2D->DrawSegment({ m_CellWidth * i - m_SpaceWidth / 2, m_SpaceHeight / 2 }, { m_CellWidth * i - m_SpaceWidth / 2, -m_SpaceHeight / 2 }, { 1,0,0 }, 0.2);
		DEBUGRENDERER2D->DrawSegment({m_SpaceWidth / 2, m_CellHeight * i - m_SpaceHeight / 2 }, { -m_SpaceWidth / 2, m_CellHeight * i - m_SpaceHeight / 2 }, { 1,0,0 }, 0.2);
	}

	Elite::Rect rect = { {pAgent->GetPosition().x - queryRadius + m_SpaceWidth / 2, pAgent->GetPosition().y - queryRadius + m_SpaceHeight / 2}, queryRadius * 2, queryRadius * 2 };

	DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), queryRadius, { 1,1,1 }, 0.05);

	for (const Cell& cell : m_Cells)
	{
		if (Elite::IsOverlapping(cell.boundingBox, rect))
		{
			DEBUGRENDERER2D->DrawSegment({cell.boundingBox.bottomLeft.x - m_SpaceWidth / 2, cell.boundingBox.bottomLeft.y - m_SpaceHeight / 2 },
				{ cell.boundingBox.bottomLeft.x + m_CellWidth - m_SpaceWidth / 2, cell.boundingBox.bottomLeft.y - m_SpaceHeight / 2 }, { 1,1,0 }, 0.01);
			DEBUGRENDERER2D->DrawSegment({ cell.boundingBox.bottomLeft.x - m_SpaceWidth / 2, cell.boundingBox.bottomLeft.y - m_SpaceHeight / 2 + m_CellHeight },
				{ cell.boundingBox.bottomLeft.x + m_CellWidth - m_SpaceWidth / 2, cell.boundingBox.bottomLeft.y - m_SpaceHeight / 2  + m_CellHeight }, { 1,1,0 }, 0.01);
			DEBUGRENDERER2D->DrawSegment({ cell.boundingBox.bottomLeft.x - m_SpaceWidth / 2, cell.boundingBox.bottomLeft.y - m_SpaceHeight / 2 },
				{ cell.boundingBox.bottomLeft.x - m_SpaceWidth / 2, cell.boundingBox.bottomLeft.y - m_SpaceHeight / 2 + m_CellHeight }, { 1,1,0 }, 0.01);
			DEBUGRENDERER2D->DrawSegment({ cell.boundingBox.bottomLeft.x - m_SpaceWidth / 2 + m_CellWidth, cell.boundingBox.bottomLeft.y - m_SpaceHeight / 2 },
				{ cell.boundingBox.bottomLeft.x - m_SpaceWidth / 2 + m_CellWidth, cell.boundingBox.bottomLeft.y - m_SpaceHeight / 2 + m_CellHeight }, { 1,1,0 }, 0.01);
		}
		int size = cell.agents.size();
		DEBUGRENDERER2D->DrawString({ cell.boundingBox.bottomLeft.x - m_SpaceWidth / 2 + 1, cell.boundingBox.bottomLeft.y - m_SpaceHeight / 2 + 7 },
			&std::to_string(size)[0]);
	}
}

void CellSpace::ResetCellAgents()
{
	for (Cell& cell : m_Cells)
	{
		cell.agents.clear();
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	float y = pos.y;
	float x = pos.x;

	if (y >= m_SpaceHeight / 2 - 1)
	{
		y = m_SpaceHeight / 2 - 1;
	}
	if (x >= m_SpaceWidth / 2 - 1)
	{
		x = m_SpaceWidth / 2 - 1;
	}

	int c = (x + m_SpaceWidth / 2) / m_CellWidth;
	int r = (y + m_SpaceHeight / 2) / m_CellHeight;
	return (r * m_NrOfCols) + c;
}
