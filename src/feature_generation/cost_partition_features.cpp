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
  
  //TODO: Adapt to PyTorch
  CostPartition CostPartitionFeatures::predict_cost_partition(const std::vector<std::shared_ptr<graph::Graph>> &graphs) {
  CostPartition cost_part(graphs.size(), std::vector<double>(actions.size()));
    // Compute cost partition vectors
    for (size_t i = 0; i < graphs.size(); i++) {
      // std::cout << "new graph" << std::endl;
      std::unordered_map<std::string, Embedding> action_emb = graph_and_actions_embed_impl(graphs[i], i);

      for (size_t j = 0; j < actions.size(); j++) {
        // std::cout << actions[j].name << std::endl;
        // if (action_emb.find(actions[j].name) != action_emb.end()) {
        std::vector<double> weights = get_action_schema_weights(actions[j].action_schema.name);
        cost_part[i][j] = std::inner_product(action_emb[actions[j].name].begin(), 
                                              action_emb[actions[j].name].end(),
                                              weights.begin(), 0.0);
        // std::cout << cost_part[i][j] << std::endl;
        // }
      }
    }

    // Apply softmax on the computed values
    for (size_t j = 0; j < actions.size(); j++) {
      std::vector<double> exp_vec;

      double max = std::numeric_limits<double>::min();
      for (size_t i = 0; i < graphs.size(); i++) {
        if (cost_part[i][j] > max) {
          max = cost_part[i][j];
        }
      }

      for (size_t i = 0; i < graphs.size(); i++) {
        exp_vec.push_back(std::exp(cost_part[i][j] - max));
      }

      double exp_sum = std::reduce(exp_vec.begin(), exp_vec.end());
      // std::cout << exp_sum << std::endl;

      for (size_t i = 0; i < graphs.size(); i++) {
        if (exp_sum > 0) cost_part[i][j] = exp_vec[i] / exp_sum;
        else cost_part[i][j] = 0;
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
    CostPartitionFeatures::graph_and_actions_embed_dataset(const data::GroundedDataset &dataset) {
    std::cout << "graph and actions" << std::endl;
    return _embed_dataset(dataset, EmbedType::GraphActions);
  }

  std::generator<std::unordered_map<std::string, std::vector<Embedding>>>
    CostPartitionFeatures::actions_embed_dataset(const data::GroundedDataset &dataset) {
    std::cout << "actions" << std::endl;
    return _embed_dataset(dataset, EmbedType::Actions);
  }

  std::generator<std::unordered_map<std::string, std::vector<Embedding>>>
  CostPartitionFeatures::_embed_dataset(const data::GroundedDataset &dataset, const EmbedType type) {

    std::vector<data::ProblemPatternsAssignments> data = dataset.get_data();

    for (size_t i = 0; i < data.size(); i++) {
      const auto &problem_states = data[i];
      const auto &problem = problem_states.problem;
      const auto &assignments = problem_states.assignments;
      const auto &patterns = problem_states.patterns;
      set_grounded_problem_and_pattern(problem, patterns);
      for (const planning::Assignment &assign : assignments) {
        co_yield _embed_assignment(assign, type);
      }
    }
  }

  std::unordered_map<std::string, std::vector<Embedding>>
  CostPartitionFeatures::_embed_assignment(const planning::Assignment &assignment, const EmbedType type) {
    std::vector<std::shared_ptr<graph::Graph>> graphs = graph_generator->to_graphs(assignment);

    return _embed_graphs(graphs, type);
  }

  std::unordered_map<std::string, std::vector<Embedding>>
  CostPartitionFeatures::_embed_graphs(const std::vector<std::shared_ptr<graph::Graph>> &graphs, const EmbedType type) {
    std::unordered_map<std::string, std::vector<Embedding>> ret;
    
    for (size_t i = 0; i < graphs.size(); i++) {
      const std::shared_ptr<graph::Graph> &graph = graphs[i];
      std::unordered_map<std::string, Embedding> embeds = _embed(graph, i, type);

      for (auto embed : embeds) {
        if (ret.find(embed.first) == ret.end()) {
          switch (type) {
            case EmbedType::GraphActions:
              ret[embed.first] = std::vector<Embedding>(graphs.size(), Embedding(get_n_features(), 0));
              break;
            case EmbedType::Actions:
              ret[embed.first] = std::vector<Embedding>(graphs.size(), Embedding(iterations, 0));
              break;
          }
        }
        ret[embed.first][i] = embed.second;
      }
    }

    return ret;
  }

  std::unordered_map<std::string, Embedding>
  CostPartitionFeatures::_embed(const std::shared_ptr<graph::Graph> &graph,
                                const int graph_id,
                                const EmbedType type) {
    collecting = false;
    if (!collected) {
      throw std::runtime_error("collect() must be called before embedding");
    }

    switch (type) {
      case EmbedType::GraphActions:
        return graph_and_actions_embed_impl(graph, graph_id);
        break;
      case EmbedType::Actions:
        return actions_embed_impl(graph, graph_id);
        break;
      default:
        throw std::runtime_error("Unknown embedding type");
    }
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
