#ifndef SRC_ALGORITHMS_GAUSSALGORITHM_H
#define SRC_ALGORITHMS_GAUSSALGORITHM_H

#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "../helpers/matrix.h"

using std::thread;
using std::vector;

namespace s21 {
class GaussAlgorithm {
 public:
  static std::vector<double> GaussWithoutParallelism(Matrix<double> &matrix);
  static std::vector<double> GaussWithParallelism(Matrix<double> &matrix);
  static bool CheckGaussMatrix(const Matrix<double> &matrix);

 private:
  static int threads_in_level_;

  static void DivideEquation(Matrix<double> &matrix, double matrix_elen, int i);
  static void DivideEquationCycle(Matrix<double> &matrix, double tmp, int i,
                                  int thread_id);
  static void SubtractElementsInMatrix(Matrix<double> &matrix, int i);
  static void SubtractElementsInMatrixCycle(Matrix<double> &matrix, int i,
                                            int thread_id);
  static void EquateResultsToRightValues(Matrix<double> &matrix,
                                         std::vector<double> &result);
  static void EquateResultsToRightValuesCycle(Matrix<double> &matrix,
                                              std::vector<double> &result,
                                              int thread_id);
  static void SubtractCalculatedVariables(Matrix<double> &matrix,
                                          std::vector<double> &result, int i);
  static void SubtractCalculatedVariablesCycle(Matrix<double> &matrix,
                                               std::vector<double> &result,
                                               int i, int thread_id,
                                               std::mutex &mtx);
  static std::pair<std::vector<int>, std::vector<int>>
  InitializeStartAndEndIndices(int start_index, int end_index,
                               bool start_is_less_than_end);
  static void JoinThreads(std::vector<std::thread> &threads);
};
}  // namespace s21

#endif  // SRC_ALGORITHMS_GAUSSALGORITHM_H
