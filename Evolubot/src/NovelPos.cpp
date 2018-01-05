#include "NovelPos.h"

double NovelPos::novelty_distance(const NovelPos& other) const {
	return std::abs(other.fitness - fitness);
}

netkit::serializer& operator<<(netkit::serializer& ser, const NovelPos& np) {
	ser.append(np.fitness);
	ser.new_line();

	return ser;
}

netkit::deserializer& operator>>(netkit::deserializer& des, NovelPos& np) {
	des.get_next(np.fitness);

	return des;
}
