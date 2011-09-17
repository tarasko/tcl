#ifndef __GRIDWORLD_H__
#define __GRIDWORLD_H__

#include <tcl/rll/rll.hpp>

class CGridWorld : public tcl::rll::CEnvAction 
{
public:
    CGridWorld();

    /// @brief Describe different moves in grid world.
    class CGridWorldAction : public tcl::rll::CAction 
    {
    public:
        CGridWorldAction() : m_x(0), m_y(0), m_id(0) 
        {}
        CGridWorldAction(int i_x, int i_y, int i_id) : m_x(i_x), m_y(i_y), m_id(i_id) 
        {}

        virtual int getData() const 
        {
            return m_id;
        }

        void ChangeState(tcl::rll::CStatePtr i_ptrState, const std::map<int, int>& i_wind) 
        {
            int curRow = i_ptrState->GetValue("ROW");
            int curCol = i_ptrState->GetValue("COLUMN");
            i_ptrState->SetValue("COLUMN", applyColBounds(curCol + m_x));
            i_ptrState->SetValue("ROW", applyRowBounds(curRow + m_y + i_wind.at(curCol)));
        }

        bool IsPossibleInState(const tcl::rll::CStatePtr& state, const std::map<int, int>& wind) const
        {
            int row = state->GetValue("ROW");
            int col = state->GetValue("COLUMN");
            return 
                !(applyRowBounds(row + m_y + wind.at(col)) == row && 
                  applyColBounds(col + m_x) == col);
        }

    protected:
        static int applyRowBounds(int i_row) 
        {
            i_row = i_row > 6 ? 6 : i_row;
            i_row = i_row < 0 ? 0 : i_row;
            return i_row;
        }

        static int applyColBounds(int i_col) 
        {
            i_col = i_col > 9 ? 9 : i_col;
            i_col = i_col < 0 ? 0 : i_col;
            return i_col;
        }

        int m_x;
        int m_y;
        int m_id;
    };

    virtual void initEpisode();
    virtual size_t activeAgent() const;
    virtual tcl::rll::CStatePtr currentState() const;
    virtual std::vector<tcl::rll::CActionPtr> getPossibleActions() const;
    virtual bool doActionAssignRewards(const tcl::rll::CActionPtr& action);

    void                  PrintValueFunc();

private:
    std::map<int, int>    m_wind;
    std::vector<std::shared_ptr<CGridWorldAction> > m_actions;

    tcl::rll::CStatePtr   m_state;
};

#endif //__GRIDWORLD_H__
