//Included inside MatrixBase

//most general version, static size

/**
 * \brief Creates a block of the matrix of static size.
 * By using this method, you can convert a dynamically-sized matrix into a statically sized one.
 * 
 * \param start_row the start row of the block (zero based)
 * \param start_column the start column of the block (zero based)
 * \param start_batch the start batch of the block (zero based)
 * \tparam NRows the number of rows of the block on compile time
 * \tparam NColumsn the number of columns of the block on compile time
 * \tparam NBatches the number of batches of the block on compile time
 */
template<int NRows, int NColumns, int NBatches>
MatrixBlock<typename internal::traits<_Derived>::Scalar, NRows, NColumns, NBatches, internal::traits<_Derived>::Flags, const _Derived>
block(Index start_row, Index start_column, Index start_batch, Index num_rows = NRows,
    Index num_columns = NColumns, Index num_batches = NBatches) const
{
    CUMAT_ASSERT_ARGUMENT(NRows > 0 ? NRows == num_rows : true);
    CUMAT_ASSERT_ARGUMENT(NColumns > 0 ? NColumns == num_columns : true);
    CUMAT_ASSERT_ARGUMENT(NBatches > 0 ? NBatches == num_batches : true);
    CUMAT_ASSERT_ARGUMENT(num_rows >= 0);
    CUMAT_ASSERT_ARGUMENT(num_columns >= 0);
    CUMAT_ASSERT_ARGUMENT(num_batches >= 0);
    CUMAT_ASSERT_ARGUMENT(start_row >= 0);
    CUMAT_ASSERT_ARGUMENT(start_column >= 0);
    CUMAT_ASSERT_ARGUMENT(start_batch >= 0);
    CUMAT_ASSERT_ARGUMENT(start_row + num_rows <= rows());
    CUMAT_ASSERT_ARGUMENT(start_column + num_columns <= cols());
    CUMAT_ASSERT_ARGUMENT(start_batch + num_batches <= batches());
    return MatrixBlock<typename internal::traits<_Derived>::Scalar, NRows, NColumns, NBatches, internal::traits<_Derived>::Flags, const _Derived>(
        derived(), num_rows, num_columns, num_batches, start_row, start_column, start_batch);
}

//most general version, dynamic size

/**
* \brief Creates a block of the matrix of dynamic size.
*
* \param start_row the start row of the block (zero based)
* \param start_column the start column of the block (zero based)
* \param start_batch the start batch of the block (zero based)
* \param num_rows the number of rows in the block
* \param num_columns the number of columns in the block
* \param num_batches the number of batches in the block
*/
MatrixBlock<typename internal::traits<_Derived>::Scalar, Dynamic, Dynamic, Dynamic, internal::traits<_Derived>::Flags, const _Derived>
block(Index start_row, Index start_column, Index start_batch, Index num_rows, Index num_columns, Index num_batches) const
{
    CUMAT_ASSERT_ARGUMENT(start_row >= 0);
    CUMAT_ASSERT_ARGUMENT(start_column >= 0);
    CUMAT_ASSERT_ARGUMENT(start_batch >= 0);
    CUMAT_ASSERT_ARGUMENT(num_rows > 0);
    CUMAT_ASSERT_ARGUMENT(num_columns > 0);
    CUMAT_ASSERT_ARGUMENT(num_batches > 0);
    CUMAT_ASSERT_ARGUMENT(start_row + num_rows <= rows());
    CUMAT_ASSERT_ARGUMENT(start_column + num_columns <= cols());
    CUMAT_ASSERT_ARGUMENT(start_batch + num_batches <= batches());
    return MatrixBlock<typename internal::traits<_Derived>::Scalar, Dynamic, Dynamic, Dynamic, internal::traits<_Derived>::Flags, const _Derived>(
        derived(), num_rows, num_columns, num_batches, start_row, start_column, start_batch);
}

// TODO: specializations for batch==1, vectors, slices
