#include "../../include/feature_generation/cost_partition_features.hpp"

#include <math.h>
#include <numeric>
#include <coroutine>

namespace feature_generation {

  CostPartitionFeatures::CostPartitionFeatures(
                  const std::string feature_name,
                  const planning::Domain &domain,
                  std::string graph_representation,
                  int iterations,
                  std::string pruning,
                  bool multiset_hash,
                  PredictionTask task) : Features(feature_name, domain, graph_representation,
                                                  iterations, pruning, multiset_hash, task) {}

  CostPartitionFeatures::CostPartitionFeatures(const std::string &filename) : Features(filename) {}
  
  CostPartition CostPartitionFeatures::predict_cost_partition(const std::vector<std::shared_ptr<graph::Graph>> &graphs) {
    CostPartition cost_part(graphs.size(), std::vector<double>(actions.size(), 0));

    // Compute cost partition vectors
    for (size_t i = 0; i < graphs.size(); i++) {
      std::unordered_map<std::string, Embedding> action_emb = actions_embed_impl(graphs[i], i);

      for (size_t j = 0; j < actions.size(); j++) {
        if (action_emb.find(actions[j].name) != action_emb.end()) {
          std::vector<double> weights = get_action_schema_weights(actions[j].action_schema.name);
          cost_part[i][j] = std::inner_product(action_emb[actions[j].name].begin(), 
                                               action_emb[actions[j].name].end(),
                                               weights.begin(), 0.0);
        }
      }
    }

    // Apply softmax on the computed values
    for (size_t j = 0; j < actions.size(); j++) {
      std::vector<double> exp_vec;

      for (size_t i = 0; i < graphs.size(); i++) {
        if (cost_part[i][j] != 0)
          exp_vec.push_back(std::exp(cost_part[i][j]));
      }

      double exp_sum = std::reduce(exp_vec.begin(), exp_vec.end());

      int exp_index = 0;
      for (size_t i = 0; i < graphs.size(); i++) {
        if (cost_part[i][j] > 0){
          cost_part[i][j] = exp_vec[exp_index] / exp_sum;
          exp_index++;
        }
      }
    }

    return cost_part;
  }

  CostPartition CostPartitionFeatures::predict_cost_partition(const planning::Assignment &assignment) {
    if (graph_generator == nullptr || graph_representation != "cplg") {
      throw std::runtime_error("Graph generator is not correctly set. CPLGGenerator must be used for this task.");
    }

    std::vector<std::shared_ptr<graph::Graph>> graphs = graph_generator->to_graphs_opt(assignment);

    CostPartition cost_partition = predict_cost_partition(graphs);

    graph_generator->reset_graph();

    return cost_partition;
  }

  std::generator<std::unordered_map<std::string, std::vector<Embedding>>>
  CostPartitionFeatures::actions_embed_dataset(const data::GroundedDataset &dataset) {

    std::vector<data::ProblemPatternsAssignments> data = dataset.get_data();

    for (size_t i = 0; i < data.size(); i++) {
      const auto &problem_states = data[i];
      const auto &problem = problem_states.problem;
      const auto &assignments = problem_states.assignments;
      const auto &patterns = problem_states.patterns;
      set_grounded_problem_and_pattern(problem, patterns);
      for (const planning::Assignment &assign : assignments) {
        co_yield actions_embed_assignment(assign);
      }
    }
  }

  std::unordered_map<std::string, std::vector<Embedding>>
  CostPartitionFeatures::actions_embed_assignment(const planning::Assignment &assignment) {
    std::vector<std::shared_ptr<graph::Graph>> graphs = graph_generator->to_graphs(assignment);

    return actions_embed_graphs(graphs);
  }

  std::unordered_map<std::string, std::vector<Embedding>>
  CostPartitionFeatures::actions_embed_graphs(const std::vector<std::shared_ptr<graph::Graph>> &graphs) {
    std::unordered_map<std::string, std::vector<Embedding>> ret;
    
    for (size_t i = 0; i < graphs.size(); i++) {
      const std::shared_ptr<graph::Graph> &graph = graphs[i];
      std::unordered_map<std::string, Embedding> embeds = actions_embed(graph, i);

      for (auto embed : embeds) {
        if (ret.find(embed.first) == ret.end()) {
          ret[embed.first] = std::vector<Embedding>(graphs.size(), Embedding(get_n_features() + iterations, 0));
        }
        ret[embed.first][i] = embed.second;
      }
    }

    return ret;
  }

  std::unordered_map<std::string, Embedding>
  CostPartitionFeatures::actions_embed_graph(const graph::Graph &graph, const int graph_id) {
    return actions_embed(std::make_shared<graph::Graph>(graph), graph_id);
  }

  std::unordered_map<std::string, Embedding>
  CostPartitionFeatures::actions_embed(const std::shared_ptr<graph::Graph> &graph,
                                       const int graph_id) {
    collecting = false;
    if (!collected) {
      throw std::runtime_error("collect() must be called before embedding");
    }

    return actions_embed_impl(graph, graph_id);
  }

  void CostPartitionFeatures::set_grounded_problem_and_pattern(
    const planning::GroundedProblem &problem, 
    const planning::Patterns &patterns) {
    if (graph_generator != nullptr && task == PredictionTask::COST_PARTITIONING) {
      graph_generator->set_grounded_problem_and_pattern(problem, patterns);
    }
    this->actions = problem.get_actions();
  }

}  // namespace feature_generation
