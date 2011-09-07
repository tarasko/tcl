#pragma once

#include <tcl/rll/rll.hpp>

using namespace tcl::rll;

class CGridWorld : public CEnvState 
{
public:
	CGridWorld(void);

	void PrintValueFunc();

private:
	virtual void initEpisodeImpl();
    virtual size_t activeAgentImpl() const;
    virtual CStatePtr currentStateImpl() const;
    virtual bool nextStepImpl();
	virtual CVectorDbl observeTerminalRewardsImpl() const;
    virtual std::vector<CStatePtr> getPossibleNextStatesImpl() const;
    virtual double setNextStateObserveRewardImpl(const CStatePtr& state);

	static bool isTerminalState(CStatePtr i_ptrState);
	static int applyRowBounds(int i_row);
	static int applyColBounds(int i_col);

	/** @brief Wind map */
	std::map<int, int> m_wind;
    CStatePtr m_state;
};
