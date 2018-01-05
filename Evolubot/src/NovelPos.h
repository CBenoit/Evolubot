#pragma once

#include <vector>

#include <netkit/csv/deserializer.h>
#include <netkit/csv/serializer.h>

struct NovelPos {
	double novelty_distance(const NovelPos& other) const;

	double fitness;
};

netkit::serializer& operator<<(netkit::serializer& ser, const NovelPos& np);

netkit::deserializer& operator>>(netkit::deserializer& des, NovelPos& np);
