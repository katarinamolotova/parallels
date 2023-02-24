#ifndef SRC_HELPERS_MATRIX_H
#define SRC_HELPERS_MATRIX_H

#include <cmath>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <random>
#include <stdexcept>

namespace s21 {
template <class T>
class Matrix {
 public:
  [[nodiscard]] int GetRows() const;
  [[nodiscard]] int GetCols() const;
  void SetRows(const int rows);
  void SetCols(const int cols);
  void ResizeMatrix(int rows, int cols);
  bool GetError() { return error_; };
  bool IsEqualMatrix(const Matrix &other);
  void FillMatrix(T value);
  void FillRandomMatrix();
  bool IsMatrixEmpty() { return rows_ == 0 && cols_ == 0; }
  void MulMatrix(const Matrix<T> &other);
  void DeleteMatrix();

  Matrix() = default;
  explicit Matrix(int size);
  Matrix(int rows, int cols);
  Matrix(const Matrix &other);
  Matrix(std::initializer_list<T> const &items);  // only for square matrix

  bool operator==(const Matrix &other);
  Matrix &operator=(const Matrix &other);
  T &operator()(int row, int col);
  Matrix<T> operator*(const Matrix<T> &other);

  ~Matrix();

 private:
  int rows_{}, cols_{};
  T **matrix_{};
  bool error_{false};

  void CreateMatrix();
  void CopyMatrix(Matrix const &other);
  bool IsEqualSize(const Matrix &other);
  void InitMatrix(std::initializer_list<T> const &items);
  T RandomGenerate_();
};
}  // namespace s21

#endif  // SRC_HELPERS_MATRIX_H
