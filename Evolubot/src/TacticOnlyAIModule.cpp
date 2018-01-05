#include <iostream>
#include <algorithm>
#include <filesystem>

#include <netkit/csv/deserializer.h>
#include <netkit/csv/serializer.h>
#include <netkit/network/network.h>
#include <netkit/network/activation_functions.h>

#include "TacticOnlyAIModule.h"
#include "config.h"
#include "Experiment.h"

using namespace BWAPI;
using namespace Filter;

unsigned int nb_generations = 100;
unsigned int nb_generations_unified = 20;
unsigned int nb_best_rounds = 50;
unsigned int runs_per_exp = 3;
unsigned int nb_exps = 12;

#ifdef NOVELTY_SEARCH
const Experiment exps[] = {
	//     marines unified  cascade  map                    filename
	Experiment(true, false, false, "marine_vs_marine.scx", "exp_marine_vs_marine_novelty_NEAT/"),
	Experiment(true, false, false, "marine_vs_zerg.scx", "exp_marine_vs_zerg_novelty_NEAT/"),
	Experiment(false, false, false, "vulture_vs_vulture.scx", "exp_vulture_vs_vulture_novelty_NEAT/"),
	Experiment(false, false, false, "vulture_vs_xealot.scx", "exp_vulture_vs_xealot_novelty_NEAT/")
};
#else
const Experiment exps[] = {
	//     marines unified cascade  map                    filename
	Experiment(true, false, false, "marine_vs_marine.scx", "exp_marine_vs_marine_vanilla_NEAT/"),
	Experiment(true, true, false, "marine_vs_marine.scx", "exp_marine_vs_marine_unified_NEAT/"),
	Experiment(true, false, true, "marine_vs_marine.scx", "exp_marine_vs_marine_cascade_NEAT/"),

	Experiment(true, false, false, "marine_vs_zerg.scx", "exp_marine_vs_zerg_vanilla_NEAT/"),
	Experiment(true, true, false, "marine_vs_zerg.scx", "exp_marine_vs_zerg_unified_NEAT/"),
	Experiment(true, false, true, "marine_vs_zerg.scx", "exp_marine_vs_zerg_cascade_NEAT/"),

	Experiment(false, false, false, "vulture_vs_vulture.scx", "exp_vulture_vs_vulture_vanilla_NEAT/"),
	Experiment(false, true, false, "vulture_vs_vulture.scx", "exp_vulture_vs_vulture_unified_NEAT/"),
	Experiment(false, false, true, "vulture_vs_vulture.scx", "exp_vulture_vs_vulture_cascade_NEAT/"),

	Experiment(false, false, false, "vulture_vs_xealot.scx", "exp_vulture_vs_xealot_vanilla_NEAT/"),
	Experiment(false, true, false, "vulture_vs_xealot.scx", "exp_vulture_vs_xealot_unified_NEAT/"),
	Experiment(false, false, true, "vulture_vs_xealot.scx", "exp_vulture_vs_xealot_cascade_NEAT/")
};
#endif

TacticOnlyAIModule::TacticOnlyAIModule()
	: m_nmanager()
	, m_fast_mode()
	, m_stop()
	, m_show_best_next_round()
	, m_show_best_this_round()
	, m_log_stats()
	, m_nb_evolution_rounds()
	, m_nb_best_rounds()
	, m_run_number()
	, m_exp_number() {}

void TacticOnlyAIModule::onStart() {
	// Hello World!
	Broodwar->sendText("Hello, it's TacticOnlyAIModule featuring NEAT.");

	// Enable the UserInput flag, which allows us to control the bot and type messages.
	Broodwar->enableFlag(Flag::UserInput);

	// Set the command optimization level so that common commands can be grouped
	// and reduce the bot's APM (Actions Per Minute).
	Broodwar->setCommandOptimizationLevel(2);

	// Retrieve Evolubot persistant data.
	loadModuleData();
	m_show_best_this_round = m_show_best_next_round;

	bool force_reset_neat = false;
	if (m_exp_mode) {
		if (m_run_number == 0) {
			force_reset_neat = true;
			m_run_number = 1;
		}

		if (!m_show_best_this_round) {
			++m_nb_evolution_rounds;
		}

		if (exps[m_exp_number].unified_mode) {
			if (m_nb_evolution_rounds > nb_generations_unified * POPULATION_SIZE) {
				m_show_best_this_round = true;
				m_show_best_next_round = true;
			}
		} else {
			if (m_nb_evolution_rounds > nb_generations) {
				m_show_best_this_round = true;
				m_show_best_next_round = true;
			}
		}

		if (m_show_best_this_round) {
			++m_nb_best_rounds;
		}
		
		if (m_nb_best_rounds > nb_best_rounds) {
			std::rename(MODULE_FOLDER LAST_NEAT_STATE_FILENAME,
				(MODULE_FOLDER + exps[m_exp_number].log_folder + std::to_string(m_run_number) + "_" LAST_NEAT_STATE_FILENAME).c_str());

			++m_run_number;
			m_nb_best_rounds = 0;
			m_nb_evolution_rounds = 1;
			m_show_best_this_round = false;
			m_show_best_next_round = false;
			force_reset_neat = true;

			if (m_run_number > runs_per_exp) {
				++m_exp_number;
				if (m_exp_number > nb_exps) {
					Broodwar->leaveGame();
				}

				m_run_number = 0;
				m_nb_evolution_rounds = 0;

				saveModuleData();
				Broodwar->setMap("maps/custom/" + exps[m_exp_number].map_filename);
				Broodwar->restartGame();
			}
		}
	}

	// Init the NEAT manager.
	if (m_exp_mode) {
		m_nmanager.init(POPULATION_SIZE, m_show_best_this_round, force_reset_neat,
			exps[m_exp_number].use_marines, exps[m_exp_number].unified_mode, exps[m_exp_number].cascade_mode);
	} else {
		m_nmanager.init(POPULATION_SIZE, m_show_best_this_round, force_reset_neat);
	}
	
	if (m_fast_mode) {
		Broodwar->setLocalSpeed(0);
	}
}

void TacticOnlyAIModule::onEnd(bool isWinner) {
	if (Broodwar->elapsedTime() < 6) {
		return;
	}

	// Called when the game ends
	if (isWinner) {
		Broodwar->sendText("Looks like I won.");
	} else {
		Broodwar->sendText("Oow...");
	}

	if (m_show_best_this_round) {
		if (m_log_stats) {
			std::string filename;
			if (m_exp_mode) {
				filename = MODULE_FOLDER + exps[m_exp_number].log_folder + std::to_string(m_run_number) + "_" STATS_BEST_UNITS_FILENAME;
			} else {
				filename = MODULE_FOLDER STATS_BEST_UNITS_FILENAME;
			}

			netkit::serializer ser(filename, ";", true);
			ser.append(isWinner);
			ser.append(m_nmanager.number_of_agents());
			ser.new_line();
			ser.close();
		}
	} else {
		m_nmanager.rate(); // rate survivors / current organism (unified mode)
		m_nmanager.get_neat().update_best_genome_ever();

		if (m_log_stats && m_nmanager.last_organism_used()) {
			std::string filename;
			if (m_exp_mode) {
				filename = MODULE_FOLDER + exps[m_exp_number].log_folder + std::to_string(m_run_number) + "_" STATS_EVOLVING_FILENAME;
			} else {
				filename = MODULE_FOLDER STATS_EVOLVING_FILENAME;
			}

			netkit::serializer ser(filename, ";", true);

			ser.append(isWinner);
			ser.append(m_nmanager.number_of_agents());

			// compute population's average fitness.
			double whole_population_avg_fit = 0.0;
			for (const auto& geno : m_nmanager.get_neat().pop()->get_all_genomes()) {
				whole_population_avg_fit += geno.get_fitness();
			}
			
			whole_population_avg_fit /= static_cast<double>(m_nmanager.get_neat().pop()->size());
			ser.append(whole_population_avg_fit);

			ser.append(m_nmanager.get_neat().get_current_best_genome().get_fitness());
			ser.append(m_nmanager.get_neat().get_best_genome_ever()->get_fitness());

			double current_best = m_nmanager.get_neat().get_current_best_genome().get_fitness();
			double best_ever = m_nmanager.get_neat().get_best_genome_ever()->get_fitness();

			ser.new_line();
			ser.close();
		}

		#ifdef NOVELTY_SEARCH
		m_nmanager.update_novelty();
		#endif

		m_nmanager.save();
	}

	if (!m_stop) {
		saveModuleData();
		Broodwar->restartGame();
	}
}

void TacticOnlyAIModule::onFrame() {
	// Display the game frame rate as text in the upper left area of the screen
	Broodwar->drawTextScreen(200, 0, "FPS: %d", Broodwar->getFPS());
	Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());

	if (m_exp_mode) {
		Broodwar->drawTextScreen(200, 40, "Exp nb %ul", m_exp_number);
		Broodwar->drawTextScreen(200, 50, "Run nb %ul", m_run_number);
		Broodwar->drawTextScreen(200, 60, "Nb evol: %ul", m_nb_evolution_rounds);
		Broodwar->drawTextScreen(200, 70, "Nb best: %ul", m_nb_best_rounds);
	}

	if (m_log_stats) {
		Broodwar->drawTextScreen(300, 40, "Log stats on");
	}

	if (m_fast_mode) {
		Broodwar->drawTextScreen(300, 55, "Fast mode on");
	}

	// Return if the game is a replay or is paused
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self())
		return;

	// The neat manager already avoid spamming (act as a scheduler).
	m_nmanager.update_units();

	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
		return;

	// Room for other actions.
}

void TacticOnlyAIModule::onSendText(std::string text) {
	if (text == "/tf") { // toggle fast mode
		if (m_fast_mode) {
			Broodwar << "Not in fast mode anymore." << std::endl;
			Broodwar->setLocalSpeed(50);
			m_fast_mode = false;
		} else {
			Broodwar << "Now in fast mode." << std::endl;
			Broodwar->setLocalSpeed(0);
			m_fast_mode = true;
		}
		return;
	} else if (text == "/stop") { // stop the simulation at the end of the round.
		if (m_stop) {
			Broodwar << "Canceled stop order." << std::endl;
			m_stop = false;
		} else {
			Broodwar << "No round will be restarted." << std::endl;
			m_stop = true;
		}
		return;
	} else if (text == "/showbest") {
		if (m_show_best_next_round) {
			Broodwar << "Will resume evolution starting next round." << std::endl;
			m_show_best_next_round = false;
		} else {
			Broodwar << "Will only show best oganisms starting next round." << std::endl;
			m_show_best_next_round = true;
		}
		return;
	} else if (text == "/logstats") {
		if (m_log_stats) {
			Broodwar << "Stop logging stats." << std::endl;
			m_log_stats = false;
		} else {
			Broodwar << "Start logging stats." << std::endl;
			m_log_stats = true;
		}
		return;
	} else if (text == "/startexps") {
		if (m_exp_mode == false) {
			Broodwar << "Start experiments." << std::endl;
			m_log_stats = true;
			m_exp_mode = true;
			m_show_best_next_round = false;
			m_run_number = 0;
			m_nb_best_rounds = 0;
			m_nb_evolution_rounds = 0;
			m_exp_number = 0;

			saveModuleData();
			Broodwar->setMap("maps/custom/" + exps[m_exp_number].map_filename);
			Broodwar->restartGame();
		} else {
			Broodwar << "Stop experiments." << std::endl;
			m_log_stats = false;
			m_exp_mode = false;
		}
		return;
	}

	// Send the text to the game if it is not being processed.
	Broodwar->sendText("%s", text.c_str());

	// Make sure to use %s and pass the text as a parameter,
	// otherwise you may run into problems when you use the %(percent) character!
}

void TacticOnlyAIModule::onReceiveText(BWAPI::Player player, std::string text) {
	// Parse the received text
	Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void TacticOnlyAIModule::onPlayerLeft(BWAPI::Player player) {
}

void TacticOnlyAIModule::onNukeDetect(BWAPI::Position target) {
}

void TacticOnlyAIModule::onUnitDiscover(BWAPI::Unit unit) {
}

void TacticOnlyAIModule::onUnitEvade(BWAPI::Unit unit) {
}

void TacticOnlyAIModule::onUnitShow(BWAPI::Unit unit) {
}

void TacticOnlyAIModule::onUnitHide(BWAPI::Unit unit) {
}

void TacticOnlyAIModule::onUnitCreate(BWAPI::Unit unit) {
	m_nmanager.register_unit(unit);
}

void TacticOnlyAIModule::onUnitDestroy(BWAPI::Unit unit) {
	m_nmanager.unregister_unit(unit);
}

void TacticOnlyAIModule::onUnitMorph(BWAPI::Unit unit) {
}

void TacticOnlyAIModule::onUnitRenegade(BWAPI::Unit unit) {
}

void TacticOnlyAIModule::onSaveGame(std::string gameName) {
}

void TacticOnlyAIModule::onUnitComplete(BWAPI::Unit unit) {
}

void TacticOnlyAIModule::saveModuleData() {
	netkit::serializer ser(MODULE_FOLDER LAST_AI_MODULE_STATE_FILENAME);
	ser.append(m_fast_mode);
	ser.append(m_show_best_next_round);
	ser.append(m_log_stats);
	ser.append(m_exp_mode);
	ser.append(m_nb_evolution_rounds);
	ser.append(m_nb_best_rounds);
	ser.append(m_run_number);
	ser.append(m_exp_number);
	ser.close();
}

void TacticOnlyAIModule::loadModuleData() {
	netkit::deserializer des(MODULE_FOLDER LAST_AI_MODULE_STATE_FILENAME);
	des.get_next(m_fast_mode);
	des.get_next(m_show_best_next_round);
	des.get_next(m_log_stats);
	des.get_next(m_exp_mode);
	des.get_next(m_nb_evolution_rounds);
	des.get_next(m_nb_best_rounds);
	des.get_next(m_run_number);
	des.get_next(m_exp_number);
	des.close();
}
