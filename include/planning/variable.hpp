#ifndef PLANNING_VARIABLE_HPP
#define PLANNING_VARIABLE_HPP

#include <string>

namespace planning
{
  class Variable {
    public:
     const std::string name;
     const int index;
     int value;

     Variable(std::string name, int index) : name(name), index(index) {}
     Variable(std::string name, int index, int value) : name(name), index(index), value(value) {}

     bool operator==(const Variable &other) const { return name == other.name && index == other.index && value == other.value; }
  };
}  // namespace planning

#endif