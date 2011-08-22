#pragma once

#include "config.hpp"

#include <random>
#include <map>
#include <algorithm>

namespace tcl { namespace rll {

/// @brief Help to select actions.
class CPolicy 
{
public:
    CPolicy(CConfigPtr i_ptrConfig) : m_ptrConfig(i_ptrConfig) {}
  
    DEFINE_EXCEPTION(CPolicyException)

    /** @brief Select element from map according to selected policy. 
        T - should be map with key type as double */
    template<class T>
    typename T::const_iterator Select(const T& i_variants) 
    {
        if (i_variants.empty()) {
            throw CPolicyException("Cannot select action from empty set");
        }
        if (1 == i_variants.size()) {
            return i_variants.begin();
        }
        switch (m_ptrConfig->m_policy) {
            case CConfig::GREEDY: return runGreedy(i_variants);
            case CConfig::EPSILON_GREEDY: return runEgreedy(i_variants);
            default: return runEgreedy(i_variants);
        }    
    }

protected:
    /** @brief Use greedy policy to determine next action */
    template<class T>
    typename T::const_iterator runGreedy(const T& i_variants) 
    {
        // Determine first greedy action (We can have many greedy actions)
        typename T::const_iterator iBegin = i_variants.lower_bound((--i_variants.end())->first);
        typename T::size_type num = std::distance(iBegin, i_variants.end());

        std::uniform_int<> int_dist(0, static_cast<int>(num) - 1);
        std::advance(iBegin, int_dist(m_gen));

        return iBegin;
    }

    /** @brief Use e-greedy policy to determine next action */
    template<class T>
    typename T::const_iterator runEgreedy(const T& i_variants) 
    {
        std::uniform_real<> real_dist(0.0, 1.0);

        if (m_ptrConfig->m_epsilon >= real_dist(m_gen)) {
            // Make random move 
            // Determine range of non-greedy actions and number of elements in it
            typename T::const_iterator iBegin = i_variants.begin();
            typename T::const_iterator iEnd = i_variants.lower_bound((--i_variants.end())->first);
            typename T::size_type num = std::distance(iBegin, iEnd);
            // Check if we have non greedy actions
            if (!num) {
                return runGreedy(i_variants);
            }
            // Select non-greedy action
            std::uniform_int<> int_dist(0, static_cast<int>(num) - 1);
            std::advance(iBegin, int_dist(m_gen));

            return iBegin;
        } else {
            return runGreedy(i_variants);
        }
    }

    /** @brief Determine type of policy and policy constants */
    CConfigPtr            m_ptrConfig;
    /** @brief Random number generator */
    std::mt19937 m_gen;
};

}}
