#pragma once

#include <memory>
#include <optional>
#include <tuple>

#include <BWAPI/Unit.h>
#include <netkit/neat/neat.h>
#include <netkit/neat/novelbank.h>

#include "config.h"
#include "NeuralDrivenAgent.h"
#include "NovelPos.h"

namespace priv {
	// for local use only.
	struct NeatAgent {
		NeatAgent(NeuralDrivenAgent agent_, std::optional<netkit::organism> org_,
				int cooldown_last_frame_, int damages_dealt_)
			: agent(agent_)
			, org(org_)
			, cooldown_last_frame(cooldown_last_frame_)
			, damages_dealt(damages_dealt_) {}

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
	//
	// Enable unified mode to use only one genome per initialization.
	// The way of rating the organism will differ.
	//
	// allow_stimpack_behaviour is whether the neural driven allow agent
	// to develop stimpack usage ability or not.
	// /!\ It modifiy genomes by adding one input and one output. Therefore, it's not
	// possible to use saved data from experiments without stimpacks behaviour.
	void init(unsigned int population_size,
		bool use_best_genomes = false, bool force_initial_start = false,
		bool allow_stimpack_behaviour = false, bool unified_mode = false,
		bool cascade_neat = false);

	// Save the NEAT state.
	void save();

	// Rate each registered agents.
	void rate();

	#ifdef NOVELTY_SEARCH
	// Update population's fitness with novelty.
	void update_novelty();
	#endif

	// Needs to be called at each frame.
	void update_units();

	// Call to register the unit and let a neural network control it.
	void register_unit(BWAPI::Unit unit);

	// Rate the agent being unregistered if not using best genomes or unified mode.
	void unregister_unit(BWAPI::Unit unit);

	netkit::neat& get_neat() { return *m_neat; }

	unsigned int number_of_agents() { return m_agents.size(); }

	bool last_organism_used() { return !m_neat->has_more_organisms_to_process(); }

private:
	bool m_unified_mode;
	bool m_use_best_genomes;
	bool m_allow_stimpack_behaviour;
	unsigned int m_frame;
	unsigned int m_total_damages_dealt;

	std::unique_ptr<netkit::organism> m_org;
	std::unique_ptr<netkit::neat> m_neat;
	std::vector<priv::NeatAgent> m_agents;

	#ifdef NOVELTY_SEARCH
	std::vector<netkit::novelgenome<NovelPos>> m_all_ng;
	std::vector<netkit::genome> m_best_genomes;
	netkit::novelbank<NovelPos> m_novelbank;
	#endif
};
