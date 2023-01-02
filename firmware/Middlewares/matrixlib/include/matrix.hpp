/*
  Matrix.h - A free library for matrix calculation. Invalid dimension for
  calculation will lead to empty matrix, therefore, one should check whether the
  matrix is empty after calculation.

  This library is easy to use, I have overloaded operators, one can use +,*,/,-,
  =,==,!= as they want, but there is no definition for matrix division (like
  A/B). One should use the inverse function inv() instead.

  The transpose() and inv() functions are static functions, which should use
  Matrix<Type>::transpose() and Matrix<Type>::inv() format to call, Type is the
  type you used to create the matrix.

  The elements of the matrix are easy to access. For example, we have a 2 by 1
  vector called A, one could use A[1][0] to get the second element or A[1][0] to
  assign a value to it.

  The show(int decimal) function can be used to visulise the matrix, where
  decimal specifies the decimal numbers.

  Detailed description: https://playground.arduino.cc/Code/Matrix.

  Feedback and contribution is welcome!

  Version 1.3
  * Overloading operator [], so we can access the element by using A[][].
  Because the internal data structure is a two dimensional array, we have to use
  A[*][0] or A[0][*] to access the element in a column or row matrix.

  Version 1.2
  * Now it can perform mixed calculation with scalars.
    But you need to ensure the dimension of the matrix is 1 for plus and minus.

  Version 1.1
  * Improved the processing speed, it becomes more efficient.

  ----------------
  Version 1.0

  Created by Yudi Ren, Jan 05, 2018.
  renyudicn@outlook.com
*/

#ifndef MATRIX_H
#define MATRIX_H

template <class Any> class Matrix {
public:
  int _row;
  int _column;
  Any **_entity;
  Matrix();
  // ini stands for the initial value of all elements, r is row, c is column
  Matrix(int r, int c, int ini = 0);
  // copy constructor, one can directly use the "=" operator like A=B
  Matrix(const Matrix &m);
  // move constructor, improve the processing speed
  Matrix(Matrix &&m);
  // for a 2d array, you need to cast it to (Any*) type, see example for
  // more information
  Matrix(int r, int c, Any *m);
  /*
      'I': create an identity matrix when r==c, otherwise it's empty
  */
  Matrix(int r, int c, char type);
  ~Matrix();
  Matrix &operator*=(Any a);
  Matrix &operator*=(const Matrix &A);
  Matrix &operator/=(Any a);
  Matrix &operator+=(const Matrix &A);
  Matrix &operator-=(const Matrix &A);
  Matrix operator*(const Matrix &A);
  Matrix operator*(Any a);
  template <class T> friend Matrix<T> operator*(T a, Matrix<T> &A);
  Matrix operator/(Any a);
  Matrix operator+(const Matrix &A);
  Any operator+(Any a);

  Any *operator[](int index);

  template <class T> friend T operator+(T a, const Matrix<T> &A);

  Matrix operator-(const Matrix &A);
  Any operator-(Any a);
  template <class T> friend T operator-(T a, const Matrix<T> &A);

  Matrix &operator=(const Matrix &A);
  // move assignment, improve the processing speed
  Matrix &operator=(Matrix &&A);
  bool operator!=(const Matrix &A);
  bool operator==(const Matrix &A);
  // The inverse function uses the Gauss-Jordan Elimination,it won't modify
  // the original matrix and it returns the inverse of matrix A
  static Matrix inv(const Matrix &A);
  static Any inv(Any a);
  // The transpose function won't modify the original matrix and it returns
  // the transpose of matrix A
  static Matrix transpose(const Matrix &A);
  static Any transpose(Any a);
  // i: the row needs to be swaped, j: the position the row i goes to, this
  // function modifies the original matrix
  void swapRow(int i, int j);
  // decimal: the numbers of decimal place
  void show(int decimal = 0);
  // check if it's an empty matrix.
  bool notEmpty();
};

#endif