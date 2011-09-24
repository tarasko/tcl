// GridWorldState.cpp : Defines the entry point for the console application.
//
#include <tcl/rll/rll.hpp>

#include <vector>
#include <iostream>
#include <fstream>

using namespace tcl::rll;
using namespace std;

class grid_world : public env_state 
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

	void print_value_func();

private:
	virtual void init_episode();
    virtual size_t active_agent() const;
    virtual state_type current_state() const;
    virtual vector<state> get_possible_next_states() const;
    virtual bool set_next_state_assign_rewards(const state& state);

	static int apply_row_bounds(int i_row);
	static int apply_col_bounds(int i_col);

	vector<int> m_wind;
    state_type state_;
};

grid_world::action grid_world::ACTIONS[] = {
    { -1, 0 },
    { 1, 0 },
    { 0, -1 },
    { 0, 1 }
};

grid_world::grid_world()
    : m_wind(COLUMNS, 0)
    , state_(2)
{
    // Create value function and agent
    value_function_sp ptrFunc = make_shared<vf_lookup_table>();
    agents().push_back(make_shared<agent>(ptrFunc));

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

vector<grid_world::state_type> grid_world::get_possible_next_states() const
{
    vector<state_type> result;
    result.reserve(4);

    int curRow = state_[0];
    int curCol = state_[1];

    for (int i = 0; i < ACTIONS_SIZE; ++i) 
    {
        int new_row = apply_row_bounds(curRow + ACTIONS[i].y_ - m_wind.at(curCol));
        int new_col = apply_col_bounds(curCol + ACTIONS[i].x_);

        if (new_row != curRow || new_col != curCol)
        {
            state_type state = state_.clone();
            state[0] = new_row;
            state[1] = new_col;
            result.push_back(state);
        }
    }

    return result;
}

bool grid_world::set_next_state_assign_rewards(const state& state)
{
    state_ = state;

    if (state_[0] == 3 && state_[1] == 7)
    {
        cout << "Episode number:" << episode() << "\t" <<
            "Episode takes: " << step() << endl;

        agents()[0]->add_reward(1.0);
        return false;
    }
    else
    {
        agents()[0]->add_reward(-1.0);
        return true;
    }
}

void grid_world::print_value_func() 
{
    ofstream fout("out.txt");

    for (int row = 0; row < ROWS; ++row) 
    {
        state_[0] = row;
        for (int col = 0; col < COLUMNS; ++col) 
        {
            state_[1] = col;
            fout.width(9);
            fout << agents()[0]->get_value(state_.get_internal_rep()) << " ";
        }
        fout << endl;
    }
}

int grid_world::apply_row_bounds(int i_row) 
{
    i_row = i_row > 6 ? 6 : i_row;
    i_row = i_row < 0 ? 0 : i_row;
    return i_row;
}

int grid_world::apply_col_bounds(int i_col) 
{
    i_col = i_col > 9 ? 9 : i_col;
    i_col = i_col < 0 ? 0 : i_col;
    return i_col;
}

int main(int argc, char* argv[]) 
{
    CConfigPtr config(new CConfig);
    config->m_gamma = 1.0;

    grid_world gw;
    policy_egreedy pol;
    method_state_onpolicy m(&gw, &pol, config);

    m.run(5000);
    gw.print_value_func();
    return 0;
}

