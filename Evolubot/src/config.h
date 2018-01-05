#pragma once

// === General settings ===
#define MODULE_FOLDER "evolubot/"
#define LAST_NEAT_STATE_FILENAME "last_neat_state.csv"
#define LAST_AI_MODULE_STATE_FILENAME "tactic_only_ai_module_data.csv"
#define STATS_EVOLVING_FILENAME "stats_evolving.csv"
#define STATS_BEST_UNITS_FILENAME "stats_best_units.csv"

#define POPULATION_SIZE 22

// whether to rate organisms by novelty.
//#define NOVELTY_SEARCH

// === Neural driven agents settings ===
// --- See: NeuralDrivenAgent.

// the maximal distance a neural network can perceive. For normalization purpose.
#define MAX_DISTANCE_NN 1500

// the maximal number of entities a neural network can perveice. For normalization purpose again.
#define MAX_ENTITY_NN 20

// === NEAT evolution settings ===
// --- See: NeatManager.

// The number of frames between each agent's update.
#define FRAMES_PER_UPDATE 10

#define SURVIVAL_PERF_WEIGHT 0.5
#define ATTACK_PERF_WEIGHT 0.5
#define COOPERATIVE_PERF_WEIGHT 10

#define UNIFIED_ATTACK_PERF_WEIGHT 0.33
#define UNIFIED_COOPERATIVE_PERF_WEIGHT 100

#define EXPONENT_ON_FITNESS 1.3

// === Debug outputs ===

// Disable all debug output.
//#define NO_DEBUG_OUTPUT

// Generic debug outputs (stats, ...)
#define DEBUG_OUTPUT_GENERIC
// Show fitness over agents.
#define DEBUG_SHOW_FITNESS
// Show selected action over agents.
#define DEBUG_SHOW_DECISION

#ifdef NO_DEBUG_OUTPUT
	#undef DEBUG_OUTPUT_GENERIC
	#undef DEBUG_SHOW_FITNESS
	#undef DEBUG_SHOW_DECISION
#endif
