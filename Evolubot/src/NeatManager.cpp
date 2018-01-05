#include <BWAPI.h>

#include "NeatManager.h"
#include "utils.h"

NeatManager::NeatManager()
	: m_unified_mode()
	, m_use_best_genomes()
	, m_allow_stimpack_behaviour()
	, m_frame(0)
	, m_total_damages_dealt(0)
	, m_org(nullptr)
	, m_neat(nullptr)
	, m_agents()
	#ifdef NOVELTY_SEARCH
	, m_all_ng()
	, m_best_genomes()
	, m_novelbank(999, 15, 10)
	#endif
	{}

void NeatManager::init(unsigned int population_size, bool use_best_genomes, bool force_initial_start, bool allow_stimpack_behaviour, bool unified_mode, bool cascade_neat) {
	#ifdef NOVELTY_SEARCH
	if (unified_mode) {
		throw std::runtime_error("cannot use unified_mode with novelty search.");
	}
	#endif

	m_unified_mode = unified_mode;
	m_allow_stimpack_behaviour = allow_stimpack_behaviour;
	m_use_best_genomes = use_best_genomes;

	// init NEAT
	netkit::parameters params;

	if (m_allow_stimpack_behaviour) {
		params.number_of_inputs = 6;
		params.number_of_outputs = 5;
	} else {
		params.number_of_inputs = 5;
		params.number_of_outputs = 4;
	}

	params.initial_population_size = population_size;
	params.compatibility_threshold = 2;
	params.dynamic_compatibility_threshold = true;
	params.target_number_of_species = 4;
	params.keep_same_representant_for_species = true;

	#ifdef NOVELTY_SEARCH
	params.use_best_genomes_library = false;
	#else
	params.use_best_genomes_library = true;
	if (m_unified_mode) {
		params.bad_genome_max_fitness = 1000;
	} else {
		params.bad_genome_max_fitness = 100;
	}
	#endif

	if (cascade_neat) {
		params.mutation_probs[netkit::ADD_LINK] = 0.00;
		params.mutation_probs[netkit::ADD_NEURON] = 0.00;
		params.mutation_probs[netkit::REMOVE_NEURON] = 0.00;
		params.mutation_probs[netkit::ADD_CASCADE] = 0.03;
		params.mutation_probs[netkit::REMOVE_GENE] = 0.00;
	}

	m_neat = std::make_unique<netkit::neat>(params);

	if (file_exist(MODULE_FOLDER LAST_NEAT_STATE_FILENAME) && !force_initial_start) {
		netkit::deserializer des(MODULE_FOLDER LAST_NEAT_STATE_FILENAME);
		des >> *m_neat;

		#ifdef NOVELTY_SEARCH
		des >> m_novelbank;

		size_t number_of_genomes;
		des.get_next(number_of_genomes);
		for (size_t i = 0; i < number_of_genomes; ++i) {
			netkit::genome g(&*m_neat);
			des >> g;
			m_best_genomes.push_back(std::move(g));
		}
		#endif

		des.close();
		if (!m_use_best_genomes) {
			if (m_unified_mode) {
				if (!m_neat->has_more_organisms_to_process()) {
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
			} else {
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
		}
	} else { // from scratch
		m_neat->init();
	}

	if (m_unified_mode) {
		if (!m_use_best_genomes) {
			m_org = std::make_unique<netkit::organism>(m_neat->generate_and_get_next_organism());
		}
	}
}

void NeatManager::save() {
	netkit::serializer ser(MODULE_FOLDER LAST_NEAT_STATE_FILENAME);
	ser << *m_neat;

	#ifdef NOVELTY_SEARCH
	ser << m_novelbank;

	// the best genomes
	ser.append(m_best_genomes.size());
	ser.new_line();
	for (auto& geno : m_best_genomes) {
		ser << geno;
	}
	#endif

	ser.close();
}

void NeatManager::rate() {
	if (!m_use_best_genomes) { // useless if not learning.
		if (m_unified_mode) {
			m_org->set_fitness(std::pow(static_cast<double>(
				m_total_damages_dealt * UNIFIED_ATTACK_PERF_WEIGHT
				+ m_agents.size() * UNIFIED_COOPERATIVE_PERF_WEIGHT
				), EXPONENT_ON_FITNESS));
		} else {
			for (auto& u : m_agents) {
				// evaluate the organism's performance.
				u.org->set_fitness(std::pow(static_cast<double>(u.damages_dealt * ATTACK_PERF_WEIGHT
					+ u.agent.body->getHitPoints() * SURVIVAL_PERF_WEIGHT
					+ m_agents.size() * COOPERATIVE_PERF_WEIGHT), EXPONENT_ON_FITNESS));
			}
		}
	}
}

#ifdef NOVELTY_SEARCH
// Update population's fitness with novelty.
void NeatManager::update_novelty() {
	if (!m_use_best_genomes) {
		for (auto& u : m_agents) {
			netkit::novelgenome<NovelPos> ng{ u.org->get_genome_id() };
			ng.get_pos().fitness = u.org->get_fitness();
			m_novelbank.pop_register(ng);
			m_all_ng.push_back(std::move(ng));
		}

		for (auto& ng : m_all_ng) {
			netkit::genome& geno = m_neat->pop()->get_genome(ng.get_genome_id());

			// update list of best genomes
			if (std::find(m_best_genomes.begin(), m_best_genomes.end(), geno) == m_best_genomes.end()) {
				// not already in the vector.

				if (m_best_genomes.size() < m_neat->params.best_genomes_library_max_size) {
					m_best_genomes.emplace_back(geno);
				}
				else {
					auto worst = std::min_element(m_best_genomes.begin(), m_best_genomes.end(), [&geno](const netkit::genome & g1,
						const netkit::genome & g2) {
						return g1.get_fitness() < g2.get_fitness();
					});

					if (worst->get_fitness() < geno.get_fitness()) {
						*worst = geno;
					}
				}
			}

			// update fitness by evaluating novelty.
			geno.set_fitness(m_novelbank.evaluate(ng));
		}

		m_novelbank.bank_update();
		m_novelbank.pop_clear();

		m_all_ng.clear();
	}
}
#endif

void NeatManager::update_units() {
	++m_frame;

	if (!m_use_best_genomes) {
		// if learning only.
		if (m_unified_mode) {
			for (auto& u : m_agents) {
				if (u.agent.body->getGroundWeaponCooldown() > u.cooldown_last_frame) {
					m_total_damages_dealt += u.agent.body->getType().groundWeapon().damageAmount();
				}
			}
		} else {
			for (auto& u : m_agents) {
				if (u.agent.body->getGroundWeaponCooldown() > u.cooldown_last_frame) {
					u.damages_dealt += u.agent.body->getType().groundWeapon().damageAmount();
				}

				u.cooldown_last_frame = u.agent.body->getGroundWeaponCooldown();
			}
		}

		#ifdef DEBUG_SHOW_FITNESS
		rate();

		if (m_unified_mode) {
			BWAPI::Broodwar->drawTextScreen(10, 10,
				"%cTotal damages dealt: %d\n"
				"Alive agents: %d\n"
				"Current fitness: %0.1f",
				BWAPI::Text::White,
				m_total_damages_dealt,
				m_agents.size(),
				m_org->get_fitness()
			);
		} else {
			for (auto& u : m_agents) {
				BWAPI::Broodwar->drawTextMap(u.agent.body->getPosition() - BWAPI::Position(20, 10),
					"%c%d %0.1f", BWAPI::Text::White,
					u.damages_dealt,
					u.org->get_fitness()
				);
			}
		}
		#endif
	}

	for (size_t i = m_frame % FRAMES_PER_UPDATE; i < m_agents.size(); i += FRAMES_PER_UPDATE) {
		m_agents[i].agent.update();
	}
}

void NeatManager::register_unit(BWAPI::Unit unit) {
	if (m_use_best_genomes) {
		#ifdef NOVELTY_SEARCH
		std::optional<netkit::genome> geno;
		if (!m_best_genomes.empty()) {
			std::uniform_int_distribution<size_t> index_selector(0, m_best_genomes.size() - 1);
			geno = { m_best_genomes[index_selector(m_neat->rand_engine)] };
		}
		#else
		// Get genomes from the best genome library.
		auto geno = m_neat->get_random_genome_from_best_genome_library();
		#endif
		if (geno.has_value()) {
			m_agents.emplace_back(
				NeuralDrivenAgent(unit, geno->generate_network(), m_allow_stimpack_behaviour),
				std::optional<netkit::organism>(),
				0,
				0
			);
		}
	} else {
		if (m_unified_mode) {
			// create a new agent from the current organism
			m_agents.emplace_back(
				NeuralDrivenAgent(unit, m_org->get_genome().generate_network(), m_allow_stimpack_behaviour),
				std::optional<netkit::organism>(),
				0,
				0
			);
		} else {
			// Generate organisms from the population.
			netkit::organism org = m_neat->generate_and_get_next_organism();
			m_agents.emplace_back(
				NeuralDrivenAgent(unit, org.get_genome().generate_network(), m_allow_stimpack_behaviour),
				std::optional<netkit::organism>(std::move(org)),
				0,
				0
			);
		}
	}
}

void NeatManager::unregister_unit(BWAPI::Unit unit) {
	if (m_use_best_genomes | m_unified_mode) {
		m_agents.erase(std::remove_if(m_agents.begin(), m_agents.end(), [&unit](auto& na) {
			return na.agent.body->getID() == unit->getID();
		}), m_agents.end());
	} else {
		// rate the agent being removed if learning.
		#ifdef NOVELTY_SEARCH
		m_agents.erase(std::remove_if(m_agents.begin(), m_agents.end(), [this, &unit](auto& na) {
		#else
		m_agents.erase(std::remove_if(m_agents.begin(), m_agents.end(), [&unit](auto& na) {
		#endif
			if (na.agent.body->getID() == unit->getID()) {
				na.org->set_fitness(std::pow(
					static_cast<double>(na.damages_dealt * ATTACK_PERF_WEIGHT),
					EXPONENT_ON_FITNESS
				));

				#ifdef NOVELTY_SEARCH
				netkit::novelgenome<NovelPos> ng{ na.org->get_genome_id() };
				ng.get_pos().fitness = na.org->get_fitness();
				m_novelbank.pop_register(ng);
				m_all_ng.push_back(std::move(ng));
				#endif
				return true;
			} else {
				return false;
			}
		}), m_agents.end());
	}
}
