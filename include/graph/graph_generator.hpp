#ifndef GRAPH_GRAPH_GENERATOR_HPP
#define GRAPH_GRAPH_GENERATOR_HPP

#include "../planning/atom.hpp"
#include "../planning/domain.hpp"
#include "../planning/problem.hpp"
#include "../planning/grounded_problem.hpp"
#include "../planning/abstract.hpp"
#include "../planning/state.hpp"
#include "graph.hpp"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace graph {
  class GraphGenerator {
   public:
    // Change the base graph based on the input problem
    virtual void set_problem(const planning::Problem &problem) = 0;

    // Set the number of graphs and their layout with a grounded problem and patterns
    virtual void set_grounded_problem_and_pattern(const planning::GroundedProblem &problem,
                                                  const planning::Patterns &patterns) = 0;

    virtual ~GraphGenerator() = default;

    // Makes a copy of the base graph and makes the necessary modifications
    // Assumes the state is from the problem that is set but does not check this.
    virtual std::shared_ptr<Graph> to_graph(const planning::State &state) = 0;

    virtual std::shared_ptr<Graph> to_graph_opt(const planning::State &state) = 0;

    // Makes a copy of the abstract graphs and makes the necessary modifications
    // Assumes the state is from the problem that is set but does not check this.
    virtual std::vector<std::shared_ptr<Graph>> to_graphs(const planning::Assignment &assignment) = 0;

    virtual std::vector<std::shared_ptr<Graph>> to_graphs_opt(const planning::Assignment &assignment) = 0;

    virtual std::set<int> get_action_indexes(const int graph_id) const = 0;
    virtual std::unordered_map<std::string, std::vector<int>> get_action_name_to_indexes() const = 0;

    virtual void reset_graph() const = 0;

    virtual int get_n_edge_labels() const = 0;
    virtual int get_n_graphs() const = 0;

    virtual void print_init_colours() const = 0;

    virtual void dump_graph() const = 0;
  };

}  // namespace graph

#endif  // GRAPH_GRAPH_GENERATOR_HPP
