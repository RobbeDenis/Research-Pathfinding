#include "stdafx.h"
#include "TheFlock.h"

#include "../SteeringAgent.h"
#include "../SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"
#include "SpacePartitioning.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 25 }
	, m_NrOfNeighbors{0}
	, m_ResetCells{ false }
{
	m_pToggleSpaceParUI = new bool(false);
	m_pTogglePartitioning = new bool(true);
	m_Neighbors.reserve(m_FlockSize);
	CreateAgents();
	CreateCellSpace();
}

Flock::~Flock()
{
	SAFE_DELETE(m_pCohesionBehaviour);
	SAFE_DELETE(m_pSeparationBehaviour);
	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pAlignmentBehaviour);
	SAFE_DELETE(m_pSeekBehaviour);
	SAFE_DELETE(m_pWanderBehaviour);
	SAFE_DELETE(m_pCellSpace);
	SAFE_DELETE(m_pToggleSpaceParUI);
	SAFE_DELETE(m_pTogglePartitioning);

	for (SteeringAgent* agent : m_Agents)
	{
		SAFE_DELETE(agent);
	}
}

void Flock::Update(float deltaT)
{
	// loop over all the boids
	// register its neighbors
	// update it
	// trim it to the world
	m_pEvadeBehaviour->SetTarget(m_pAgentToEvade->GetPosition());

	if (*m_pTogglePartitioning)
	{
		for (SteeringAgent* agent : m_Agents)
		{
			agent->TrimToWorld(m_WorldSize);
			m_pCellSpace->UpdateAgentCell(agent, agent->GetOldPosition());
			m_pCellSpace->RegisterNeighbors(agent->GetPosition(), m_NeighborhoodRadius, deltaT);
			m_Neighbors = m_pCellSpace->GetNeighbors();
			m_NrOfNeighbors = m_pCellSpace->GetNrOfNeighbors();
			agent->Update(deltaT);
		}
		m_ResetCells = true;
	}
	else
	{
		for (SteeringAgent* agent : m_Agents)
		{
			agent->TrimToWorld(m_WorldSize);
			RegisterNeighbors(agent);
			agent->Update(deltaT);
		}
		if (m_ResetCells)
		{
			m_pCellSpace->ResetCellAgents();
			m_ResetCells = false;
		}
	}

}

void Flock::Render(float deltaT)
{
	if (*m_pToggleSpaceParUI && *m_pTogglePartitioning)
	{
		m_pCellSpace->RenderCells(m_Agents[0], m_NeighborhoodRadius);
		m_Agents[0]->SetBodyColor({ 0,0,1 });
		m_Agents[0]->Render(deltaT);
	}
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("PARTITIONING");
	ImGui::Spacing();

	ImGui::Checkbox("Enable Partitioning", m_pTogglePartitioning);
	ImGui::Spacing();
	ImGui::Checkbox("Enable Debug", m_pToggleSpaceParUI);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("FLOCK BEHAVIOUR");
	ImGui::Spacing();

	ImGui::SliderFloat("Cohesion", GetWeight(m_pCohesionBehaviour), 0.f, 0.1f, "%.2");
	ImGui::SliderFloat("Separation", GetWeight(m_pSeparationBehaviour), 0.f, 0.1f, "%.2");
	ImGui::SliderFloat("Alignment", GetWeight(m_pAlignmentBehaviour), 0.f, 0.1f, "%.2");
	ImGui::SliderFloat("Seek", GetWeight(m_pSeekBehaviour), 0.f, 0.1f, "%.2");
	ImGui::SliderFloat("Wander", GetWeight(m_pWanderBehaviour), 0.f, 0.1f, "%.2");

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Implement checkboxes and sliders here

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// register the agents neighboring the currently evaluated agent
	// store how many they are, so you know which part of the vector to loop over
	m_NrOfNeighbors = 0;
	for (SteeringAgent* pOther : m_Agents)
	{
		if (pOther != pAgent)
		{
			if (Elite::Distance(pAgent->GetPosition(), pOther->GetPosition()) <= m_NeighborhoodRadius)
			{
				if (m_Neighbors.size() <= m_NrOfNeighbors)
				{
					m_Neighbors.push_back(pOther);
				}
				else
				{
					m_Neighbors[m_NrOfNeighbors] = nullptr;
					delete m_Neighbors[m_NrOfNeighbors];
					m_Neighbors[m_NrOfNeighbors] = pOther;
				}
				m_NrOfNeighbors++;
			}
		}
	}
}

void Flock::CreateAgents()
{
	// Create Flock 
	m_pCohesionBehaviour = new Cohesion{ *this };
	m_pSeparationBehaviour = new Separation{ *this };
	m_pAlignmentBehaviour = new Alignment{ *this };
	m_pSeekBehaviour = new Seek{ };
	m_pWanderBehaviour = new Wander{ };
	m_pBlendedSteering = new BlendedSteering({
		{m_pCohesionBehaviour, 0.03},
		{m_pSeparationBehaviour, 0.05},
		{m_pAlignmentBehaviour, 0.06},
		{m_pSeekBehaviour, 0.0},
		{m_pWanderBehaviour, 0.09}
	});

	m_pEvadeBehaviour = new Evade{};
	m_pPrioritySteering = new PrioritySteering{ {m_pEvadeBehaviour, m_pBlendedSteering} };
	m_pEvadeBehaviour->SetEvadeRadius(30);
	

	m_pWanderBehaviour->SetWanderOffset(1);
	const float spawnOffset = { m_WorldSize };

	for (int i = 0; i < m_FlockSize; i++)
	{
		m_Agents.push_back(new SteeringAgent{});
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[i]->SetMaxLinearSpeed(1000);
		m_Agents[i]->SetAutoOrient(true);
		m_Agents[i]->SetMass(1.f);
		m_Agents[i]->SetBodyColor({ 1, 1, 0 });
		m_Agents[i]->SetPosition({ Elite::randomFloat(-spawnOffset, spawnOffset), Elite::randomFloat(-spawnOffset, spawnOffset) });
	}
}

void Flock::SetSeekTarget(const Elite::Vector2& pos)
{
	m_pSeekBehaviour->SetTarget(pos);
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	Elite::Vector2 average{};

	for (int i = 0; i < m_NrOfNeighbors; i++)
	{
		average += m_Neighbors[i]->GetPosition();
	}
	if (m_NrOfNeighbors != 0)
	{
		average /= m_NrOfNeighbors;
	}

	return average;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	Elite::Vector2 average{};

	for (int i = 0; i < m_NrOfNeighbors; i++)
	{
		average += m_Neighbors[i]->GetLinearVelocity();
	}
	if (m_NrOfNeighbors != 0)
	{
		average /= m_NrOfNeighbors;
	}
	return average;
}


float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->m_WeightedBehaviors;
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

float Flock::GetNeighborHoodRadius() const
{
	return m_NeighborhoodRadius;
}

void Flock::CreateCellSpace()
{
	const int col = 25;
	const int row = 25;
	m_pCellSpace = new CellSpace{ m_WorldSize * 2, m_WorldSize * 2, row, col, m_FlockSize };

	for (SteeringAgent* agent : m_Agents)
	{
		m_pCellSpace->AddAgent(agent);
	}
}
