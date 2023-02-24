#ifndef SRC_ALGORITHMS_ANTALGORITHM_H
#define SRC_ALGORITHMS_ANTALGORITHM_H

#include <climits>
#include <map>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#include "../helpers/matrix.h"

using std::map;
using std::thread;
using std::vector;

namespace s21 {

enum GraphError { GRAPH_INCOMPLETE, GRAPH_DIRECT, GRAPH_SMALL, GRAPH_NORMAL };

struct TsmResult {
  std::vector<int> vertices;
  double distance = 0;

  TsmResult() = default;

  TsmResult(std::vector<int> path, double distance) {
    this->distance = distance;
    this->vertices = std::move(path);
  }
};

class AntAlgorithm {
 public:
  explicit AntAlgorithm(const Matrix<double> &graph, int count);
  ~AntAlgorithm() = default;
  TsmResult GetResult(bool isMultithreading);
  GraphError GetError() { return error_; }

 private:
  TsmResult result_;
  Matrix<double> graph_;
  std::mutex mutex_;
  int size_, count_;
  Matrix<double> pheromones_;
  Matrix<double> pheromones_delta_;
  GraphError error_ = GraphError::GRAPH_NORMAL;

  GraphError CheckGraph_();
  void PreparingForExecution_(bool isMultithreading);
  void StartIteration_(int end);
  void AlgorithmExecution_(int end);
  void SetStartingValueForPheromones_();
  void TransitionProbabilityCalculation_(map<int, double> &probability,
                                         int position);
  int GetNextPosition_(map<int, double> &probability);
  void IncreaseDelta_(const vector<int> &visited);
  void UpdatePheromones_();
  map<int, double> GetAvailableVertices_();
  vector<int> GetRightVertices_(vector<int> vertices);
  int GetCostPath_(const vector<int> &path);
};
}  // namespace s21

#endif  //  SRC_ALGORITHMS_ANTALGORITHM_H
