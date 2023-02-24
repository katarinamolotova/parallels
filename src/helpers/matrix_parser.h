#ifndef SRC_HELPERS_MATRIX_PARSER_H
#define SRC_HELPERS_MATRIX_PARSER_H

#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>

#include "matrix.h"

namespace s21 {

class MatrixParser {
 public:
  MatrixParser();
  ~MatrixParser();
  Matrix<double> LoadMatrixFromFile(const std::string &filename);
  bool GetError() { return error_; }

 private:
  bool error_{};
  double ParsingValue_(int pos, std::string line);
  void GetSizeOfMatrix_(std::string line, int &rows, int &cols);
  int GetLengthDouble_(double num);
};
}  // namespace s21

#endif  // SRC_HELPERS_MATRIX_PARSER_H
