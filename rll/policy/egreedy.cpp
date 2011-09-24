#include "egreedy.hpp"

#include <algorithm>

namespace tcl { namespace rll { namespace policy {

egreedy::egreedy(double epsilon)
    : epsilon_(epsilon)
{
}

const egreedy::selection& egreedy::select(const variants& sorted_variants) 
{
    std::uniform_real_distribution<> real_dist(0.0, 1.0);

    if (epsilon_ >= real_dist(greedy_.gen_)) {
        // Make random move
        // Determine range of non-greedy actions and number of elements in it
        auto lb = std::lower_bound(
            sorted_variants.begin()
          , sorted_variants.end()
          , sorted_variants.back()
          , [](const selection& r1, const selection& r2) -> bool
            {
                return r1.first < r2.first;
            }
          );

        size_t num = lb - sorted_variants.begin();

        // Check if we have non greedy actions
        if (!num)
            return greedy_.select_with_hint(sorted_variants, lb);

        // Select non-greedy action
        std::uniform_int_distribution<> int_dist(0, static_cast<int>(num) - 1);
        return sorted_variants[int_dist(greedy_.gen_)];
    } 
    else 
        return greedy_.select(sorted_variants);
}

}}}
