#include "matrix.hpp"
#include "log.h"

template <class T> Matrix<T>::Matrix() {
  _row = 0;
  _column = 0;
  _entity = nullptr;
}

template <class T> Matrix<T>::Matrix(int r, int c, int ini) {
  _row = r;
  _column = c;
  _entity = new T *[_row];
  for (int i = 0; i < _row; i++)
    _entity[i] = new T[_column];

  for (int i = 0; i < _row; i++)
    for (int j = 0; j < _column; j++)
      _entity[i][j] = ini;
}

template <class T> Matrix<T>::Matrix(const Matrix<T> &m) {
  _row = m._row;
  _column = m._column;

  _entity = new T *[_row];
  for (int i = 0; i < _row; i++)
    _entity[i] = new T[_column];

  int bytes = _column * sizeof(T);
  for (int i = 0; i < _row; i++)
    memcpy(_entity[i], m._entity[i], bytes);
}

template <class T> Matrix<T>::Matrix(Matrix<T> &&m) {
  _row = m._row;
  _column = m._column;
  _entity = m._entity;
  m._entity = nullptr;
  m._row = 0;
  m._column = 0;
}

template <class T> Matrix<T>::Matrix(int r, int c, T *m) {
  _row = r;
  _column = c;

  _entity = new T *[_row];
  for (int i = 0; i < r; i++)
    _entity[i] = new T[_column];

  int bytes = _column * sizeof(T);
  for (int i = 0; i < _row; i++)
    memcpy(_entity[i], m + i * _column, bytes);
}

template <class T> Matrix<T>::Matrix(int r, int c, char type) {
  _row = r;
  _column = c;

  _entity = new T *[_row];
  for (int i = 0; i < r; i++)
    _entity[i] = new T[_column];

  switch (type) {
  case 'I': // identity matrix
    if (r == c) {
      for (int i = 0; i < _row; i++)
        for (int j = 0; j < _column; j++)
          _entity[i][j] = (i == j ? 1 : 0);
    } else {
      if (_entity != nullptr) {
        for (int i = 0; i < _row; i++)
          delete[] _entity[i];
        delete[] _entity;
      }
    }
    break;
  default:
    for (int i = 0; i < _row; i++)
      for (int j = 0; j < _column; j++)
        _entity[i][j] = 0;
  }
}

template <class T> Matrix<T>::~Matrix() {
  if (_entity != nullptr) {
    for (int i = 0; i < _row; i++)
      delete[] _entity[i];
    delete[] _entity;
  }
}

template <class T> Matrix<T> Matrix<T>::operator*(const Matrix<T> &A) {
  if (_column != A._row)
    LogDebug("Multiplication matrix dimension not match\n");

  Matrix<T> tmp(_row, A._column);

  for (int i = 0; i < _row; i++)
    for (int j = 0; j < A._column; j++) {
      for (int k = 0; k < _column; k++)
        tmp._entity[i][j] += _entity[i][k] * A._entity[k][j];
    }

  return tmp;
}

template <class T> Matrix<T> &Matrix<T>::operator*=(const Matrix<T> &A) {
  // check if the dimension matches
  if (_column != A._row) {
    if (_entity != nullptr) {
      for (int i = 0; i < _row; i++)
        delete[] _entity[i];
      delete[] _entity;
    }
    return *this;
  }

  Matrix<T> tmp(*this);

  // the dimension of the result matrix does not match the calling matrix
  if (_column != A._column) {
    for (int i = 0; i < _row; i++)
      delete[] _entity[i];

    _column = A._column;

    for (int i = 0; i < _row; i++)
      _entity[i] = new T[_column];

    for (int i = 0; i < _row; i++)
      for (int j = 0; j < _column; j++) {
        _entity[i][j] = 0;
        for (int k = 0; k < tmp._column; k++)
          _entity[i][j] += tmp._entity[i][k] * A._entity[k][j];
      }
  } else { // the dimension of the result matrix matches the calling matrix
    for (int i = 0; i < _row; i++)
      for (int j = 0; j < _column; j++) {
        _entity[i][j] = 0;
        for (int k = 0; k < _column; k++)
          _entity[i][j] += tmp._entity[i][k] * A._entity[k][j];
      }
  }

  return *this;
}

template <class T> Matrix<T> Matrix<T>::operator*(T a) {
  Matrix<T> tmp(_row, _column);

  for (int i = 0; i < _row; i++)
    for (int j = 0; j < _column; j++) {
      tmp._entity[i][j] = _entity[i][j] * a;
    }

  return tmp;
}

template <class T> Matrix<T> &Matrix<T>::operator*=(T a) {
  for (int i = 0; i < _row; i++)
    for (int j = 0; j < _column; j++) {
      _entity[i][j] *= a;
    }

  return *this;
}

template <class T> Matrix<T> operator*(T a, Matrix<T> &A) { return A * a; }

template <class T> Matrix<T> Matrix<T>::operator/(T a) {
  Matrix<T> tmp(_row, _column);

  for (int i = 0; i < _row; i++)
    for (int j = 0; j < _column; j++) {
      tmp._entity[i][j] = _entity[i][j] / a;
    }

  return tmp;
}

template <class T> Matrix<T> &Matrix<T>::operator/=(T a) {
  for (int i = 0; i < _row; i++)
    for (int j = 0; j < _column; j++) {
      _entity[i][j] /= a;
    }

  return *this;
}

template <class T> Matrix<T> Matrix<T>::operator+(const Matrix<T> &A) {
  if (_row != A._row || _column != A._column)
    LogDebug("Plus matrix dimension not match\n");

  Matrix<T> tmp(_row, _column);

  for (int i = 0; i < _row; i++)
    for (int j = 0; j < _column; j++) {
      tmp._entity[i][j] = _entity[i][j] + A._entity[i][j];
    }

  return tmp;
}

template <class T> T Matrix<T>::operator+(T a) {
  if (_row != 1 || _column != 1)
    LogDebug("Plus scalar dimension not match\n");

  return _entity[0][0] + a;
}

template <class T> T operator+(T a, const Matrix<T> &A) {
  if (A._row != 1 || A._column != 1)
    LogDebug("Plus scalar dimension not match\n");

  return a + A._entity[0][0];
}

template <class T> Matrix<T> &Matrix<T>::operator+=(const Matrix<T> &A) {
  if (_row != A._row || _column != A._column) {
    for (int i = 0; i < _row; i++)
      delete[] _entity[i];
    delete[] _entity;
    return *this;
  }

  for (int i = 0; i < _row; i++)
    for (int j = 0; j < _column; j++) {
      _entity[i][j] = _entity[i][j] + A._entity[i][j];
    }

  return *this;
}

template <class T> Matrix<T> Matrix<T>::operator-(const Matrix<T> &A) {
  if (_row != A._row || _column != A._column)
    LogDebug("Sub scalar dimension not match\n");

  Matrix<T> tmp(_row, _column);

  for (int i = 0; i < _row; i++)
    for (int j = 0; j < _column; j++) {
      tmp._entity[i][j] = _entity[i][j] - A._entity[i][j];
    }

  return tmp;
}

template <class T> T Matrix<T>::operator-(T a) {
  if (_row != 1 || _column != 1)
    LogDebug("Sub scalar dimension not match\n");

  return _entity[0][0] - a;
}

template <class T> T operator-(T a, const Matrix<T> &A) {
  if (A._row != 1 || A._column != 1)
    LogDebug("Sub scalar dimension not match\n");

  return a - A._entity[0][0];
}

template <class T> Matrix<T> &Matrix<T>::operator-=(const Matrix<T> &A) {
  if (_row != A._row || _column != A._column) {
    for (int i = 0; i < _row; i++)
      delete[] _entity[i];
    delete[] _entity;
    return *this;
  }

  for (int i = 0; i < _row; i++)
    for (int j = 0; j < _column; j++) {
      _entity[i][j] = _entity[i][j] - A._entity[i][j];
    }

  return *this;
}

template <class T> T *Matrix<T>::operator[](int index) {
  return _entity[index];
}

template <class T> Matrix<T> Matrix<T>::transpose(const Matrix<T> &A) {
  Matrix<T> tmp(A._column, A._row);

  for (int i = 0; i < A._row; i++)
    for (int j = 0; j < A._column; j++)
      tmp._entity[j][i] = A._entity[i][j];

  return tmp;
}

template <class T> Matrix<T> &Matrix<T>::operator=(const Matrix<T> &A) {
  if (_entity != nullptr) {
    for (int i = 0; i < _row; i++)
      delete[] _entity[i];
    delete[] _entity;
  }

  _row = A._row;
  _column = A._column;

  _entity = new T *[_row];
  for (int i = 0; i < _row; i++)
    _entity[i] = new T[_column];

  for (int i = 0; i < _row; i++)
    for (int j = 0; j < _column; j++)
      _entity[i][j] = A._entity[i][j];

  return *this;
}

template <class T> Matrix<T> &Matrix<T>::operator=(Matrix<T> &&A) {
  if (_entity != nullptr) {
    for (int i = 0; i < _row; i++)
      delete[] _entity[i];
    delete[] _entity;
  }
  _row = A._row;
  _column = A._column;
  _entity = A._entity;
  A._entity = nullptr;
  A._row = 0;
  A._column = 0;
  return *this;
}

template <class T> bool Matrix<T>::operator!=(const Matrix<T> &A) {
  if (_row != A._row || _column != A._column)
    return true;

  for (int i = 0; i < _row; i++)
    for (int j = 0; j < _column; j++)
      if (_entity[i][j] != A._entity[i][j])
        return true;

  return false;
}

template <class T> bool Matrix<T>::operator==(const Matrix<T> &A) {
  if (_row != A._row || _column != A._column)
    return false;

  for (int i = 0; i < _row; i++)
    for (int j = 0; j < _column; j++)
      if (_entity[i][j] != A._entity[i][j])
        return false;

  return true;
}

template <class T> Matrix<T> Matrix<T>::inv(const Matrix &A) {
  // has to be a square matrix
  if (A._row != A._column) {
    LogDebug("Inv Has to be a square matrix\n");
  }

  Matrix<T> tmp(A._row, A._column, 'I');
  Matrix<T> copy(A);

  // set the pivot to be the largest element in that column
  for (int i = 0; i < A._row - 1; i++) {
    T pivot = copy._entity[i][i];
    int row = i;
    for (int j = i + 1; j < A._row; j++)
      if (copy._entity[j][i] > pivot) {
        pivot = copy._entity[j][i];
        row = j;
      }

    if (pivot == 0) {
      return Matrix<T>();
    } else {
      copy.swapRow(i, row);
      tmp.swapRow(i, row);
    }
  }

  for (int i = 0; i < A._row; i++)
    for (int j = 0; j < A._row; j++)
      if (i != j) {
        T backup = copy._entity[j][i];
        for (int k = 0; k < A._column; k++) {
          copy._entity[j][k] -=
              backup * copy._entity[i][k] / copy._entity[i][i];
          tmp._entity[j][k] -= backup * tmp._entity[i][k] / copy._entity[i][i];
        }
      }

  for (int i = 0; i < A._row; i++)
    for (int j = 0; j < A._column; j++) {
      tmp._entity[i][j] /= copy._entity[i][i];
    }

  return tmp;
}

template <class T> T Matrix<T>::inv(T a) { return 1.0 / a; }

template <class T> void Matrix<T>::swapRow(int i, int j) {
  if (i != j && i < _row && j < _row && i >= 0 && j >= 0) {
    T tmp;
    for (int k = 0; k < _column; k++) {
      tmp = _entity[j][k];
      _entity[j][k] = _entity[i][k];
      _entity[i][k] = tmp;
    }
  }
}

template <class T> void Matrix<T>::show(int decimal) {
  if (decimal <= 0) {
    for (int i = 0; i < _row; i++) {
      LogDebug("[");
      for (int j = 0; j < _column; j++) {
        LogDebug("  ");
        LogDebug("%f", _entity[i][j]);
        LogDebug(" ");
      }
      LogDebug("]");
    }
    LogDebug("\n");
  } else {
    for (int i = 0; i < _row; i++) {
      LogDebug("[");
      for (int j = 0; j < _column; j++) {
        LogDebug("  ");
        LogDebug("%f %d", _entity[i][j], decimal);
        LogDebug(" ");
      }
      LogDebug("]");
    }
    LogDebug("\n");
  }
}

template <class T> bool Matrix<T>::notEmpty() {
  if (_entity == nullptr)
    return false;
  return true;
}