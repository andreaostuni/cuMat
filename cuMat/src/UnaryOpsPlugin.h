//Included inside MatrixBase, define the accessors

#define UNARY_OP_ACCESSOR(Name) \
	UnaryOp<_Derived, functor::UnaryMathFunctor_ ## Name <Scalar> > Name () { \
		return UnaryOp<_Derived, functor::UnaryMathFunctor_ ## Name <Scalar> >(derived()); \
	}

/**
 * \brief computes the component-wise negation (x -> -x)
 */
UNARY_OP_ACCESSOR(cwiseNegate);
/**
* \brief computes the component-wise absolute value (x -> |x|)
*/
UNARY_OP_ACCESSOR(cwiseAbs);
/**
* \brief computes the component-wise inverse (x -> 1/x)
*/
UNARY_OP_ACCESSOR(cwiseInverse);

/**
* \brief computes the component-wise exponent (x -> exp(x))
*/
UNARY_OP_ACCESSOR(cwiseExp);
/**
* \brief computes the component-wise natural logarithm (x -> log(x))
*/
UNARY_OP_ACCESSOR(cwiseLog);
/**
* \brief computes the component-wise value of (x -> log(x+1))
*/
UNARY_OP_ACCESSOR(cwiseLog1p);
/**
* \brief computes the component-wise value of (x -> log_10(x))
*/
UNARY_OP_ACCESSOR(cwiseLog10);

/**
* \brief computes the component-wise square root (x -> sqrt(x))
*/
UNARY_OP_ACCESSOR(cwiseSqrt);
/**
* \brief computes the component-wise reciprocal square root (x -> 1 / sqrt(x))
*/
UNARY_OP_ACCESSOR(cwiseRsqrt);
/**
* \brief computes the component-wise cube root (x -> x^(1/3))
*/
UNARY_OP_ACCESSOR(cwiseCbrt);
/**
* \brief computes the component-wise reciprocal cube root (x -> x^(-1/3))
*/
UNARY_OP_ACCESSOR(cwiseRcbrt);

/**
* \brief computes the component-wise value of (x -> sin(x))
*/
UNARY_OP_ACCESSOR(cwiseSin);
/**
* \brief computes the component-wise value of (x -> cos(x))
*/
UNARY_OP_ACCESSOR(cwiseCos);
/**
* \brief computes the component-wise value of (x -> tan(x))
*/
UNARY_OP_ACCESSOR(cwiseTan);
/**
* \brief computes the component-wise value of (x -> asin(x))
*/
UNARY_OP_ACCESSOR(cwiseAsin);
/**
* \brief computes the component-wise value of (x -> acos(x))
*/
UNARY_OP_ACCESSOR(cwiseAcos);
/**
* \brief computes the component-wise value of (x -> atan(x))
*/
UNARY_OP_ACCESSOR(cwiseAtan);
/**
* \brief computes the component-wise value of (x -> sinh(x))
*/
UNARY_OP_ACCESSOR(cwiseSinh);
/**
* \brief computes the component-wise value of (x -> cosh(x))
*/
UNARY_OP_ACCESSOR(cwiseCosh);
/**
* \brief computes the component-wise value of (x -> tanh(x))
*/
UNARY_OP_ACCESSOR(cwiseTanh);
/**
* \brief computes the component-wise value of (x -> asinh(x))
*/
UNARY_OP_ACCESSOR(cwiseAsinh);
/**
* \brief computes the component-wise value of (x -> acosh(x))
*/
UNARY_OP_ACCESSOR(cwiseAcosh);
/**
* \brief computes the component-wise value of (x -> atanh(x))
*/
UNARY_OP_ACCESSOR(cwiseAtanh);
/**
* \brief Component-wise rounds up the entries to the next larger integer.
* For an integer matrix, this does nothing
*/
UNARY_OP_ACCESSOR(cwiseCeil);
/**
* \brief Component-wise rounds down the entries to the next smaller integer.
* For an integer matrix, this does nothing
*/
UNARY_OP_ACCESSOR(cwiseFloor);
/**
* \brief Component-wise rounds the entries to the next integer.
* For an integer matrix, this does nothing
*/
UNARY_OP_ACCESSOR(cwiseRound);

/**
 * \brief Negates this matrix
 */
UnaryOp<_Derived, functor::UnaryMathFunctor_cwiseNegate<Scalar> > operator-() {
	return UnaryOp<_Derived, functor::UnaryMathFunctor_cwiseNegate <Scalar> >(derived());
}

#undef UNARY_OP_ACCESSOR