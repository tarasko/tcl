#pragma once

#include <tcl/rll/rll.hpp>

class CGridWorld : public tcl::rll::CEnvState 
{
public:
	CGridWorld(void);

	void PrintValueFunc();

private:
	virtual void initEpisodeImpl();
	virtual void fillPossibilities(CPossibleStates& o_states);
	virtual bool observeRewardImpl(double& o_reward) const;
	virtual tcl::rll::CVectorDbl observeTerminalRewardsImpl() const;
    virtual int selectNextAgentImpl();

	static bool isTerminalState(tcl::rll::CStatePtr i_ptrState);

	int applyRowBounds(int i_row);
	int applyColBounds(int i_col);

	/** @brief Wind map */
	std::map<int, int> m_wind;
};
