#include <fstream>

#include <BWAPI.h>

#include "utils.h"

void draw_line_between(BWAPI::Position from, BWAPI::Position to, BWAPI::Color color, unsigned int nb_frames) {
	BWAPI::Broodwar->registerEvent([from, to, color](BWAPI::Game*) {
			BWAPI::Broodwar->drawLineMap(from.x, from.y, to.x, to.y, color);
		},
		nullptr,
		nb_frames
	);
}

void draw_circle_at(BWAPI::Position pos, BWAPI::Color color, unsigned int nb_frames) {
		BWAPI::Broodwar->registerEvent([pos, color](BWAPI::Game*) {
			BWAPI::Broodwar->drawCircleMap(pos.x, pos.y, 10, color, false);
		},
		nullptr,
		nb_frames
	);
}

bool file_exist(const char *filename) {
	std::ifstream f(filename);
	return f.good();
}
