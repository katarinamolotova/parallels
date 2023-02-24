#ifndef SRC_ALGORITHMS_WINOGRADALGORITHM_H
#define SRC_ALGORITHMS_WINOGRADALGORITHM_H

#include <mutex>
#include <thread>
#include <vector>

#include "../helpers/matrix.h"

using std::condition_variable;
using std::mutex;
using std::thread;
using std::unique_lock;
using std::vector;

namespace s21 {

enum ExecutionType {
  WITHOUT_PARALLELISM,
  CLASSICAL_PARALLELISM,
  PIPELINED_PARALLELISM
};

class WinogradAlgorithm {
 public:
  WinogradAlgorithm(const Matrix<double> &, const Matrix<double> &,
                    int count = 1);
  ~WinogradAlgorithm() = default;

  Matrix<double> GetResultMatrix(ExecutionType type, int number_of_thread = 1);
  bool GetError() { return error_; }

 private:
  Matrix<double> first_matrix_;
  Matrix<double> second_matrix_;
  Matrix<double> result_matrix_;

  vector<double> row_factor_;
  vector<double> column_factor_;

  int count_;
  int half_cols_;
  bool error_ = false;

  void PreparingForExecution_(ExecutionType type, int number_of_thread);
  void ClassicalParallelismExecution(int number_of_thread);
  void PipelineParallelismExecution();
  void AlgorithmExecutionFirstPart_(int start_row, int end_row, int start_col,
                                    int end_col);
  void AlgorithmExecutionSecondPart_(int start_row, int end_row);
  void CheckMatrixSize_();
  void MulMatrixInOneColumn();
  void CalculateRowFactor_(int start, int end);
  void CalculateColumnFactor_(int start, int end);
  void CalculateResultMatrix_(int start, int end);
  void AddValueIfOdd_(int start, int end);

  bool stage_one = false, stage_two = false, stage_three = false;
  mutex mutex_one, mutex_two, mutex_three;
  condition_variable cv_one, cv_two, cv_three;
  void PipelineParallelismStageOne_();
  void PipelineParallelismStageTwo_();
  void PipelineParallelismStageThree_();
  void PipelineParallelismStageFour_();
};

}  // namespace s21

#endif  //  SRC_ALGORITHMS_WINOGRADALGORITHM_H
