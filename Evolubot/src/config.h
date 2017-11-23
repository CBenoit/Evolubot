#pragma once

// === General settings ===
#define MODULE_FOLDER "evolubot/"
#define LAST_NEAT_STATE_FILENAME MODULE_FOLDER"last_neat_state.csv"
#define LAST_AI_MODULE_STATE_FILENAME MODULE_FOLDER"tactic_only_ai_module_data.csv"
#define STATS_EVOLVING_FILENAME MODULE_FOLDER"stats_evolving.csv"
#define STATS_BEST_UNITS_FILENAME MODULE_FOLDER"stats_best_units.csv"

// === Neural driven agents settings ===
// --- See: NeuralDrivenAgent.

// the maximal distance a neural network can perceive. For normalization purpose.
#define MAX_DISTANCE_NN 1000

// the maximal number of entities a neural network can perveice. For normalization purpose again.
#define MAX_ENTITY_NN 20

// === NEAT evolution settings ===
// --- See: NeatManager.

// The number of frames between each agent's update.
#define FRAMES_PER_UPDATE 10

#define SURVIVAL_PERF_WEIGHT 1
#define ATTACK_PERF_WEIGHT 1
#define COOPERATIVE_PERF_WEIGHT 20

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
