#pragma once

#include "../config.hpp"

#include <random>
#include <map>
#include <algorithm>

namespace tcl { namespace rll {
namespace detail {

/// @brief Help to select actions.
/// @todo Use sorted vector.
class CPolicy
{
public:
    DEFINE_EXCEPTION(CPolicyException)

    CPolicy(const CConfigPtr& ptrConfig) : m_ptrConfig(ptrConfig) {}

    /// @brief Select element from map according to selected policy.
    /// T - should be map with key type as double
    template<class T>
    const std::pair<double, T>& select(const std::vector<std::pair<double, T> >& sortedVariants)
    {
        if (sortedVariants.empty())
            throw CPolicyException("Cannot select action from empty set");

        if (1 == sortedVariants.size())
            return sortedVariants.back();

        switch (m_ptrConfig->m_policy)
        {
            case CConfig::GREEDY: return runGreedy(sortedVariants);
            case CConfig::EPSILON_GREEDY: return runEGreedy(sortedVariants);
            default: return runEGreedy(sortedVariants);
        }
    }

private:
    template<class T>
    const std::pair<double, T>& runGreedy(
        const std::vector<std::pair<double, T> >& sortedVariants
      )
    {
        typedef const std::pair<double, T>& const_ref;
        typedef std::vector<std::pair<double, T> > variants_vector;

        typename variants_vector::const_iterator greedyLowerBoundHint = std::lower_bound(
            sortedVariants.begin()
          , sortedVariants.end()
          , sortedVariants.back()
          , [](const_ref r1, const_ref r2) -> bool
            {
                return r1.first < r2.first;
            }
          );

        return runGreedyWithHint(sortedVariants, greedyLowerBoundHint);
    }

    /// @brief Use greedy policy to determine next action.
    template<class T>
    const std::pair<double, T>& runGreedyWithHint(
        const std::vector<std::pair<double, T> >& sortedVariants
      , typename std::vector<std::pair<double, T> >::const_iterator greedyLowerBoundHint
      )
    {
        std::uniform_int_distribution<> int_dist(
            greedyLowerBoundHint - sortedVariants.begin()
          , sortedVariants.size() - 1
          );

        return sortedVariants[int_dist(m_gen)];
    }

    /// @brief Use e-greedy policy to determine next action.
    template<class T>
    const std::pair<double, T>& runEGreedy(const std::vector<std::pair<double, T> >& sortedVariants)
    {
        typedef const std::pair<double, T>& const_ref;

        std::uniform_real_distribution<> real_dist(0.0, 1.0);

        if (m_ptrConfig->m_epsilon >= real_dist(m_gen)) {
            // Make random move
            // Determine range of non-greedy actions and number of elements in it
            auto lb = std::lower_bound(
                sortedVariants.begin()
              , sortedVariants.end()
              , sortedVariants.back()
              , [](const_ref r1, const_ref r2) -> bool
                {
                    return r1.first < r2.first;
                }
              );

            size_t num = lb - sortedVariants.begin();

            // Check if we have non greedy actions
            if (!num) {
                return runGreedyWithHint(sortedVariants, lb);
            }

            // Select non-greedy action
            std::uniform_int_distribution<> int_dist(0, static_cast<int>(num) - 1);
            return sortedVariants[int_dist(m_gen)];
        } 
        else 
            return runGreedy(sortedVariants);
    }

    CConfigPtr   m_ptrConfig;  //!< Determine type of policy and policy constants
    std::mt19937 m_gen;        //!< Random number generator
};

}
}}
