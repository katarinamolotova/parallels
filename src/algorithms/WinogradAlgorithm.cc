#include "WinogradAlgorithm.h"

namespace s21 {
WinogradAlgorithm::WinogradAlgorithm(const Matrix<double> &first,
                                     const Matrix<double> &second, int count)
    : first_matrix_(first), second_matrix_(second), count_(count) {
  half_cols_ = first_matrix_.GetCols() / 2;
}

Matrix<double> WinogradAlgorithm::GetResultMatrix(ExecutionType type,
                                                  int number_of_thread) {
  CheckMatrixSize_();
  if (!error_ && first_matrix_.GetCols() == 1)
    MulMatrixInOneColumn();
  else if (!error_)
    PreparingForExecution_(type, number_of_thread);
  return result_matrix_;
}

void WinogradAlgorithm::CheckMatrixSize_() {
  error_ = first_matrix_.GetCols() != second_matrix_.GetRows();
}

void WinogradAlgorithm::MulMatrixInOneColumn() {
  for (int i = 0; i < count_; i++) {
    result_matrix_ = first_matrix_ * second_matrix_;
  }
}

void WinogradAlgorithm::PreparingForExecution_(ExecutionType type,
                                               int number_of_thread) {
  row_factor_ = vector<double>(first_matrix_.GetRows());
  column_factor_ = vector<double>(second_matrix_.GetCols());
  result_matrix_ =
      Matrix<double>(first_matrix_.GetRows(), second_matrix_.GetCols());
  if (type == ExecutionType::WITHOUT_PARALLELISM) {
    AlgorithmExecutionFirstPart_(0, first_matrix_.GetRows(), 0,
                                 second_matrix_.GetCols());
    AlgorithmExecutionSecondPart_(0, first_matrix_.GetRows());
  } else if (type == ExecutionType::CLASSICAL_PARALLELISM) {
    ClassicalParallelismExecution(number_of_thread);
  } else if (type == ExecutionType::PIPELINED_PARALLELISM) {
    PipelineParallelismExecution();
  }
}

void WinogradAlgorithm::AlgorithmExecutionFirstPart_(int start_row, int end_row,
                                                     int start_col,
                                                     int end_col) {
  for (int i = 0; i < count_; i++) {
    CalculateRowFactor_(start_row, end_row);
    CalculateColumnFactor_(start_col, end_col);
  }
}

void WinogradAlgorithm::AlgorithmExecutionSecondPart_(int start_row,
                                                      int end_row) {
  for (int i = 0; i < count_; i++) {
    CalculateResultMatrix_(start_row, end_row);
    AddValueIfOdd_(start_row, end_row);
  }
}

//  check number of the threads in interface 2, 4, 6 ... 24
void WinogradAlgorithm::ClassicalParallelismExecution(int number_of_thread) {
  vector<thread> threads(number_of_thread);
  for (int i = 0; i < number_of_thread; i++) {
    threads[i] = thread(&WinogradAlgorithm::AlgorithmExecutionFirstPart_, this,
                        i * first_matrix_.GetRows() / number_of_thread,
                        (i + 1) * first_matrix_.GetRows() / number_of_thread,
                        i * second_matrix_.GetCols() / number_of_thread,
                        (i + 1) * second_matrix_.GetCols() / number_of_thread);
  }
  for (int i = 0; i < number_of_thread; i++) threads[i].join();

  for (int i = 0; i < number_of_thread; i++) {
    threads[i] = thread(&WinogradAlgorithm::AlgorithmExecutionSecondPart_, this,
                        i * first_matrix_.GetRows() / number_of_thread,
                        (i + 1) * first_matrix_.GetRows() / number_of_thread);
  }
  for (int i = 0; i < number_of_thread; i++) threads[i].join();
}

void WinogradAlgorithm::PipelineParallelismExecution() {
  thread th1(&WinogradAlgorithm::PipelineParallelismStageOne_, this);
  thread th2(&WinogradAlgorithm::PipelineParallelismStageTwo_, this);
  thread th3(&WinogradAlgorithm::PipelineParallelismStageThree_, this);
  thread th4(&WinogradAlgorithm::PipelineParallelismStageFour_, this);
  th1.join();
  th2.join();
  th3.join();
  th4.join();
}

void WinogradAlgorithm::CalculateRowFactor_(int start, int end) {
  for (int i = start; i < end; i++) {
    row_factor_[i] = first_matrix_(i, 0) * first_matrix_(i, 1);
    for (int j = 1; j < half_cols_; j++)
      row_factor_[i] += first_matrix_(i, 2 * j + 1) * first_matrix_(i, 2 * j);
  }
}

void WinogradAlgorithm::CalculateColumnFactor_(int start, int end) {
  for (int i = start; i < end; i++) {
    column_factor_[i] = second_matrix_(0, i) * second_matrix_(1, i);
    for (int j = 1; j < half_cols_; j++)
      column_factor_[i] +=
          second_matrix_(2 * j + 1, i) * second_matrix_(2 * j, i);
  }
}

void WinogradAlgorithm::CalculateResultMatrix_(int start, int end) {
  for (int i = start; i < end; i++) {
    for (int j = 0; j < second_matrix_.GetCols(); j++) {
      result_matrix_(i, j) = -row_factor_[i] - column_factor_[j];
      for (int k = 0; k < half_cols_; k++) {
        result_matrix_(i, j) +=
            (first_matrix_(i, 2 * k) + second_matrix_(2 * k + 1, j)) *
            (first_matrix_(i, 2 * k + 1) + second_matrix_(2 * k, j));
      }
    }
  }
}

void WinogradAlgorithm::AddValueIfOdd_(int start, int end) {
  int cols = first_matrix_.GetCols();
  if (half_cols_ * 2 != cols) {
    for (int i = start; i < end; i++)
      for (int j = 0; j < second_matrix_.GetCols(); j++)
        result_matrix_(i, j) +=
            first_matrix_(i, cols - 1) * second_matrix_(cols - 1, j);
  }
}

void WinogradAlgorithm::PipelineParallelismStageOne_() {
  for (int i = 0; i < count_; i++) {
    stage_one = false;
    mutex_one.lock();
    CalculateRowFactor_(0, first_matrix_.GetRows());
    mutex_one.unlock();
    stage_one = true;
    cv_one.notify_all();
  }
}

void WinogradAlgorithm::PipelineParallelismStageTwo_() {
  for (int i = 0; i < count_; i++) {
    stage_two = false;
    mutex_two.lock();
    CalculateColumnFactor_(0, second_matrix_.GetCols());
    mutex_two.unlock();
    stage_two = true;
    cv_two.notify_all();
  }
}

void WinogradAlgorithm::PipelineParallelismStageThree_() {
  for (int i = 0; i < count_; i++) {
    unique_lock<mutex> ul_one(mutex_one);
    unique_lock<mutex> ul_two(mutex_two);

    cv_one.wait(ul_one, [=]() { return stage_one; });
    cv_two.wait(ul_two, [=]() { return stage_two; });

    stage_three = false;
    mutex_three.lock();
    CalculateResultMatrix_(0, first_matrix_.GetRows());
    mutex_three.unlock();
    stage_three = true;

    ul_one.unlock();
    ul_two.unlock();
    cv_three.notify_all();
  }
}

void WinogradAlgorithm::PipelineParallelismStageFour_() {
  for (int i = 0; i < count_; i++) {
    unique_lock<mutex> ul_three(mutex_three);
    cv_three.wait(ul_three, [=]() { return stage_three; });
    AddValueIfOdd_(0, first_matrix_.GetRows());
    ul_three.unlock();
  }
}

}  // namespace s21
