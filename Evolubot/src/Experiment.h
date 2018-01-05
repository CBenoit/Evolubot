#pragma once

#include <string>

#include "config.h"

struct Experiment {
	Experiment(bool use_marines_, bool unified_mode_,
		bool cascade_mode_, std::string map_filename_, std::string log_folder_ = "")
		: use_marines(use_marines_)
		, unified_mode(unified_mode_)
		#ifdef NOVELTY_SEARCH
		, novelty_mode(true)
		#else
		, novelty_mode(false)
		#endif
		, cascade_mode(cascade_mode_)
		, map_filename(map_filename_)
		, log_folder(log_folder_) {}

	bool use_marines;
	bool unified_mode;
	bool novelty_mode;
	bool cascade_mode;
	std::string map_filename;
	std::string log_folder;
};
