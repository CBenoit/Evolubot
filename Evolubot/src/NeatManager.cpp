#include <BWAPI.h>

//#include <netkit/neat/neat_primitive_types.h>

#include "NeatManager.h"
#include "config.h"

// Quick and simple method to check if a file "exists" (not "exactly" that though).
// Good enough for our purpose.
bool file_exist(const char *filename) {
	std::ifstream f(filename);
	return f.good();
}

NeatManager::NeatManager()
	: m_use_best_genomes()
	, m_frame(0)
	, m_neat(nullptr)
	, m_agents() {
	// init NEAT
	netkit::parameters params;
	params.number_of_inputs = 5;
	params.number_of_outputs = 4;
	params.initial_population_size = 22;
	params.compatibility_threshold = 2;
	params.dynamic_compatibility_threshold = true;
	params.target_number_of_species = 3;
	params.keep_same_representant_for_species = true;
	params.use_best_genomes_library = true;
	params.bad_genome_max_fitness = 6;
	params.mutation_probs[netkit::ADD_LINK] = 0.02;
	params.mutation_probs[netkit::ADD_NEURON] = 0.00;
	params.mutation_probs[netkit::REMOVE_NEURON] = 0.00;
	params.mutation_probs[netkit::ADD_CASCADE] = 0.03;
	params.mutation_probs[netkit::REMOVE_GENE] = 0.01;
	m_neat = std::make_unique<netkit::neat>(params);
}

void NeatManager::init(bool use_best_genomes) {
	m_use_best_genomes = use_best_genomes;
	if (file_exist(LAST_NEAT_STATE_FILENAME)) {
		netkit::deserializer des(LAST_NEAT_STATE_FILENAME);
		des >> *m_neat;
		des.close();
		if (!m_use_best_genomes) {
			#ifdef DEBUG_OUTPUT_GENERIC
			BWAPI::Broodwar << "Last generation stats:" << std::endl;
			for (netkit::species& spec : m_neat->get_all_species()) {
				spec.update_stats();
				BWAPI::Broodwar << "<species: id = " << spec.get_id() << ", age = " << spec.get_age()
					<< ", aoli = " << spec.get_age_of_last_improvement()
					<< ", nb members = " << spec.number_of_members()
					<< "\n\tavg fitness = " << spec.get_avg_fitness()
					<< ", best fitness = " << spec.get_best_fitness()
					<< ", best fitness ever = " << spec.get_best_fitness_ever() << ">" << std::endl;
			}
			#endif

			m_neat->epoch(); // go to the next generation
		}
	} else { // from scratch
		m_neat->init();
	}
}

void NeatManager::save() {
	netkit::serializer ser(LAST_NEAT_STATE_FILENAME);
	ser << *m_neat;
	ser.close();
}

void NeatManager::rate_agents() {
	if (!m_use_best_genomes) { // useless if not learning.
		for (auto& u : m_agents) {
			// evaluate the organism's performance.
			u.org->set_fitness(static_cast<double>(u.damages_dealt * ATTACK_PERF_WEIGHT // attack performance.
				+ u.agent.body->getHitPoints() * SURVIVAL_PERF_WEIGHT) // survival performance.
				+ m_agents.size() * COOPERATIVE_PERF_WEIGHT); // cooperative performance.
		}
	}
}

void NeatManager::update_units() {
	++m_frame;

	if (!m_use_best_genomes) {
		// if learning only.
		for (auto& u : m_agents) {
			if (u.agent.body->getGroundWeaponCooldown() > u.cooldown_last_frame) {
				u.damages_dealt += u.agent.body->getType().groundWeapon().damageAmount();
			}

			u.cooldown_last_frame = u.agent.body->getGroundWeaponCooldown();
		}

		#ifdef DEBUG_SHOW_FITNESS
		rate_agents();

		for (auto& u : m_agents) {
			BWAPI::Broodwar->drawTextMap(u.agent.body->getPosition() - BWAPI::Position(20, 10),
				"%c%d %0.1f", BWAPI::Text::White,
				u.damages_dealt,
				u.org->get_fitness()
			);
		}
		#endif
	}

	for (size_t i = m_frame % FRAMES_PER_UPDATE; i < m_agents.size(); i += FRAMES_PER_UPDATE) {
		m_agents[i].agent.update();
	}
}

void NeatManager::register_unit(BWAPI::Unit unit) {
	if (m_use_best_genomes) {
		// Get genomes from the best genome library.
		auto geno = m_neat->get_random_genome_from_best_genome_library();
		if (geno.has_value()) {
			m_agents.emplace_back(
				NeuralDrivenAgent(unit, geno->generate_network()),
				std::optional<netkit::organism>(),
				false,
				0
			);
		}
	} else {
		// Generate organisms from the population.
		netkit::organism org = m_neat->generate_and_get_next_organism();
		m_agents.emplace_back(
			NeuralDrivenAgent(unit, org.get_genome().generate_network()),
			std::optional<netkit::organism>(std::move(org)),
			false,
			0
		);
	}
}

void NeatManager::unregister_unit(BWAPI::Unit unit) {
	if (m_use_best_genomes) {
		m_agents.erase(std::remove_if(m_agents.begin(), m_agents.end(), [&unit](auto& na) {
			return na.agent.body->getID() == unit->getID();
		}), m_agents.end());
	} else {
		// rate the agent being removed if learning.
		m_agents.erase(std::remove_if(m_agents.begin(), m_agents.end(), [&unit](auto& na) {
			if (na.agent.body->getID() == unit->getID()) {
				na.org->set_fitness(static_cast<double>(na.damages_dealt * ATTACK_PERF_WEIGHT));
				return true;
			}
			else {
				return false;
			}
		}), m_agents.end());
	}
}
