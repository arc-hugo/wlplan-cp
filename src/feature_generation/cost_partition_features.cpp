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
  
  std::generator<std::unordered_map<std::string, std::vector<Embedding>>>
    CostPartitionFeatures::embed_dataset(const data::GroundedDataset &dataset) {

    std::vector<data::ProblemPatternsAssignments> data = dataset.get_data();

    for (size_t i = 0; i < data.size(); i++) {
      const auto &problem_states = data[i];
      const auto &problem = problem_states.problem;
      const auto &assignments = problem_states.assignments;
      const auto &patterns = problem_states.patterns;
      set_grounded_problem_and_pattern(problem, patterns);
      for (const planning::Assignment &assign : assignments) {
        co_yield _embed_assign(assign);
      }
    }
  }

  Embedding CostPartitionFeatures::get_flattened_embeddings(
      const planning::Assignment &assignment,
      const std::vector<std::string> &operators) {
    std::unordered_map<std::string, std::vector<Embedding>> embeds = _embed_assign(assignment);

    Embedding ops_embeds;
    for (std::string op : operators) {
      const std::vector<Embedding> op_embed = embeds[op];

      for (Embedding embd : op_embed) {
        ops_embeds.insert(ops_embeds.end(), embd.begin(), embd.end());
      }
    }

    return ops_embeds;
  }

  std::unordered_map<std::string, std::vector<Embedding>>
  CostPartitionFeatures::_embed_assign(const planning::Assignment &assignment) {
    std::vector<std::shared_ptr<graph::Graph>> graphs = graph_generator->to_graphs(assignment);

    return _embed_graphs(graphs);
  }

  std::unordered_map<std::string, std::vector<Embedding>>
  CostPartitionFeatures::_embed_graphs(const std::vector<std::shared_ptr<graph::Graph>> &graphs) {
    std::unordered_map<std::string, std::vector<Embedding>> ret;
    
    for (size_t i = 0; i < graphs.size(); i++) {
      const std::shared_ptr<graph::Graph> &graph = graphs[i];
      std::unordered_map<std::string, Embedding> embeds = _embed(graph, i);

      for (auto embeded : embeds) {
        if (ret.find(embeded.first) == ret.end()) {
          ret[embeded.first] = std::vector<Embedding>(graphs.size(), Embedding(get_n_features(), 0));
        }
        ret[embeded.first][i] = embeded.second;
      }
    }

    return ret;
  }

  std::unordered_map<std::string, Embedding>
  CostPartitionFeatures::_embed(const std::shared_ptr<graph::Graph> &graph,
                                const int graph_id) {
    collecting = false;
    if (!collected) {
      throw std::runtime_error("collect() must be called before embedding");
    }

    return subgraph_embed_impl(graph, graph_id);
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
