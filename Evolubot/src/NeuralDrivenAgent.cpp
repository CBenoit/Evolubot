#include <algorithm>

#include <BWAPI/Position.h>
#include <BWAPI.h>
#include <netkit/network/network.h>
#include <netkit/network/activation_functions.h>

#include "NeuralDrivenAgent.h"
#include "config.h"
#include "utils.h"

NeuralDrivenAgent::NeuralDrivenAgent(BWAPI::Unit unit, netkit::network&& net, bool allow_stimpack_behaviour)
	: body(unit)
	, m_net(std::move(net))
	, m_allow_stimpack_behaviour(allow_stimpack_behaviour) {}

void NeuralDrivenAgent::update() {
	int distance_to_closest_enemy = std::numeric_limits<int>::max();
	BWAPI::Unit closest_enemy = body->getClosestUnit(BWAPI::Filter::IsEnemy);
	if (closest_enemy != nullptr) {
		distance_to_closest_enemy = body->getDistance(closest_enemy);
	}

	int distance_to_closest_ally = std::numeric_limits<int>::max();
	BWAPI::Unit closest_ally = body->getClosestUnit(BWAPI::Filter::IsAlly);
	if (closest_ally != nullptr) {
		distance_to_closest_ally = body->getDistance(closest_ally);
	}

	// network feeding
	{
		// == normalize inputs ==
		double input_cooldown = static_cast<double>(body->getGroundWeaponCooldown())
									/ body->getType().groundWeapon().damageCooldown();

		double input_enemy_distance = distance_to_closest_enemy >= MAX_DISTANCE_NN
			? 1.0
			: static_cast<double>(distance_to_closest_enemy) / MAX_DISTANCE_NN;

		size_t number_enemies = body->getUnitsInRadius(body->getType().sightRange(), BWAPI::Filter::IsEnemy).size();
		double input_enemies_density = number_enemies >= MAX_ENTITY_NN
			? 1.0
			: static_cast<double>(number_enemies) / MAX_ENTITY_NN;

		double input_ally_distance = distance_to_closest_ally >= MAX_DISTANCE_NN
			? 1.0
			: static_cast<double>(distance_to_closest_ally) / MAX_DISTANCE_NN;

		size_t number_allies = body->getUnitsInRadius(body->getType().sightRange(), BWAPI::Filter::IsAlly).size();
		double input_allies_density = number_allies >= MAX_ENTITY_NN
			? 1.0
			: static_cast<double>(number_allies) / MAX_ENTITY_NN;
		// == normalizaton done ==

		if (m_allow_stimpack_behaviour) {
			m_net.load_inputs({
				input_cooldown,
				input_enemy_distance,
				input_enemies_density,
				input_ally_distance,
				input_allies_density,
				body->isStimmed() ? 1.0 : 0.0
			});
		} else {
			m_net.load_inputs({
				input_cooldown,
				input_enemy_distance,
				input_enemies_density,
				input_ally_distance,
				input_allies_density
			});
		}

		m_net.activate();
	}

	// action selection
	std::vector<netkit::neuron_value_t> outputs = m_net.get_outputs();
	unsigned int choosen_action = 99;
	netkit::neuron_value_t best = 0;
	for (unsigned int i = 0; i < outputs.size(); i++) {
		if (outputs[i] > 0.5 && outputs[i] > best) {
			best = outputs[i];
			choosen_action = i;
		}
	}

	// build and give order based on the selected action to the constrolled body.
	BWAPI::Position weighted_vector(0, 0);
	switch (choosen_action)
	{
	case 0: { // attack
		// select the weaker enemy in weapon range
		BWAPI::Unit weaker = nullptr;
		int weaker_hp = std::numeric_limits<int>::max();
		for (BWAPI::Unit e : body->getUnitsInRadius(body->getType().groundWeapon().maxRange(), BWAPI::Filter::IsEnemy)) {
			if (e->getHitPoints() < weaker_hp) {
				weaker = e;
				weaker_hp = e->getHitPoints();
			}
		}

		if (weaker != nullptr) {
			body->attack(weaker);
		} else if (closest_ally != nullptr) {
			body->attack(closest_enemy);
		}
		break;
	}
	case 1: // retreat
		// takes into accound all enemies in range.
		for (BWAPI::Unit e : body->getUnitsInRadius(body->getType().sightRange(), BWAPI::Filter::IsEnemy)) {
			int damages_per_attack = e->getType().groundWeapon().damageAmount() * e->getType().groundWeapon().damageFactor();

			if (body->getPosition().x != e->getPosition().x) {
				weighted_vector.x += 1000 / (body->getPosition().x - e->getPosition().x) * damages_per_attack;
			}

			if (body->getPosition().y != e->getPosition().y) {
				weighted_vector.y += 1000 / (body->getPosition().y - e->getPosition().y) * damages_per_attack;
			}
		}

		// TODO: takes into accound terrain
		/* This is NOT working to my knowledge.
		BWAPI::TilePosition tilepos = body->getTilePosition();
		int ground_height = BWAPI::Broodwar->getGroundHeight(tilepos);
		for (int i = -5; i <= 5; ++i) {
		for (int j = -5; i <= 5; ++i) {
		int height_diff = std::abs(ground_height - BWAPI::Broodwar->getGroundHeight(tilepos.x + i, tilepos.y + j));
		weighted_vector += (body->getPosition() - BWAPI::Position(i * 32 + 16, j * 32 + 16)) * height_diff;
		BWAPI::Broodwar << height_diff << " ";
		}
		BWAPI::Broodwar << std::endl;
		}
		BWAPI::Broodwar << "==============="; */

		if (weighted_vector != BWAPI::Position(0, 0)) {
			// takes into accound the allies as obstacles
			for (BWAPI::Unit e : body->getUnitsInRadius(body->getType().sightRange(), BWAPI::Filter::IsAlly)) {
				if (body->getPosition().x != e->getPosition().x) {
					weighted_vector.x += 100 / (body->getPosition().x - e->getPosition().x);
				}

				if (body->getPosition().y != e->getPosition().y) {
					weighted_vector.y += 100 / (body->getPosition().y - e->getPosition().y);
				}
			}

			// normalize
			weighted_vector = weighted_vector * 150 / weighted_vector.getLength();
		}

		body->move(body->getPosition() + weighted_vector);
		break;
	case 2: // spread out
		// takes into accound all units in range.
		for (BWAPI::Unit e : body->getUnitsInRadius(body->getType().sightRange(), BWAPI::Filter::IsAlly || BWAPI::Filter::IsEnemy)) {
			if (body->getPosition().x != e->getPosition().x) {
				weighted_vector.x += 1000 / (body->getPosition().x - e->getPosition().x);
			}

			if (body->getPosition().y != e->getPosition().y) {
				weighted_vector.y += 1000 / (body->getPosition().y - e->getPosition().y);
			}
		}

		// normalize
		if (weighted_vector != BWAPI::Position(0, 0)) {
			weighted_vector = weighted_vector * 150 / weighted_vector.getLength();
		}

		body->move(body->getPosition() + weighted_vector);
		break;
	case 3: // gathering
		// takes into accound all allies in range.
		for (BWAPI::Unit e : body->getUnitsInRadius(body->getType().sightRange(), BWAPI::Filter::IsAlly)) {
			if (body->getPosition().x != e->getPosition().x) {
				weighted_vector.x += 1000 / (e->getPosition().x - body->getPosition().x);
			}

			if (body->getPosition().y != e->getPosition().y) {
				weighted_vector.y += 1000 / (e->getPosition().y - body->getPosition().y);
			}
		}

		// normalize
		if (weighted_vector != BWAPI::Position(0, 0)) {
			weighted_vector = weighted_vector * 150 / weighted_vector.getLength();
			body->move(body->getPosition() + weighted_vector);
		} else if (closest_ally != nullptr) {
			body->move(closest_ally->getPosition());
		} else {
			body->move(body->getPosition()); // = do nothing
		}
		break;
	case 4: // use stimpack
		if (!body->isStimmed()) {
			body->useTech(BWAPI::TechTypes::Stim_Packs);
		}
	default: // do nothing
		body->move(body->getPosition());
		break;
	}

	#ifdef DEBUG_SHOW_DECISION
	draw_line_between(body->getPosition(), body->getTargetPosition(),
		BWAPI::Color(255, 128, 128), FRAMES_PER_UPDATE);

	BWAPI::Broodwar->registerEvent([choosen_action, this](BWAPI::Game*) {
			BWAPI::Broodwar->drawTextMap(body->getPosition() - BWAPI::Position(10, 0),
				"%c%ul %d", BWAPI::Text::White,
				choosen_action, static_cast<int>(body->isStimmed()));
		},
		nullptr,
		FRAMES_PER_UPDATE
	);
	#endif
}
