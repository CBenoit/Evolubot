#pragma once

#include <BWAPI/Color.h>
#include <BWAPI/Position.h>

void draw_line_between(BWAPI::Position from, BWAPI::Position to, BWAPI::Color color, unsigned int nb_frames);

void draw_circle_at(BWAPI::Position pos, BWAPI::Color color, unsigned int nb_frames);

// Quick and simple method to check if a file "exists" (not "exactly" that though).
// Good enough for our purpose.
bool file_exist(const char *filename);
