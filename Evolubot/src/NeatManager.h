#pragma once

#include <memory>
#include <optional>
#include <tuple>

#include <BWAPI/Unit.h>
#include <netkit/neat/neat.h>

#include "NeuralDrivenAgent.h"

namespace priv {
	// for local use only.
	struct NeatAgent {
		NeatAgent(NeuralDrivenAgent agent_, std::optional<netkit::organism> org_,
				int cooldown_last_frame_, int number_of_attacks_)
			: agent(agent_)
			, org(org_)
			, cooldown_last_frame(cooldown_last_frame_)
			, damages_dealt(number_of_attacks_) {}

		NeuralDrivenAgent agent;
		std::optional<netkit::organism> org;
		int cooldown_last_frame;
		int damages_dealt;
	};
}

class NeatManager {
public:
	NeatManager();

	// If use_best_units is set to true, no new epoch is processed
	// when continuing from a previous NEAT state and only the genomes
	// from the best genomes library are used.
	void init(bool use_best_genomes = false);

	// Save the NEAT state.
	void save();

	// Rate each registered agents.
	void rate_agents();

	// Needs to be called at each frame.
	void update_units();

	void register_unit(BWAPI::Unit unit);
	void unregister_unit(BWAPI::Unit unit); // rate the agent being unregistered if not using best genomes.

	netkit::neat& get_neat() { return *m_neat; }

	unsigned int number_of_agents() { return m_agents.size(); }

private:
	bool m_use_best_genomes;
	unsigned int m_frame;

	std::unique_ptr<netkit::neat> m_neat;
	std::vector<priv::NeatAgent> m_agents;
};
