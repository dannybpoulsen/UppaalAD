#ifndef _EXPRWRAPPER__
#define _EXPRWRAPPER__

#include "utap/common.h"
#include "utap/expression.h"

#include <cassert>
#include <type_traits>
#include <stdexcept>


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
  X(GT)

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
  struct is_constant : public std::false_type {};

  template<>
  struct is_constant<UTAP::Constants::kind_t::CONSTANT> : public std::true_type {};

  template<UTAP::Constants::kind_t kind>
  constexpr bool is_constant_v = is_constant<kind>::value;
  
  

  
  template<UTAP::Constants::kind_t kind>
  struct Expression {
    Expression (UTAP::expression_t& e) : expr(e) {
      assert(expr.getKind () == kind);
    }
    
    auto& getType () const {return expr.getType ();}
    auto getName () const requires (kind == UTAP::Constants::kind_t::IDENTIFIER) {
      return expr.getSymbol().getName ();
    }
    
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
	X(IDENTIFIER)
	X(SYNC)
#undef X
     
    default:
      throw std::logic_error ("Unsupported");
    };
  }
  
}


#endif
