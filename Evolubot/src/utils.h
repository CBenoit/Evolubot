#pragma once

#include <BWAPI/Color.h>
#include <BWAPI/Position.h>

void draw_line_between(BWAPI::Position from, BWAPI::Position to, BWAPI::Color color, unsigned int nb_frames);

void draw_circle_at(BWAPI::Position pos, BWAPI::Color color, unsigned int nb_frames);
