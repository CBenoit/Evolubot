#pragma once

#include <BWAPI/Unit.h>
#include <netkit/network/network.h>

class NeuralDrivenAgent
{
public:
	NeuralDrivenAgent(BWAPI::Unit unit, netkit::network&& org);

	void update();

public:
	BWAPI::Unit body; // the agent's body

private:
	netkit::network m_net;
};
