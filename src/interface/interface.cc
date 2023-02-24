#include "interface.h"

namespace s21 {
Interface::Interface() {
  base_matrix_ = Matrix<double>(3, 3);
#ifdef WINOGRADALGORITHM
  extra_matrix_for_winograd_ = Matrix<double>(3, 3);
#endif
}

Interface::~Interface() = default;

void s21::Interface::Start() {
  Message_(NUMBER_OF_REPEAT);
  error_ = InputOptions_(number_of_repeat_);

  if (!error_) {
#ifdef WINOGRADALGORITHM
    Message_(CHOSE_RANDOM);
    int option_temp;
    error_ = InputOptions_(option_temp);
    random_option = (bool)option_temp;
#endif
    if (random_option && !error_) {
      Message_(GIVE_ME_FILE);
      std::string file_address;
      std::cin >> file_address;
      std::ifstream file(file_address);
      if (file.is_open()) {
        base_matrix_ = MatrixParser().LoadMatrixFromFile(file_address);
      } else {
        error_ = true;
        Message_(WRONG_FILE);
      }
    }
#ifdef WINOGRADALGORITHM
    if (!random_option && !error_) {
      Message_("First matrix:\n");
      RandomMatrix_(base_matrix_);
      Message_("Second matrix:\n");
      RandomMatrix_(extra_matrix_for_winograd_);
    } else if (!error_) {
      Message_(GIVE_ME_FILE);
      std::string file_address;
      std::cin >> file_address;
      std::ifstream file(file_address);
      if (file.is_open()) {
        extra_matrix_for_winograd_ =
            MatrixParser().LoadMatrixFromFile(file_address);
      } else {
        error_ = true;
        Message_(WRONG_FILE);
      }
    }
    if (!error_) {
      SetNumberOfThreads();
      RunWinogradAlgorithm();
    }
#endif
#ifdef ANTALGORITHM
    if (!error_) RunAntAlgorithm();
#endif
#ifdef GAUSSALGORITHM
    if (!error_) GaussAlgorithm();
#endif
  }
  Message_(FINAL_MESSAGE);
}

#ifdef ANTALGORITHM
void Interface::RunAntAlgorithm() {
  AntAlgorithm algorithm(base_matrix_, number_of_repeat_);
  std::array<double, 2> result_time{};
  std::array<TsmResult, 2> result{};
  for (unsigned int i = 0; i < result.size(); ++i) {
    auto start_time = std::chrono::high_resolution_clock::now();
    result[i] = algorithm.GetResult(i);
    std::chrono::duration<double> duration =
        std::chrono::high_resolution_clock::now() - start_time;
    result_time[i] = duration.count();
  }
  GraphError error = algorithm.GetError();
  if (error == GraphError::GRAPH_NORMAL || error == GraphError::GRAPH_DIRECT)
    PrintAntResult(result_time, result);
  else
    Message_(WRONG_GRAPH);
}

void Interface::PrintAntResult(std::array<double, 2> &result_time,
                               std::array<TsmResult, 2> &result) {
  std::array<std::string, 2> sample = {"\nNo Parallels", "\nParallels"};
  for (unsigned int i = 0; i < sample.size(); ++i) {
    std::cout << sample[i] << std::endl;
    std::cout << "Distance " + std::to_string(result[i].distance) << std::endl
              << "Path: ";
    for (int j : result[i].vertices) {
      std::cout << j << " ";
    }
    std::cout << std::endl
              << "Time: " << std::to_string(result_time[i]) << " sec\n";
  }
}
#endif

#ifdef GAUSSALGORITHM
void Interface::GaussAlgorithm() {
  if (GaussAlgorithm::CheckGaussMatrix(base_matrix_)) {
    std::array<double, 2> times{};
    std::vector<double> result =
        GaussAlgorithm::GaussWithoutParallelism(base_matrix_);

    for (int t = 0; t < (int)times.max_size(); ++t) {
      auto start_time = std::chrono::high_resolution_clock::now();
      for (int i = 1; i < number_of_repeat_; ++i) {
        ChoseThreadMode(t);
      }
      std::chrono::duration<double> duration =
          std::chrono::high_resolution_clock::now() - start_time;
      times[t] = duration.count();
    }
    PrintResultGauss(result, times);
  } else {
    Message_("Error Wrong Matrix");
  }
}

void Interface::ChoseThreadMode(bool mode) {
  if (mode)
    GaussAlgorithm::GaussWithParallelism(base_matrix_);
  else
    GaussAlgorithm::GaussWithoutParallelism(base_matrix_);
}

void Interface::PrintResultGauss(const std::vector<double> &result,
                                 std::array<double, 2> times) {
  Message_("Your result\n");

  for (auto it : result) std::cout << it << " ";

  Message_("\nDuration without Parallelism\n Time: ");
  std::cout << times[0];
  Message_("\nDuration using Parallelism\n Time: ");
  std::cout << times[1];
}

#endif

#ifdef WINOGRADALGORITHM
void Interface::RunWinogradAlgorithm() {
  WinogradAlgorithm algorithm(base_matrix_, extra_matrix_for_winograd_,
                              number_of_repeat_);
  std::array<double, 3> result_time{};
  std::array<Matrix<double>, 3> result_matrix{};
  for (unsigned int i = 0; i < result_matrix.size(); ++i) {
    auto start_time = std::chrono::high_resolution_clock::now();
    result_matrix[i] = algorithm.GetResultMatrix(static_cast<ExecutionType>(i),
                                                 number_of_threads_);
    std::chrono::duration<double> duration =
        std::chrono::high_resolution_clock::now() - start_time;
    result_time[i] = duration.count();
  }
  if (!algorithm.GetError())
    PrintWinogradResult(result_time, result_matrix);
  else
    Message_(WRONG_MATRIX);
}

void Interface::PrintWinogradResult(std::array<double, 3> &result_time,
                                    std::array<Matrix<double>, 3> &result) {
  std::array<std::string, 3> sample = {
      "\nNo parallelism", "\nClassical parallelism", "\nPipeline parallelism"};
  for (unsigned int i = 0; i < sample.size(); ++i) {
    std::cout << sample[i] << std::endl;
    PrintMatrix_(result[i]);
    std::cout << std::endl
              << "Time: " << std::to_string(result_time[i]) << " sec\n";
  }
}
#endif

void Interface::PrintMatrix_(Matrix<double> matrix) {
  for (int i = 0; i < matrix.GetRows(); i++) {
    for (int j = 0; j < matrix.GetCols(); j++) {
      std::cout << matrix(i, j) << "\t";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void Interface::Message_(const std::string &message) { std::cout << message; }

bool Interface::ThisStringIsDigit_(const std::string &example) {
  bool status = true;
  for (char i : example)
    if (!isdigit(i)) {
      status = false;
      break;
    }
  return status;
}

bool Interface::InputOptions_(int &options) {
  bool status = false;
  std::string input_sample;
  std::cin >> input_sample;
  if (!input_sample.empty() && ThisStringIsDigit_(input_sample)) {
    options = std::stoi(input_sample);
  } else {
    status = true;
    Message_(ERROR_MESSAGE);
  }
  return status;
}
#ifdef WINOGRADALGORITHM
void Interface::RandomMatrix_(Matrix<double> &matrix) {
  int tmp_size = 0;
  Message_(SET_ROWS);
  InputOptions_(tmp_size);
  matrix.SetRows(tmp_size);
  Message_(SET_COLS);
  InputOptions_(tmp_size);
  matrix.SetCols(tmp_size);
  matrix.FillRandomMatrix();
}

void Interface::SetNumberOfThreads() {
  int tmp_max_threads = (int)std::thread::hardware_concurrency();
  Message_(NUMBER_OF_THREADS);
  error_ = InputOptions_(number_of_threads_);
  if (number_of_threads_ > tmp_max_threads || number_of_threads_ < 1)
    number_of_threads_ = tmp_max_threads;
  if (number_of_threads_ % 2 != 0) number_of_threads_++;
}

#endif
}  // namespace s21
