#include <iostream>
#include <algorithm>

#include <netkit/csv/deserializer.h>
#include <netkit/csv/serializer.h>
#include <netkit/network/network.h>
#include <netkit/network/activation_functions.h>

#include "TacticOnlyAIModule.h"
#include "config.h"

using namespace BWAPI;
using namespace Filter;

void TacticOnlyAIModule::onStart() {
	// Hello World!
	Broodwar->sendText("Hello, it's TacticOnlyAIModule featuring NEAT.");

	// Enable the UserInput flag, which allows us to control the bot and type messages.
	Broodwar->enableFlag(Flag::UserInput);

	// Set the command optimization level so that common commands can be grouped
	// and reduce the bot's APM (Actions Per Minute).
	Broodwar->setCommandOptimizationLevel(2);

	// Retrieve Evolubot persistant data.
	netkit::deserializer des(LAST_AI_MODULE_STATE_FILENAME);
	des.get_next(m_fast_mode);
	des.get_next(m_show_best);
	des.close();

	// Init the NEAT manager.
	m_nmanager.init(m_show_best);

	if (m_fast_mode) {
		Broodwar->setLocalSpeed(0);
	}
}

void TacticOnlyAIModule::onEnd(bool isWinner) {
	// Called when the game ends
	if (isWinner) {
		Broodwar->sendText("Looks like I won.");
	} else {
		Broodwar->sendText("Oow...");
	}

	if (!m_show_best) {
		m_nmanager.rate_agents(); // rate survivors.
		m_nmanager.save();
	}

	if (!m_stop) {
		netkit::serializer ser(LAST_AI_MODULE_STATE_FILENAME);
		ser.append(m_fast_mode);
		ser.append(m_show_best);
		ser.close();

		Broodwar->restartGame();
	}
}

void TacticOnlyAIModule::onFrame() {
	// Display the game frame rate as text in the upper left area of the screen
	Broodwar->drawTextScreen(200, 0, "FPS: %d", Broodwar->getFPS());
	Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());

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
		if (m_show_best) {
			Broodwar << "Will resume evolution starting next round." << std::endl;
			m_show_best = false;
		} else {
			Broodwar << "Will only show best oganisms starting next round." << std::endl;
			m_show_best = true;
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
