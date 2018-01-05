#pragma once

#include <BWAPI/Unit.h>
#include <netkit/network/network.h>

class NeuralDrivenAgent
{
public:
	NeuralDrivenAgent(BWAPI::Unit unit, netkit::network&& org, bool allow_stimpack_behaviour = false);

	void update();

public:
	BWAPI::Unit body; // the agent's body

protected:
	netkit::network m_net;
	bool m_allow_stimpack_behaviour;
};
