#pragma once

#include <vector>
#include <memory>

#include <BWAPI.h>

#include "NeatManager.h"

// Remember not to use "Broodwar" in any global class constructor!

class TacticOnlyAIModule : public BWAPI::AIModule {
  public:
	TacticOnlyAIModule();

	// Virtual functions for callbacks, leave these as they are.
	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	virtual void onSendText(std::string text);
	virtual void onReceiveText(BWAPI::Player player, std::string text);
	virtual void onPlayerLeft(BWAPI::Player player);
	virtual void onNukeDetect(BWAPI::Position target);
	virtual void onUnitDiscover(BWAPI::Unit unit);
	virtual void onUnitEvade(BWAPI::Unit unit);
	virtual void onUnitShow(BWAPI::Unit unit);
	virtual void onUnitHide(BWAPI::Unit unit);
	virtual void onUnitCreate(BWAPI::Unit unit);
	virtual void onUnitDestroy(BWAPI::Unit unit);
	virtual void onUnitMorph(BWAPI::Unit unit);
	virtual void onUnitRenegade(BWAPI::Unit unit);
	virtual void onSaveGame(std::string gameName);
	virtual void onUnitComplete(BWAPI::Unit unit);

	void saveModuleData();
	void loadModuleData();

  private:
	NeatManager m_nmanager;

	bool m_fast_mode;
	bool m_stop;
	bool m_show_best_next_round;
	bool m_show_best_this_round;
	bool m_log_stats;
	bool m_exp_mode;

	size_t m_nb_evolution_rounds;
	size_t m_nb_best_rounds;
	size_t m_run_number;
	size_t m_exp_number;
};
