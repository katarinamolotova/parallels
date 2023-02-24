#include "GaussAlgorithm.h"

namespace s21 {
int GaussAlgorithm::threads_in_level_;

std::vector<double> GaussAlgorithm::GaussWithoutParallelism(
    Matrix<double> &matrix) {
  std::vector<double> result(matrix.GetRows());
  if (CheckGaussMatrix(matrix)) {
    double tmp;
    for (int i = 0; i < matrix.GetRows(); ++i) {
      tmp = matrix(i, i);
      for (int j = matrix.GetRows(); j >= i; --j) {
        matrix(i, j) /= tmp;
      }

      for (int j = i + 1; j < matrix.GetRows(); ++j) {
        tmp = matrix(j, i);
        for (int k = matrix.GetRows(); k >= i; --k) {
          matrix(j, k) -= tmp * matrix(i, k);
        }
      }
    }

    result[matrix.GetRows() - 1] =
        matrix(matrix.GetRows() - 1, matrix.GetRows());

    for (int i = matrix.GetRows() - 2; i >= 0; --i) {
      result[i] = matrix(i, matrix.GetRows());
      for (int j = i + 1; j < matrix.GetRows(); ++j) {
        result[i] -= matrix(i, j) * result[j];
      }
    }
  }
  return result;
}

std::vector<double> GaussAlgorithm::GaussWithParallelism(
    Matrix<double> &matrix) {
  std::vector<double> result_;
  if (CheckGaussMatrix(matrix)) {
    threads_in_level_ = std::thread::hardware_concurrency() - 1;
    threads_in_level_ = matrix.GetCols() < threads_in_level_
                            ? matrix.GetCols()
                            : threads_in_level_;
    result_.reserve(matrix.GetRows());

    int rows = matrix.GetRows();

    for (int i = 0; i < rows; ++i) {
      DivideEquation(matrix, matrix(i, i), i);
      SubtractElementsInMatrix(matrix, i);
    }

    result_[rows - 1] = matrix(rows - 1, rows);
    EquateResultsToRightValues(matrix, result_);

    for (int i = rows - 2; i >= 0; --i) {
      SubtractCalculatedVariables(matrix, result_, i);
    }
  }
  return result_;
}

void GaussAlgorithm::DivideEquation(Matrix<double> &matrix, double matrix_elen,
                                    int i) {
  std::vector<std::thread> threads(threads_in_level_);
  for (int thread_id = 0; thread_id < threads_in_level_; ++thread_id) {
    threads[thread_id] = std::thread(DivideEquationCycle, std::ref(matrix),
                                     matrix_elen, i, thread_id);
  }
  JoinThreads(threads);
}

void GaussAlgorithm::DivideEquationCycle(Matrix<double> &matrix, double tmp,
                                         int i, int thread_id) {
  std::pair<std::vector<int>, std::vector<int>> start_and_end_indices =
      InitializeStartAndEndIndices(matrix.GetRows(), i - 1, false);

  for (int j = start_and_end_indices.first[thread_id];
       j > start_and_end_indices.second[thread_id]; --j) {
    matrix(i, j) /= tmp;
  }
}

void GaussAlgorithm::SubtractElementsInMatrix(Matrix<double> &matrix, int i) {
  std::vector<std::thread> threads(threads_in_level_);
  for (int thread_id = 0; thread_id < threads_in_level_; ++thread_id) {
    threads[thread_id] = std::thread(SubtractElementsInMatrixCycle,
                                     std::ref(matrix), i, thread_id);
  }
  JoinThreads(threads);
}

void GaussAlgorithm::SubtractElementsInMatrixCycle(Matrix<double> &matrix,
                                                   int i, int thread_id) {
  std::pair<std::vector<int>, std::vector<int>> start_and_end_indices =
      InitializeStartAndEndIndices(i + 1, matrix.GetRows(), true);

  for (int j = start_and_end_indices.first[thread_id];
       j < start_and_end_indices.second[thread_id]; ++j) {
    double tmp = matrix(j, i);
    for (int k = matrix.GetRows(); k >= i; --k) {
      matrix(j, k) -= tmp * matrix(i, k);
    }
  }
}

void GaussAlgorithm::EquateResultsToRightValues(Matrix<double> &matrix,
                                                std::vector<double> &result) {
  std::vector<std::thread> threads(threads_in_level_);
  for (int thread_id = 0; thread_id < threads_in_level_; ++thread_id) {
    threads[thread_id] =
        std::thread(EquateResultsToRightValuesCycle, std::ref(matrix),
                    std::ref(result), thread_id);
  }
  JoinThreads(threads);
}

void GaussAlgorithm::EquateResultsToRightValuesCycle(
    Matrix<double> &matrix, std::vector<double> &result, int thread_id) {
  std::pair<std::vector<int>, std::vector<int>> start_and_end_indices =
      InitializeStartAndEndIndices(matrix.GetRows() - 2, -1, false);
  for (int i = start_and_end_indices.first[thread_id];
       i > start_and_end_indices.second[thread_id]; --i) {
    result[i] = matrix(i, matrix.GetRows());
  }
}

void GaussAlgorithm::SubtractCalculatedVariables(Matrix<double> &matrix,
                                                 std::vector<double> &result,
                                                 int i) {
  std::vector<std::thread> threads(threads_in_level_);
  std::mutex mtx;
  for (int thread_id = 0; thread_id < threads_in_level_; ++thread_id) {
    threads[thread_id] =
        std::thread(SubtractCalculatedVariablesCycle, std::ref(matrix),
                    std::ref(result), i, thread_id, std::ref(mtx));
  }
  JoinThreads(threads);
}

void GaussAlgorithm::SubtractCalculatedVariablesCycle(
    Matrix<double> &matrix, std::vector<double> &result, int i, int thread_id,
    std::mutex &mtx) {
  std::pair<std::vector<int>, std::vector<int>> start_and_end_indices =
      InitializeStartAndEndIndices(i + 1, matrix.GetRows(), true);

  for (int j = start_and_end_indices.first[thread_id];
       j < start_and_end_indices.second[thread_id]; ++j) {
    double calculated = matrix(i, j) * result[j];
    mtx.lock();
    result[i] -= calculated;
    mtx.unlock();
  }
}

std::pair<std::vector<int>, std::vector<int>>
GaussAlgorithm::InitializeStartAndEndIndices(int start_index, int end_index,
                                             bool start_is_less_than_end) {
  std::vector<int> start_indices(threads_in_level_);
  std::vector<int> end_indices(threads_in_level_);
  start_indices[0] = start_index;
  end_indices[threads_in_level_ - 1] = end_index;
  for (int i = 1; i < threads_in_level_; ++i) {
    if (start_is_less_than_end) {
      end_indices[i - 1] = start_indices[i] =
          start_indices[i - 1] +
          (double)(end_index - start_index) / threads_in_level_;
    } else {
      end_indices[i - 1] = start_indices[i] =
          start_indices[i - 1] -
          (double)(start_index - end_index) / threads_in_level_;
    }
  }
  return {start_indices, end_indices};
}

void GaussAlgorithm::JoinThreads(std::vector<std::thread> &threads) {
  for (int i = 0; i < threads_in_level_; ++i) {
    threads[i].join();
  }
}

bool GaussAlgorithm::CheckGaussMatrix(const Matrix<double> &matrix) {
  return (matrix.GetRows() >= 2 && matrix.GetCols() == matrix.GetRows() + 1);
}
}  // namespace s21