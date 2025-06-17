#include "../../include/planning/abstract.hpp"

#include <algorithm>

namespace planning
{

  planning::Assignment filter_assignment(const planning::Assignment &assignment, const planning::Pattern &pattern) {
    planning::Assignment filtered;
    for (auto &var : assignment)
    {
      if (std::find(pattern.begin(), pattern.end(), var->index) != pattern.end()) {
        filtered.push_back(var);
      }
    }

    return filtered;
  }
    
} // namespace planning
