#ifndef GRAPH_CPLG_GENERATOR_HPP
#define GRAPH_CPLG_GENERATOR_HPP

#include "../planning/atom.hpp"
#include "../planning/domain.hpp"
#include "../planning/problem.hpp"
#include "../planning/state.hpp"
#include "graph.hpp"
#include "graph_generator.hpp"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define CPLG_VALUE_NODE_COLOUR_DESCRIPTIONS                                                                   \
  X(REACHED_GOAL, "_REACHED_GOAL_")                                                                           \
  X(UNREACHED_GOAL, "_UNREACHED_GOAL_")                                                                       \
  X(REACHED_VALUE, "_REACHED_VALUE_")                                                                         \
  X(UNREACHED_VALUE, "_UNREACHED_VALUE_")                                                                     \
  X(_LAST, "_size_of_the_enum")

#define CPLG_EDGE_COLOUR_DESCRIPTIONS                                                                         \
  X(VARVAL, "_VARVAL_EDGE_")                                                                                  \
  X(PRECONDITION, "_PRECONDITION_EDGE_")                                                                      \
  X(EFFECT, "_EFFECT_EDGE_")                                                                                  \
  X(_LAST, "_size_of_the_enum")                                                                      

#define X(description, name) description,
enum class CPLGValueDescription { CPLG_VALUE_NODE_COLOUR_DESCRIPTIONS };
enum class CPLGEdgeDescription { CPLG_EDGE_COLOUR_DESCRIPTIONS };
#undef X


namespace graph {


  class CPLGGenerator : public GraphGenerator {
   public:
    CPLGGenerator(const planning::Domain &domain);

    // Change the base graph based on the input problem
    void set_grounded_problem_and_pattern(const planning::GroundedProblem &problem,
                                          const planning::Patterns &patterns) override;

    // Not implemented
    virtual void set_problem(const planning::Problem &problem) override { (void)problem; };

    // Makes a copy of the base graphs and makes the necessary modifications
    // Assumes the assignment is from the problem that is set but does not check this.
    std::vector<std::shared_ptr<Graph>> to_graphs(const planning::Assignment &assignment);

    // Optimised variant of to_graph() but requires calling reset_graph() after.
    // Does not make a copy of the base graphs and instead modifies them directly,
    // and undoing the modifications with reset_graph().
    std::vector<std::shared_ptr<Graph>> to_graphs_opt(const planning::Assignment &assignment);


    // Not implemented
    virtual std::shared_ptr<Graph> to_graph(const planning::State &state) override{
      (void)state;
      return std::make_shared<Graph>(false);
    };

    virtual std::shared_ptr<Graph> to_graph_opt(const planning::State &state) override { 
      return to_graph(state); 
    };

    std::set<int> get_action_indexes(const int graph_id) const override;
    std::unordered_map<std::string, std::vector<int>> get_action_name_to_indexes() const override { return action_name_to_indexes; }

    void reset_graph() const;

    int get_n_edge_labels() const override;
    int get_n_graphs() const override;

    void print_init_colours() const override;

    void dump_graph() const override;

   protected:
    /* The following variables remain constant for all problems */
    const planning::Domain &domain;
    const std::unordered_map<std::string, int> predicate_to_colour;
    const std::unordered_map<std::string, int> action_schema_to_colour;

    /* These variables get reset every time a new problem is set */
    std::vector<std::shared_ptr<Graph>> base_graphs;
    std::unordered_map<std::string, std::vector<int>> action_name_to_indexes;
    std::unordered_set<std::string> goal_names;
    std::shared_ptr<planning::GroundedProblem> problem;
    planning::Patterns patterns;

    std::vector<std::string> colour_to_description;
    int value_colour(const int predicate_idx,
                     const CPLGValueDescription &value_description) const;
    int value_colour(const planning::Predicate predicate,
                     const CPLGValueDescription &value_description) const;
    int action_colour(const int predicate_idx) const;
    int action_colour(const planning::Action&action) const;

    std::vector<std::vector<planning::Predicate>> variable_value_to_predicate;
    planning::Predicate get_value_predicate(std::string value_name) const;

    /* For modifying the base graph and redoing its changes */
    std::vector<std::vector<std::string>> node_changed;
    std::vector<std::vector<int>> node_changed_pred;
    std::vector<std::vector<std::string>> goal_node_changed;
    std::vector<std::vector<int>> goal_node_changed_pred;
    std::shared_ptr<Graph> modify_graph_from_assignment(const planning::Assignment &assignment,
                                                        const int &pattern_id,
                                                        const std::shared_ptr<Graph> graph,
                                                        bool store_changes);
  };

  inline int CPLGGenerator::value_colour(const int predicate_idx,
                                         const CPLGValueDescription &value_description) const {
    int pred_idx = predicate_idx * ((int)CPLGValueDescription::_LAST);
    return 1 + pred_idx + ((int) value_description);
  }

  inline int CPLGGenerator::value_colour(const planning::Predicate predicate,
                                         const CPLGValueDescription &value_description) const {
    return value_colour(predicate_to_colour.at(predicate.name), value_description);
  }

  inline int CPLGGenerator::action_colour(const int action_schema_idx) const {
    int predicate_shift = domain.predicates.size() * ((int)CPLGValueDescription::_LAST);
    return 1 + action_schema_idx + predicate_shift;
  }

  inline int CPLGGenerator::action_colour(const planning::Action &action) const {
    return action_colour(action_schema_to_colour.at(action.action_schema.name));
  }
}  // namespace graph

#endif  // GRAPH_CPLG_GENERATOR_HPP
