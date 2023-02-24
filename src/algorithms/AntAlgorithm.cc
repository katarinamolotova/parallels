#include "AntAlgorithm.h"

namespace s21 {

AntAlgorithm::AntAlgorithm(const s21::Matrix<double> &graph, int count = 1)
    : graph_(graph), count_(count) {
  result_ = TsmResult({}, INT_MAX);
  size_ = graph.GetRows();
  pheromones_ = pheromones_delta_ = Matrix<double>(size_);
}

TsmResult AntAlgorithm::GetResult(bool isMultithreading = false) {
  error_ = CheckGraph_();
  if (error_ == GraphError::GRAPH_NORMAL || error_ == GraphError::GRAPH_DIRECT)
    PreparingForExecution_(isMultithreading);
  return result_;
}

GraphError AntAlgorithm::CheckGraph_() {
  if (graph_.GetRows() < 3) return GraphError::GRAPH_SMALL;
  for (auto row = 0; row < graph_.GetRows(); row++) {
    for (auto col = 0; col < graph_.GetCols(); col++) {
      if (row != col && graph_(row, col) == 0)
        return GraphError::GRAPH_INCOMPLETE;
      else if (graph_(row, col) != graph_(col, row))
        return GraphError::GRAPH_DIRECT;
    }
  }
  return GraphError::GRAPH_NORMAL;
}

void AntAlgorithm::PreparingForExecution_(bool isMultithreading) {
  SetStartingValueForPheromones_();
  if (isMultithreading) {
    thread th1(&AntAlgorithm::StartIteration_, this, 250);
    thread th2(&AntAlgorithm::StartIteration_, this, 250);
    thread th3(&AntAlgorithm::StartIteration_, this, 250);
    thread th4(&AntAlgorithm::StartIteration_, this, 250);
    th1.join();
    th2.join();
    th3.join();
    th4.join();
  } else {
    StartIteration_(1000);
  }
}

void AntAlgorithm::StartIteration_(int end) {
  for (int i = 0; i < count_; i++) {
    if (i > 0) UpdatePheromones_();
    AlgorithmExecution_(end);
  }
}

void AntAlgorithm::AlgorithmExecution_(int end) {
  for (int i = 0; i < end; i++) {
    for (auto ant = 0; ant < size_; ant++) {
      int position = 0;
      vector<int> visited;
      map<int, double> available_v_and_probability = GetAvailableVertices_();
      while (!available_v_and_probability.empty()) {
        visited.push_back(position);
        available_v_and_probability.erase(position);
        TransitionProbabilityCalculation_(available_v_and_probability,
                                          position);
        position = GetNextPosition_(available_v_and_probability);
      }
      IncreaseDelta_(visited);
      int cost = GetCostPath_(visited);
      if (result_.distance > cost) {
        result_.distance = cost;
        result_.vertices = GetRightVertices_(visited);
      }
    }
  }
}

void AntAlgorithm::SetStartingValueForPheromones_() {
  for (auto i = 0; i < size_; i++)
    for (auto j = 0; j < size_; j++)
      if (graph_(i, j) != 0) pheromones_(i, j) = 0.2;
}

map<int, double> AntAlgorithm::GetAvailableVertices_() {
  map<int, double> result;
  for (auto i = 0; i < size_; i++) result.insert({i, 0});
  return result;
}

void AntAlgorithm::TransitionProbabilityCalculation_(
    map<int, double> &probability, int position) {
  double sum = 0;
  for (std::pair<int, double> i : probability) {
    sum += pheromones_(position, i.first) * graph_(position, i.first);
  }
  for (std::pair<int, double> i : probability) {
    probability[i.first] =
        pheromones_(position, i.first) * graph_(position, i.first) / sum;
  }
}

int AntAlgorithm::GetNextPosition_(map<int, double> &probability) {
  double point = rand() % 100 / 100.0, sum = 0;
  int position = 0;
  for (std::pair<int, double> i : probability) {
    sum += i.second;
    if (point <= sum) {
      position = i.first;
      break;
    }
  }
  return position;
}

void AntAlgorithm::IncreaseDelta_(const vector<int> &visited) {
  const double q = 10.0;
  int prev_point = visited[0], sum = GetCostPath_(visited);
  mutex_.lock();
  for (unsigned long i = 1; i < visited.size(); i++) {
    pheromones_delta_(prev_point, visited[i]) += q / (double)sum;
  }
  mutex_.unlock();
}

void AntAlgorithm::UpdatePheromones_() {
  for (auto row = 0; row < size_; row++) {
    mutex_.lock();
    for (auto col = 0; col < size_; col++) {
      pheromones_(row, col) =
          pheromones_(row, col) * 0.64 + pheromones_delta_(row, col);
    }
    mutex_.unlock();
  }
  pheromones_delta_.FillMatrix(0.0);
}

vector<int> AntAlgorithm::GetRightVertices_(vector<int> vertices) {
  for (int &vertex : vertices) vertex++;
  vertices.push_back(vertices.front());
  return vertices;
}

int AntAlgorithm::GetCostPath_(const vector<int> &path) {
  int sum = 0, prev_point = path[0];
  for (size_t i = 1; i < path.size(); i++) {
    sum += graph_(prev_point, path[i]);
    prev_point = path[i];
  }
  sum += graph_(path.back(), path.front());
  return sum;
}

}  // namespace s21