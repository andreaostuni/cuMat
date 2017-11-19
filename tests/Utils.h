#ifndef __CUMAT_TESTS_UTILS_H__
#define __CUMAT_TESTS_UTILS_H__

#include <catch/catch.hpp>
#include <cuMat/src/Matrix.h>
#include <Eigen/Core>

#define __CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, _scalar, _rows, _cols, _batches, _flags, rows, cols, batches) \
	{ \
		INFO("Run " << #Test << ": Scalar=" << #_scalar << ", RowsAtCompileTime=" << #_rows << ", ColsAtCompileTime=" << #_cols << ", BatchesAtCompileTime=" << #_batches << ", Flags=" << #_flags << ", Rows=" << rows << ", Cols=" << cols << ", Batches=" << batches); \
		Test<_scalar, _rows, _cols, _batches, _flags>(rows, cols, batches); \
	}

/**
 * \brief Tests a function template 'Test' based on different settings for sizes.
 * The function 'Test' must be defined as follows:
 * \code
 * template<typename _Scalar, int _Rows, int _Cols, int _Batches, int _Flags>
 * void MyTestFunction(Index rows, Index cols, Index batches) {...};
 * \endcode
 * And then you can call it on floating point values e.g. by
 * \code
 * CUMAT_TESTS_CALL_MATRIX_TEST(float, MyTestFunction);
 * CUMAT_TESTS_CALL_MATRIX_TEST(double, MyTestFunction);
 * \endcode
 * \param Scalar the scalar type
 * \param Test the test function
 */
#define CUMAT_TESTS_CALL_MATRIX_TEST(Scalar, Test) \
{ \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, 3, 4, 5, cuMat::RowMajor, 3, 4, 5); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, cuMat::Dynamic, 4, 5, cuMat::RowMajor, 8, 4, 5); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, 3, cuMat::Dynamic, 5, cuMat::RowMajor, 3, 8, 5); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, 3, 4, cuMat::Dynamic, cuMat::RowMajor, 3, 4, 8); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, cuMat::Dynamic, cuMat::Dynamic, 5, cuMat::RowMajor, 8, 2, 5); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, cuMat::Dynamic, 4, cuMat::Dynamic, cuMat::RowMajor, 2, 4, 8); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, 3, cuMat::Dynamic, cuMat::Dynamic, cuMat::RowMajor, 3, 8, 2); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, cuMat::Dynamic, cuMat::Dynamic, cuMat::Dynamic, cuMat::RowMajor, 10, 3, 4); \
	\
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, 3, 4, 5, cuMat::ColumnMajor, 3, 4, 5); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, cuMat::Dynamic, 4, 5, cuMat::ColumnMajor, 8, 4, 5); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, 3, cuMat::Dynamic, 5, cuMat::ColumnMajor, 3, 8, 5); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, 3, 4, cuMat::Dynamic, cuMat::ColumnMajor, 3, 4, 8); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, cuMat::Dynamic, cuMat::Dynamic, 5, cuMat::ColumnMajor, 8, 2, 5); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, cuMat::Dynamic, 4, cuMat::Dynamic, cuMat::ColumnMajor, 2, 4, 8); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, 3, cuMat::Dynamic, cuMat::Dynamic, cuMat::ColumnMajor, 3, 8, 2); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, cuMat::Dynamic, cuMat::Dynamic, cuMat::Dynamic, cuMat::ColumnMajor, 10, 3, 4); \
}

#define CUMAT_TESTS_CALL_SIMPLE_MATRIX_TEST(Scalar, Test) \
{ \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, cuMat::Dynamic, cuMat::Dynamic, cuMat::Dynamic, cuMat::RowMajor, 10, 3, 4); \
	__CUMAT_TESTS_CALL_SINGLE_MATRIX_TEST(Test, Scalar, cuMat::Dynamic, cuMat::Dynamic, cuMat::Dynamic, cuMat::ColumnMajor, 10, 3, 4); \
}


template<typename Derived, typename Scalar, int Rows, int Cols, int Batches>
void assertMatrixEquality(const Scalar(&expected)[Batches][Rows][Cols], const cuMat::MatrixBase<Derived>& actual)
{
    const auto mat = actual.derived().eval();
    REQUIRE(Rows == mat.rows());
    REQUIRE(Cols == mat.cols());
    REQUIRE(Batches == mat.batches());
    for (int batch=0; batch<Batches; ++batch)
    {
        const auto emat = mat.template block<Rows, Cols, 1>(0, 0, batch).eval().toEigen();
        for (int column = 0; column < mat.cols(); ++column) {
            for (int row = 0; row < mat.rows(); ++row) {
                INFO("row=" << row << ", column=" << column << ", batch=" << batch);
                CHECK(expected[batch][row][column] == emat(row, column));
            }
        }
    }
}


#endif