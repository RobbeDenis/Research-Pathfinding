#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;
class CellSpace;

class Flock
{
public:
	Flock(
		int flockSize = 80, 
		float worldSize = 100, 
		SteeringAgent* pAgentToEvade = nullptr, 
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT);
	void UpdateAndRenderUI();
	void Render(float deltaT);

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const { return m_NrOfNeighbors; }
	const vector<SteeringAgent*>& GetNeighbors() const { return m_Neighbors; }
	void SetSeekTarget(const Elite::Vector2& pos);

	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;
	float GetNeighborHoodRadius() const;

private:
	// flock agents
	int m_FlockSize = 0;
	vector<SteeringAgent*> m_Agents;

	// neighborhood agents
	vector<SteeringAgent*> m_Neighbors;
	float m_NeighborhoodRadius = 5.f;
	int m_NrOfNeighbors = 0;

	// evade target
	SteeringAgent* m_pAgentToEvade = nullptr;

	// world info
	bool m_TrimWorld = false;
	float m_WorldSize = 0;
	
	// steering behaviors
	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;
	Cohesion* m_pCohesionBehaviour = nullptr;
	Separation* m_pSeparationBehaviour = nullptr;
	Alignment* m_pAlignmentBehaviour = nullptr;
	Seek* m_pSeekBehaviour = nullptr;
	Wander* m_pWanderBehaviour = nullptr;
	Evade* m_pEvadeBehaviour = nullptr;

	// space partitioning
	CellSpace* m_pCellSpace = nullptr;
	bool* m_pToggleSpaceParUI = nullptr;
	bool* m_pTogglePartitioning = nullptr;
	bool m_ResetCells;

	// private functions
	float* GetWeight(ISteeringBehavior* pBehaviour);
	void CreateAgents();
	void CreateCellSpace();

private:
	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};