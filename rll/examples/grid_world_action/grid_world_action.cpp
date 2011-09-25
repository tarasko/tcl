// GridWorldAction.cpp : Defines the entry point for the console application.
//

#include <tcl/rll/rll.hpp>

#include <vector>
#include <iostream>

using namespace std;
using namespace tcl::rll;

class grid_world : public tcl::rll::env_action 
{
    static const int ROWS = 7;
    static const int COLUMNS = 10;

    struct action
    {
        int x_;
        int y_;
    };

    static action ACTIONS[];
    static const int ACTIONS_SIZE = 4;

public:
    grid_world();

    virtual void init_episode();
    virtual size_t active_agent() const;
    virtual state_type current_state() const;
    virtual vector<rll_type> get_possible_actions() const;
    virtual bool do_action_assign_rewards(rll_type action);

private:
    static int apply_row_bounds(int i_row) 
    {
        i_row = i_row > (ROWS - 1) ? (ROWS - 1) : i_row;
        i_row = i_row < 0 ? 0 : i_row;
        return i_row;
    }

    static int apply_col_bounds(int i_col) 
    {
        i_col = i_col > (COLUMNS - 1) ? (COLUMNS - 1) : i_col;
        i_col = i_col < 0 ? 0 : i_col;
        return i_col;
    }

    vector<int> m_wind;
    state_type state_;
    value_function::lookup_table vf_;
};

grid_world::action grid_world::ACTIONS[] = {
    { -1, 0 },
    { 1, 0 },
    { 0, -1 },
    { 0, 1 }
};


grid_world::grid_world(void) 
    : m_wind(COLUMNS, 0)
    , state_(2)
{
    // Create value function and agent
    agents().push_back(make_shared<agent>(&vf_));

    // Init wind
    m_wind[3] = 1;
    m_wind[4] = 1;
    m_wind[5] = 1;
    m_wind[6] = 2;
    m_wind[7] = 2;
    m_wind[8] = 1;
}

void grid_world::init_episode() 
{
    state_[0] = 3;
    state_[1] = 0;
}

size_t grid_world::active_agent() const
{
    return 0;
}

grid_world::state_type grid_world::current_state() const
{
    return state_;
}

vector<rll_type> grid_world::get_possible_actions() const
{
    vector<rll_type> res;
    res.reserve(ACTIONS_SIZE);

    int row = state_[0];
    int col = state_[1];

    for (int i = 0; i<ACTIONS_SIZE; ++i) 
    {
        if (apply_row_bounds(row + ACTIONS[i].y_) != row ||
            apply_col_bounds(col + ACTIONS[i].x_) != col)
        {
            res.push_back(i);
        }
    }

    return res;
}

bool grid_world::do_action_assign_rewards(rll_type action)
{
    int curRow = state_[0];
    int curCol = state_[1];
    state_[0] = apply_row_bounds(state_[0] + ACTIONS[action].y_ - m_wind.at(state_[1]));
    state_[1] = apply_col_bounds(state_[1] + ACTIONS[action].x_);

    bool finished = state_[0] == 3 && state_[1] == 7;

    if (finished)
    {
        agents()[0]->add_reward(1.0);
        cout 
            << "Episode number:" << episode() << "\t" 
            << "Episode takes: " << step() << endl;
    }
    else
        agents()[0]->add_reward(-1.0);

    return !finished;
}

int main(int argc, char* argv[]) 
{
    config cfg;
    cfg.m_gamma = 1.0;
    cfg.m_accumulating = false;

    grid_world gw;
    policy::egreedy pol;
    onpolicy_action_method m(&gw, &pol, cfg);

    m.run(2000);
    return 0;
}
