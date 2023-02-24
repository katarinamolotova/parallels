#ifndef SRC_INTERFACE_INTERFACE_H
#define SRC_INTERFACE_INTERFACE_H

#define GIVE_ME_FILE "Select the file:\n"
#define NUMBER_OF_REPEAT "Number of repeat:\n"
#define NUMBER_OF_THREADS "Number of threads:\n"
#define ERROR_MESSAGE "Error! This is not digit!\n"
#define WRONG_FILE "Error! Wring file!\n"
#define WRONG_GRAPH "Error! Graph can't be incomplete or smaller than 3x3!\n"
#define WRONG_MATRIX "Error! Wrong matrix size!\n"
#define FINAL_MESSAGE "\nBye!\n"
#define CHOSE_RANDOM "Give 0 if you want random Matrix:\n"
#define SET_ROWS "Set number of rows for random Matrix:\n"
#define SET_COLS "Set number of cols for random Matrix:\n"

#ifdef ANTALGORITHM
#include "../algorithms/AntAlgorithm.h"
#endif

#ifdef GAUSSALGORITHM
#include "../algorithms/GaussAlgorithm.h"
#endif

#ifdef WINOGRADALGORITHM
#include "../algorithms/WinogradAlgorithm.h"
#endif

#include <array>

#include "../helpers/matrix_parser.h"

namespace s21 {
class Interface {
 public:
  Interface();
  ~Interface();

  void Start();

 private:
  Matrix<double> base_matrix_;
  bool error_{};
  int number_of_repeat_{};
  bool random_option = true;

#ifdef WINOGRADALGORITHM
  Matrix<double> extra_matrix_for_winograd_;
  int number_of_threads_{};
#endif

  static void Message_(const std::string &message);
  static bool ThisStringIsDigit_(const std::string &example);
  static bool InputOptions_(int &options);
  static void PrintMatrix_(Matrix<double> matrix);

#ifdef ANTALGORITHM
  void RunAntAlgorithm();
  static void PrintAntResult(std::array<double, 2> &result_time,
                             std::array<TsmResult, 2> &result);
#endif

#ifdef GAUSSALGORITHM
  void GaussAlgorithm();
  void ChoseThreadMode(bool mode);
  static void PrintResultGauss(const std::vector<double> &result,
                               std::array<double, 2> times);
#endif

#ifdef WINOGRADALGORITHM
  void RunWinogradAlgorithm();
  static void PrintWinogradResult(std::array<double, 3> &result_time,
                                  std::array<Matrix<double>, 3> &result);
  static void RandomMatrix_(Matrix<double> &matrix);
  void SetNumberOfThreads();
#endif
};
}  // namespace s21

#endif  // SRC_INTERFACE_INTERFACE_H