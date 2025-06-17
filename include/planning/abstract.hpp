#ifndef PLANNING_ABSTRACT_STATE_HPP
#define PLANNING_ABSTRACT_STATE_HPP

#include "variable.hpp"

#include <memory>
#include <vector>
#include <set>

namespace planning
{
  using Pattern = std::vector<int>;
  using Patterns = std::vector<Pattern>;
  using Assignment = std::vector<std::shared_ptr<planning::Variable>>;
}  // namespace planning

#endif