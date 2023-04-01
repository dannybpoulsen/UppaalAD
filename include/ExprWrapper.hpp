#ifndef _EXPRWRAPPER__
#define _EXPRWRAPPER__

#include "utap/common.h"
#include "utap/expression.h"

#include <cassert>
#include <type_traits>
#include <stdexcept>
#include <iostream>


namespace UppaalAD {
#define BINARY_OPS				\
  X(PLUS)					\
  X(MINUS)					\
  X(MULT)					\
  X(DIV)					\
  X(BIT_AND)					\
  X(BIT_OR)					\
  X(BIT_XOR)					\
  X(AND)					\
  X(OR)						\
  X(XOR)					\
  X(LT)						\
  X(LE)						\
  X(EQ)						\
  X(GE)						\
  X(GT)						\
  X(NEQ)					\
  X(ARRAY)					\
  X(COMMA)					\
  X(FRACTION)
  
  
#define UNARY_OPS 				\
  X(NOT)					\
  X(UNARY_MINUS)				\
  X(POSTINCREMENT)				\
  X(PREINCREMENT)				\
  X(POSTDECREMENT)				\
  X(PREDECREMENT)

#define NARY_OPS 				\
  X(LIST)					\
  X(FUNCALL)
  
#define ASSIGN_OPS				\
  X(ASSIGN)					\
  X(ASSPLUS)					\
  X(ASSMINUS)					\
  X(ASSDIV)					\
  X(ASSMOD)					\
  X(ASSMULT)					\
  X(ASSAND)					\
  X(ASSOR)					\
  X(ASSXOR)					\
  X(ASSLSHIFT)					\
  X(ASSRSHIFT)  


#define BUILTIN_F1				\
  X(ABS_F)					\
  X(FABS_F)					\
  X(EXP_F)					\
  X(EXP2_F)					\
  X(EXPM1_F)					\
  X(LN_F)					\
  X(LOG_F)					\
  X(LOG10_F)					\
  X(LOG2_F)					\
  X(LOG1P_F)					\
  X(SQRT_F)					\
  X(CBRT_F)					\
  X(SIN_F)					\
  X(COS_F)					\
  X(TAN_F)					\
  X(ASIN_F)					\
  X(ACOS_F)					\
  X(ATAN_F)					\
  X(ACOSH_F)					\
  X(ATANH_F)					\
  X(ERF_F)					\
  X(TGAMMA_F)					\
  X(LGAMMA_F)					\
  X(CEIL_F)					\
  X(FLOOR_F)					\
  X(TRUNC_F)					\
  X(ROUND_F)					\
  X(FINT_F)					\
  X(ILOGB_F)					\
  X(LOGB_F)					\
  X(FPCLASSIFY_F)				\
  X(ISFINITE_F)					\
  X(ISINF_F)					\
  X(ISNAN_F)					\
  X(ISNORMAL_F)					\
  X(SIGNBIT_F)					\
  X(ISUNORDERED_F)				\
  X(RANDOM_F)					\
  X(RANDOM_POISSON_F)
  
  
#define CONSTANTS				\
  X(CONSTANT)					\
  
  template<UTAP::Constants::kind_t kind>
  struct is_binary : public std::false_type {};
#define X(OP)								\
  template<> 								\
  struct is_binary<UTAP::Constants::kind_t::OP> : public std::true_type  { \
  };
  BINARY_OPS
#undef X
  
  template<UTAP::Constants::kind_t kind>
  constexpr bool is_binary_v = is_binary<kind>::value;
  

  template<UTAP::Constants::kind_t kind>
  struct is_unary : public std::false_type {};
#define X(OP)								\
  template<> 								\
  struct is_unary<UTAP::Constants::kind_t::OP> : public std::true_type  { \
  };
  UNARY_OPS
#undef X
  
  template<UTAP::Constants::kind_t kind>
  constexpr bool is_unary_v = is_unary<kind>::value;
  
  template<UTAP::Constants::kind_t kind>
  struct is_assign : public std::false_type {};
#define X(OP)								\
  template<> 								\
  struct is_assign<UTAP::Constants::kind_t::OP> : public std::true_type  { \
  };
  ASSIGN_OPS
#undef X
  
  template<UTAP::Constants::kind_t kind>
  constexpr bool is_assign_v = is_assign<kind>::value;
  
  
  
  template<UTAP::Constants::kind_t kind>
  struct is_constant : public std::false_type {};
  
  template<>
  struct is_constant<UTAP::Constants::kind_t::CONSTANT> : public std::true_type {};

  template<UTAP::Constants::kind_t kind>
  constexpr bool is_constant_v = is_constant<kind>::value;
  
  
  template<UTAP::Constants::kind_t kind>
  struct is_builtin : public std::false_type {
    static constexpr std::size_t params = 0;
  };

#define X(OP)					\
  template<>								\
  struct is_builtin<UTAP::Constants::kind_t::OP>: public std::true_type { \
    static constexpr std::size_t params = 1;					\
  };									\
  
  BUILTIN_F1
#undef X

  
  template<UTAP::Constants::kind_t kind>
  constexpr bool is_builtin_v = is_builtin<kind>::value;
  
  template<UTAP::Constants::kind_t kind>
  constexpr std::size_t nb_params_v = is_builtin<kind>::params;

  template<UTAP::Constants::kind_t kind>
  constexpr bool is_builtin_1_v = is_builtin<kind>::value && nb_params_v<kind> == 1;

  template<UTAP::Constants::kind_t kind>
  struct is_dot : public std::false_type {
  };

  template<>
  struct is_dot<UTAP::Constants::kind_t::DOT> : public std::true_type {
  };
  
  template<UTAP::Constants::kind_t kind>
  constexpr bool is_dot_v = is_dot<kind>::value;
  
  template<UTAP::Constants::kind_t kind>
  struct is_nary : public std::false_type {};

  
#define X(OP)								\
  template<> 								\
  struct is_nary<UTAP::Constants::kind_t::OP> : public std::true_type  { \
  };
  NARY_OPS
#undef X

  

  template<UTAP::Constants::kind_t kind>
  constexpr bool is_nary_v = is_nary<kind>::value;
  

  
  template<UTAP::Constants::kind_t kind>
  struct Expression {
    Expression (UTAP::expression_t& e) : expr(e) {
      if constexpr (kind == UTAP::Constants::kind_t::DOT) {
	if (!expr[0].getType ().isRecord ()) {
	  throw std::logic_error ("Unsupported Dot Kind");
	}
      }
      
      assert(expr.getKind () == kind);
    }

    
    
    auto& getType () const {return expr.getType ();}
    auto getName () const requires (kind == UTAP::Constants::kind_t::IDENTIFIER) {
      return expr.getSymbol().getName ();
    }

    auto& getAssignee ()  requires(is_assign_v<kind>) {return expr[0];}
    auto& getExpr ()  requires(is_assign_v<kind>) {return expr[1];}
    
    auto size () requires (is_nary_v<kind>) {return expr.getSize();}
    auto& operator[] (std::size_t i) requires (is_nary_v<kind>){return expr.get(i);}
    
    auto& getInner ()  requires(is_unary_v<kind>) {return expr[0];}
    auto& getLeft ()  requires(is_binary_v<kind>) {return expr[0];}
    auto& getRight ()  requires(is_binary_v<kind>) {return expr[1];}
    auto getValue () const requires (is_constant_v<kind>) {return expr.getValue ();}
    auto getDoubleValue () const requires (is_constant_v<kind>) {
      if (expr.getType ().isDouble())
	return expr.getDoubleValue ();
      return static_cast<double>(expr.getValue ());
    }
    
    bool isDouble () const requires (is_constant_v<kind>) {
      return expr.getType().isDouble ();
    }

    bool isOutput () const requires (kind == UTAP::Constants::kind_t::SYNC) {
      return expr.getSync () == UTAP::Constants::synchronisation_t::SYNC_BANG;
    }

    bool isInput () const requires (kind == UTAP::Constants::kind_t::SYNC) {
      return expr.getSync () == UTAP::Constants::synchronisation_t::SYNC_QUE;
    }
    
    auto& getChannelExpr () const requires (kind == UTAP::Constants::kind_t::SYNC) {
      return expr[0];
    }

    auto getSyncType () const requires (kind == UTAP::Constants::kind_t::SYNC) {
      return expr.getSync ();
    }
    template<std::size_t i>
    auto& getParam () const requires(is_builtin_v<kind> && i < nb_params_v<kind>) {
      return expr[i];
    }

    auto& getBase () const requires(is_dot_v<kind>) {
      return expr[0];
    }

    auto indexName () const requires(is_dot_v<kind>) {
      auto index = expr.getIndex ();
      return expr[0].getType().getRecordLabel (index);
    }
    
  private:
    UTAP::expression_t& expr;
  };


  
  template<class T, UTAP::Constants::kind_t k>
  concept Visitor = requires (T t, Expression<k> e) {
      t  (e);
  };

  
 
  
  
  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  
  template<class T>
  auto visit (T&& t, UTAP::expression_t& e) {
    switch (e.getKind ()) {
#define X(OP)								\
      case UTAP::Constants::kind_t::OP:					\
	if constexpr (Visitor<T,UTAP::Constants::kind_t::OP>) {		\
	  return t (Expression<UTAP::Constants::kind_t::OP> (e));	\
	}								\
	else								\
	  throw std::logic_error ("Unsupported 2");
      BINARY_OPS
	CONSTANTS
	UNARY_OPS
	NARY_OPS
	ASSIGN_OPS
	BUILTIN_F1
	X(IDENTIFIER)
	X(SYNC)
	X(DOT)
        #undef X
     
    default:
	std::cerr << e << e.getKind () << std::endl;
      throw std::logic_error ("Unsupported");
    };
  }
  
}


#endif
