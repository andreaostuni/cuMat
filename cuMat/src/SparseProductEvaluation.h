#ifndef __CUMAT_SPARSE_PRODUCT_EVALUATION__
#define __CUMAT_SPARSE_PRODUCT_EVALUATION__

#include "Macros.h"
#include "ProductOp.h"
#include "SparseMatrix.h"

CUMAT_NAMESPACE_BEGIN

namespace internal
{

    // CwiseSrcTag * CwiseSrcTag -> SparseDstTag; outer product
    //This handles all dense cwise+matrix inputs and dense matrix output
    //The sparse methods (SparseSrcTag, SparseDstTag) are handled seperately
    template<
        typename _Dst, ProductArgOp _DstOp,
        typename _SrcLeft, ProductArgOp _SrcLeftOp,
        typename _SrcRight, ProductArgOp _SrcRightOp,
        AssignmentMode _AssignmentMode
    >
    struct ProductAssignment<_Dst, SparseDstTag, _DstOp, _SrcLeft, CwiseSrcTag, _SrcLeftOp, _SrcRight, CwiseSrcTag, _SrcRightOp, _AssignmentMode>
    {
        using Op = ProductOp<_SrcLeft, _SrcRight, _SrcLeftOp, _SrcRightOp, _DstOp>;
        using Scalar = typename Op::Scalar;

        static void assign(_Dst& dst, const Op& op) {
            //Check that the input matrices are vectors
            CUMAT_STATIC_ASSERT((Op::TransposedLeft ? Op::RowsLeft : Op::ColumnsLeft == 1),
                "Product evaluation into a sparse matrix is only supported for the outer product of two vectors, left matrix is not a column vector");
            CUMAT_STATIC_ASSERT((Op::TransposedRight ? Op::ColumnsRight : Op::RowsRight == 1),
                "Product evaluation into a sparse matrix is only supported for the outer product of two vectors, right matrix is not a row vector");

            //launch cwise-evaluation
            Assignment<_Dst, Op, _AssignmentMode, typename traits<_Dst>::DstTag, CwiseSrcTag>::assign(dst, op);
        }
    };
    
    namespace
    {

    template <typename L, typename R, typename M, AssignmentMode Mode, int Batches,
        bool BroadcastMatrix = internal::traits<L>::BatchesAtCompileTime==1,
        bool BroadcastRhs = internal::traits<R>::BatchesAtCompileTime == 1>
    __global__ void CSRMVKernel_StaticBatches(dim3 virtual_size, const L matrix, const R vector, M output)
    {
        typedef typename L::Scalar LeftScalar;
        typedef typename R::Scalar RightScalar;
        typedef typename M::Scalar OutputScalar;
        typedef ProductElementFunctor<LeftScalar, RightScalar, ProductArgOp::NONE, ProductArgOp::NONE, ProductArgOp::NONE> Functor;
        const int* JA = matrix.getOuterIndices().data();
        const int* IA = matrix.getInnerIndices().data();
        CUMAT_KERNEL_1D_LOOP(outer, virtual_size)
            int start = JA[outer];
            int end = JA[outer + 1];
            if (start>=end) continue;
            int inner = IA[start];
            OutputScalar value[Batches];
#pragma unroll
            for (int b = 0; b < Batches; ++b) {
                LeftScalar tmp1 = BroadcastMatrix
                    ? matrix.getSparseCoeff(outer, inner, 0, start)
                    : matrix.getSparseCoeff(outer, inner, b, start + b * matrix.nnz());
                RightScalar tmp2 = vector.coeff(inner, 0, BroadcastRhs ? 0 : b, -1);
                OutputScalar tmp3 = Functor::mult(tmp1, tmp2);
                value[b] = tmp3;
            }
            for (int i=start+1; i<end; ++i)
            {
                inner = IA[i];
#pragma unroll
                for (int b = 0; b < Batches; ++b) {
                    LeftScalar tmp1 = BroadcastMatrix
                        ? matrix.getSparseCoeff(outer, inner, 0, i)
                        : matrix.getSparseCoeff(outer, inner, b, i + b * matrix.nnz());
                    RightScalar tmp2 = vector.coeff(inner, 0, BroadcastRhs ? 0 : b, -1);
                    OutputScalar tmp3 = Functor::mult(tmp1, tmp2);
                    value[b] += tmp3;
                }
            }
#pragma unroll
            for (int b = 0; b < Batches; ++b) {
                internal::CwiseAssignmentHandler<M, OutputScalar, Mode>::assign(output, value[b], outer + b*output.rows());
            }
		CUMAT_KERNEL_1D_LOOP_END
    }

    }

    //CwiseSrcTag (Sparse) * CwiseSrcTag (Dense-Vector) -> DenseDstTag (Vector-Vector), sparse matrix-vector product
    //Currently, only non-batched CSR matrices are supported
    //TODO: support also CSC, vector on the left, transposed and conjugated versions
    template<
        typename _Dst,// ProductArgOp _DstOp,
        typename _SrcLeftScalar, int _SrcLeftBatches,// ProductArgOp _SrcLeftOp,
        typename _SrcRight,// ProductArgOp _SrcRightOp,
        AssignmentMode _AssignmentMode
    >
    struct ProductAssignment<
        _Dst, DenseDstTag, ProductArgOp::NONE, 
        SparseMatrix<_SrcLeftScalar, _SrcLeftBatches, SparseFlags::CSR>, CwiseSrcTag, ProductArgOp::NONE,
        _SrcRight, CwiseSrcTag, ProductArgOp::NONE, 
        _AssignmentMode>
    {
        using SrcLeft = SparseMatrix<_SrcLeftScalar, _SrcLeftBatches, SparseFlags::CSR>;
        using Op = ProductOp<SrcLeft, _SrcRight, ProductArgOp::NONE, ProductArgOp::NONE, ProductArgOp::NONE>;
        using Scalar = typename Op::Scalar;

        CUMAT_STATIC_ASSERT((Op::ColumnsRight == 1),
                "SparseMatrix - DenseVector product only supports column vectors as right argument, use batches instead");
        CUMAT_STATIC_ASSERT((Op::Batches != Dynamic),
                "SparseMatrix - DenseVector does only support compile-time fixed batch count");

        static void assign(_Dst& dst, const Op& op) {
            
            typedef typename _Dst::Type DstActual;
            CUMAT_PROFILING_INC(EvalMatmulSparse);
            CUMAT_PROFILING_INC(EvalAny);
            if (dst.size() == 0) return;
            CUMAT_ASSERT(op.rows() == dst.rows());
            CUMAT_ASSERT(op.cols() == dst.cols());
            CUMAT_ASSERT(op.batches() == dst.batches());
            CUMAT_ASSERT(op.batches() == Op::Batches);

			CUMAT_LOG_DEBUG("Evaluate SparseMatrix-DenseVector multiplication " << typeid(op.derived()).name()
				<< " matrix rows=" << op.derived().left().rows() << ", cols=" << op.left().cols());;

            //here is now the real logic
            Context& ctx = Context::current();
            KernelLaunchConfig cfg = ctx.createLaunchConfig1D(dst.rows(), CSRMVKernel_StaticBatches<SrcLeft, typename _SrcRight::Type, DstActual, _AssignmentMode, Op::Batches>);
            CSRMVKernel_StaticBatches<SrcLeft, typename _SrcRight::Type, DstActual, _AssignmentMode, Op::Batches>
                <<<cfg.block_count, cfg.thread_per_block, 0, ctx.stream() >>>
                (cfg.virtual_size, op.derived().left().derived(), op.derived().right().derived(), dst.derived());
            CUMAT_CHECK_ERROR();
            CUMAT_LOG_DEBUG("Evaluation done");
        }
    };

    //CwiseSrcTag (SparseExpressionOp) * CwiseSrcTag (Dense-Vector) -> DenseDstTag (Vector-Vector), sparse matrix-vector product
    //Currently, only non-batched CSR matrices are supported
    //TODO: support also CSC, vector on the left, transposed and conjugated versions
    template<
        typename _Dst,
        typename _SrcLeftChild,
        typename _SrcRight,
        AssignmentMode _AssignmentMode
    >
    struct ProductAssignment<
        _Dst, DenseDstTag, ProductArgOp::NONE, 
        SparseExpressionOp<_SrcLeftChild, SparseFlags::CSR>, CwiseSrcTag, ProductArgOp::NONE, 
        _SrcRight, CwiseSrcTag, ProductArgOp::NONE, 
        _AssignmentMode>
    {
        using SrcLeft = SparseExpressionOp<_SrcLeftChild, SparseFlags::CSR>;
        using Op = ProductOp<SrcLeft, _SrcRight, ProductArgOp::NONE, ProductArgOp::NONE, ProductArgOp::NONE>;
        using Scalar = typename Op::Scalar;

        CUMAT_STATIC_ASSERT((Op::ColumnsRight == 1),
                "SparseMatrix - DenseVector product only supports column vectors as right argument (for now)");
        CUMAT_STATIC_ASSERT((Op::Batches != Dynamic),
            "SparseMatrix - DenseVector does only support compile-time fixed batch count");

        static void assign(_Dst& dst, const Op& op) {
            
            typedef typename _Dst::Type DstActual;
            CUMAT_PROFILING_INC(EvalMatmulSparse);
            CUMAT_PROFILING_INC(EvalAny);
            if (dst.size() == 0) return;
            CUMAT_ASSERT(op.rows() == dst.rows());
            CUMAT_ASSERT(op.cols() == dst.cols());
            CUMAT_ASSERT(op.batches() == dst.batches());

			CUMAT_LOG_DEBUG("Evaluate SparseMatrix-DenseVector multiplication " << typeid(op.derived()).name()
				<< " matrix rows=" << op.derived().left().rows() << ", cols=" << op.left().cols());;

            //here is now the real logic
            Context& ctx = Context::current();
            KernelLaunchConfig cfg = ctx.createLaunchConfig1D(static_cast<unsigned int>(dst.size()), CSRMVKernel_StaticBatches<SrcLeft, typename _SrcRight::Type, DstActual, _AssignmentMode, Op::Batches>);
            CSRMVKernel_StaticBatches<SrcLeft, typename _SrcRight::Type, DstActual, _AssignmentMode, Op::Batches>
                <<<cfg.block_count, cfg.thread_per_block, 0, ctx.stream() >>>
                (cfg.virtual_size, op.derived().left().derived(), op.derived().right().derived(), dst.derived());
            CUMAT_CHECK_ERROR();
            CUMAT_LOG_DEBUG("Evaluation done");
        }
    };
}

CUMAT_NAMESPACE_END

#endif