#include "matrix_parser.h"

namespace s21 {
MatrixParser::MatrixParser() = default;
MatrixParser::~MatrixParser() = default;

Matrix<double> MatrixParser::LoadMatrixFromFile(const std::string &filename) {
  Matrix<double> tmp_matrix_;
  std::ifstream file(filename);
  if (tmp_matrix_.GetRows() != 0) tmp_matrix_.DeleteMatrix();
  if (file.is_open()) {
    error_ = false;
    std::string line;
    int row = 0, rows = 0, cols = 0;
    bool flag = true;
    while (getline(file, line) && (row < tmp_matrix_.GetRows() || flag) &&
           !error_) {
      if (flag) {
        flag = false;
        GetSizeOfMatrix_(line, rows, cols);
        tmp_matrix_ = Matrix<double>(rows, cols);
      } else if (line.length() > 1) {
        int pos = 0;
        for (int col = 0; col < tmp_matrix_.GetCols(); ++col) {
          if (line[pos] == ' ' || line[pos] == ',') {
            pos++;
            col--;
          } else {
            double num = ParsingValue_(pos, line);
            tmp_matrix_(row, col) = num;
            pos += GetLengthDouble_(num) + 1;
          }
        }
        row++;
      }
    }
    file.close();
  } else {
    error_ = true;
  }
  return tmp_matrix_;
}

double MatrixParser::ParsingValue_(int pos, std::string line) {
  double result = 0;
  try {
    result = (pos >= (int)line.length()) ? 0 : std::stod(&line[pos], nullptr);
  } catch (std::invalid_argument &e) {
    result = 0;
    error_ = true;
  }
  return result;
}

void MatrixParser::GetSizeOfMatrix_(std::string line, int &rows, int &cols) {
  try {
    rows = std::stoi(line);
    size_t pos = std::to_string(rows).length() + 1;
    cols = (pos > line.length()) ? rows : std::stoi(&line[pos]);
  } catch (std::invalid_argument &e) {
    rows = cols = 0;
    error_ = true;
  }
}

int MatrixParser::GetLengthDouble_(double num) {
  std::string temp = std::to_string(num);
  if (temp[temp.size() - 1] == '0')
    for (size_t i = temp.size() - 1; temp[i] == '0'; i--) temp.erase(i, 1);
  if (temp[temp.size() - 1] == '.') temp.erase(temp.size() - 1, 1);
  return temp.size();
}
}  // namespace s21
