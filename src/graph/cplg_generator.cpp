#include "../../include/graph/cplg_generator.hpp"

#define X(description, name) name,
char const *value_description_name[] = {CPLG_VALUE_NODE_COLOUR_DESCRIPTIONS};
#undef X

namespace graph {
  CPLGGenerator::CPLGGenerator(const planning::Domain &domain)
      : domain(domain),
        predicate_to_colour(domain.predicate_to_colour),
        action_schema_to_colour(domain.action_schema_to_colour) {
    // initialise initial node colours (variable and domains)
    colour_to_description.push_back("_VARIABLE_");

    // add predicate values colours
    for (size_t i = 0; i < domain.predicates.size(); i++) {
      for (size_t j = 0; j < (int) CPLGValueDescription::_LAST; j++) {
        colour_to_description.push_back(domain.predicates[i].name + value_description_name[j]);
      }
    }

    // add action schema colours
    for (size_t i = 0; i < domain.action_schemas.size(); i++) {
      colour_to_description.push_back(domain.action_schemas[i].name + "_ACTION_TYPE_");
    }
  }

  void CPLGGenerator::set_grounded_problem_and_pattern(
    const planning::GroundedProblem &problem,
    const planning::Patterns &patterns) {
    // setup structures
    this->problem = std::make_shared<planning::GroundedProblem>(problem);
    this->patterns = patterns;

    node_changed = std::vector<std::vector<std::string>>(patterns.size());
    node_changed_pred = std::vector<std::vector<int>>(patterns.size());
    goal_node_changed = std::vector<std::vector<std::string>>(patterns.size());
    goal_node_changed_pred = std::vector<std::vector<int>>(patterns.size());
    variable_value_to_predicate.clear();
    action_name_to_indexes.clear();
    base_graphs.clear();

    for (size_t i = 0; i < problem.get_variable_values_names().size(); i++) {
      variable_value_to_predicate.push_back(std::vector<planning::Predicate>());
      for (size_t j = 0; j < problem.get_variable_values_names()[i].size(); j++) {
        variable_value_to_predicate[i].push_back(
          get_value_predicate(problem.get_variable_values_names()[i][j])
        );
      }
    }
    
    // init action indexes
    for (auto &action : problem.get_actions()) {
      action_name_to_indexes[action.name] = std::vector<int>(patterns.size(), -1);
    }

    // create a graph for each pattern
    for (size_t pattern_index = 0; pattern_index < patterns.size(); pattern_index++) {

      std::cout << "New graph " << pattern_index << std::endl;
      const planning::Pattern &pattern = patterns[pattern_index];
      Graph graph = Graph(/*store_node_names=*/true);
      int colour = 0;

      // add variable nodes
      for (int i : pattern) {
        std::string node = problem.get_variable_names()[i];
        graph.add_node(node, colour);
      }

      // add domain nodes and var-val edges
      for (int i : pattern) {
        auto goal_variable_value = problem.get_goals_map().find(i);
        bool is_goal_variable = (goal_variable_value != problem.get_goals_map().end());

        for (size_t j = 0; j < problem.get_variable_values_names()[i].size(); j++) {
          std::string node = problem.get_variable_values_names()[i][j];
          // std::cout << "domain: " << node << std::endl;

          // std::cout << variable_value_to_predicate[i][j].name << std::endl;
          if (is_goal_variable && (goal_variable_value->second == (int)j)) {
            colour = value_colour(variable_value_to_predicate[i][j],
                                  CPLGValueDescription::UNREACHED_GOAL);
          } else {
            colour = value_colour(variable_value_to_predicate[i][j], 
                                  CPLGValueDescription::UNREACHED_VALUE);;
          }
          
          // std::cout << colour << std::endl;
          graph.add_node(node, colour);
          graph.add_edge(problem.get_variable_names()[i], (int)CPLGEdgeDescription::VARVAL, node);
          graph.add_edge(node, (int)CPLGEdgeDescription::VARVAL, problem.get_variable_names()[i]);
        }
      }

      // add actions
      // filter actions which have no effect in current abstraction pattern
      std::set<int> pattern_vars = std::set<int>(pattern.begin(), pattern.end());

      for (const planning::Action &action : problem.get_actions()) {
        std::set<int> effect_indexes = action.get_effects_indexes();

        std::set<int> effects_pattern;
        
        std::set_intersection(pattern_vars.begin(),
                              pattern_vars.end(),
                              effect_indexes.begin(),
                              effect_indexes.end(),
                              std::inserter(effects_pattern, effects_pattern.begin()));
        

        
        if (effects_pattern.size() > 0) {
          // std::cout << action.name << std::endl;
          // std::cout << action.action_schema.name << std::endl;
          colour = action_colour(action);
          std::string node = action.name;

          int action_index = graph.add_node(node, colour);
          action_name_to_indexes[node][pattern_index] = action_index;

          // add precondition edges
          for (const auto &precond : action.get_preconditions()) {
            if (pattern_vars.find(precond.first) != pattern_vars.end()) {
              // std::cout << precond.first << ":" << precond.second << std::endl;

              std::string val_node =
                  problem.get_variable_values_names()[precond.first][precond.second];
              // std::cout << "domain: " << val_node << std::endl;

              graph.add_edge(node, (int)CPLGEdgeDescription::PRECONDITION, val_node);
              graph.add_edge(val_node, (int)CPLGEdgeDescription::PRECONDITION, node);
            }
            }

            // add effect edges
            for (const auto &effect : action.get_effects()) {
              if (pattern_vars.find(effect.first) != pattern_vars.end()) {
                // std::cout << effect.first << ":" << effect.second << std::endl;

                std::string val_node = problem.get_variable_values_names()[effect.first][effect.second];
                // std::cout << "domain: " << val_node << std::endl;
                
                graph.add_edge(node, (int)CPLGEdgeDescription::EFFECT, val_node);
                graph.add_edge(val_node, (int)CPLGEdgeDescription::EFFECT, node);
              }
            }
        }
      }

      /* set pointer */
      graph.dump();
      base_graphs.push_back(std::make_shared<Graph>(graph));
    }
  }

  std::shared_ptr<Graph> CPLGGenerator::modify_graph_from_assignment(
                                                        const planning::Assignment &assignment,
                                                        const int &pattern_id,
                                                        const std::shared_ptr<Graph> graph,
                                                        bool store_changes) {
    
    if (store_changes) {
      node_changed[pattern_id] = std::vector<std::string>();
      node_changed_pred[pattern_id] = std::vector<int>();
      goal_node_changed[pattern_id] = std::vector<std::string>();
      goal_node_changed_pred[pattern_id] = std::vector<int>();
    }

    std::string domain_node_str;

    for (const auto &var : assignment) {
      // std::cout << var->name << std::endl;

      auto goal_variable_value = problem->get_goals_map().find(var->index);
      bool is_goal_variable = (goal_variable_value != problem->get_goals_map().end());

      domain_node_str = problem->get_variable_values_names()[var->index][var->value];

      // std::cout << variable_value_to_predicate[var->index][var->value].name << std::endl;
      int pred_idx =
          predicate_to_colour.at(variable_value_to_predicate[var->index][var->value].name);
      // std::cout << pred_idx << std::endl;

      if (is_goal_variable && goal_variable_value->second == var->value) {
        graph->change_node_colour(domain_node_str,
                                  value_colour(pred_idx, CPLGValueDescription::REACHED_GOAL));
        if (store_changes) {
          goal_node_changed[pattern_id].push_back(domain_node_str);
          goal_node_changed_pred[pattern_id].push_back(pred_idx);
        }
      } else {
        graph->change_node_colour(domain_node_str,
                                  value_colour(pred_idx, CPLGValueDescription::REACHED_VALUE));
        if (store_changes) {
          node_changed[pattern_id].push_back(domain_node_str);
          node_changed_pred[pattern_id].push_back(pred_idx);
        }
      }
    }

    return graph;
  }

  std::set<int> CPLGGenerator::get_action_indexes(const int graph_id) const {
    std::set<int> action_ids;

    for (auto &itr : action_name_to_indexes) {
      const std::vector<int> &ids = itr.second;
      if (ids[graph_id] > -1) {
        action_ids.insert(ids[graph_id]);
      }
    }
    
    return action_ids;
  }

  void CPLGGenerator::reset_graph() const {
    for (size_t i = 0; i < patterns.size(); i++)
    {
      for (size_t j = 0; j < goal_node_changed[i].size(); j++) {
        base_graphs[i]->change_node_colour(goal_node_changed[i][j],
                                           value_colour(goal_node_changed_pred[i][j], CPLGValueDescription::UNREACHED_GOAL));
      }

      for (size_t j = 0; j < node_changed[i].size(); j++) {
        base_graphs[i]->change_node_colour(node_changed[i][j],
                                           value_colour(node_changed_pred[i][j], CPLGValueDescription::UNREACHED_VALUE));
      }
    }
  }

  std::vector<std::shared_ptr<Graph>> CPLGGenerator::to_graphs(const planning::Assignment &assignment) {
    std::vector<std::shared_ptr<Graph>> graphs;
    for (size_t i = 0; i < patterns.size(); i++) {
      std::cout << "Change assignment for graph " << i << std::endl;
      std::shared_ptr<Graph> graph = std::make_shared<Graph>(*base_graphs[i]);
      planning::Assignment filtered = filter_assignment(assignment, patterns[i]);

      for (auto var : filtered) {
        std::cout << var->index << " (" << var->name << "): " << var->value << std::endl;
      }

      graph = modify_graph_from_assignment(filtered, i, graph, false);
      graph->dump();

      graphs.push_back(graph);
    }
    return graphs;
  }

  std::vector<std::shared_ptr<Graph>> CPLGGenerator::to_graphs_opt(const planning::Assignment &assignment) {
    for (size_t i = 0; i < patterns.size(); i++) {
        planning::Assignment filtered = filter_assignment(assignment, patterns[i]);

        base_graphs[i] = modify_graph_from_assignment(filtered, i, base_graphs[i], true);
    }
    return base_graphs;
  }

  int CPLGGenerator::get_n_edge_labels() const { return (int) CPLGEdgeDescription::_LAST; }
  int CPLGGenerator::get_n_graphs() const { return base_graphs.size(); }

  planning::Predicate CPLGGenerator::get_value_predicate(std::string value_name) const {
    // Remove "not " if negated atom
    if (value_name.size() > 4 && value_name.substr(0, 4).compare("not ") == 0) {
      value_name = value_name.substr(4);
    }

    // remove parenthesis and keep predicate name only
    value_name = value_name.substr(1, value_name.size() - 2);
    size_t end_pred = value_name.find(" ");
    if (end_pred != value_name.npos) {
      value_name = value_name.substr(0, end_pred);
    }

    // identify and return the correct predicate
    planning::Predicate predicate;
    for (size_t i = 0; i < domain.predicates.size(); i++) {
      if (domain.predicates[i].name == value_name) {
        predicate = domain.predicates[i];
        break;
      }
    }

    return predicate;
  }

  void CPLGGenerator::print_init_colours() const {
    std::cout << "Initial node colours:" << std::endl;
    for (size_t i = 0; i < colour_to_description.size(); i++) {
      std::cout << i << " -> " << colour_to_description[i] << std::endl;
    }
  }

  void CPLGGenerator::dump_graph() const {
    for (size_t i = 0; i < patterns.size(); i++) {
      std::cout << "Pattern" << std::to_string(i) << "[" << std::endl;
      base_graphs[i]->dump();
      std::cout << "]" << std::endl;
    }
  }
}  // namespace graph
