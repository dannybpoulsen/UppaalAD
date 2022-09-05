#include "utap/common.h"
#include "utap/expression.h"

#include <cassert>
#include <type_traits>
#include <stdexcept>


namespace UppaalAD {
  template<UTAP::Constants::kind_t kind>
  struct is_binary : public std::false_type {};
  
  template<UTAP::Constants::kind_t kind> requires (kind == UTAP::Constants::kind_t::PLUS ||
		  kind == UTAP::Constants::kind_t::MINUS ||
		  kind == UTAP::Constants::kind_t::MULT ||
		  kind == UTAP::Constants::kind_t::DIV  ||
		  kind == UTAP::Constants::kind_t::BIT_AND ||
		  kind == UTAP::Constants::kind_t::BIT_OR ||
		  kind == UTAP::Constants::kind_t::BIT_XOR ||
		  kind == UTAP::Constants::kind_t::BIT_LSHIFT ||
		  kind == UTAP::Constants::kind_t::BIT_RSHIFT ||
		  kind == UTAP::Constants::kind_t::AND ||
		  kind == UTAP::Constants::kind_t::OR ||
		  kind == UTAP::Constants::kind_t::XOR ||
		  kind == UTAP::Constants::kind_t::OR ||
		  kind == UTAP::Constants::kind_t::LT ||
		  kind == UTAP::Constants::kind_t::LE ||
		  kind == UTAP::Constants::kind_t::EQ ||
		  kind == UTAP::Constants::kind_t::NEQ ||
		  kind == UTAP::Constants::kind_t::GE ||
		  kind == UTAP::Constants::kind_t::GT) 

  struct is_binary<kind> : public std::true_type  {
  };

  template<UTAP::Constants::kind_t kind>
  constexpr bool is_binary_v = is_binary<kind>::value;
  
  template<UTAP::Constants::kind_t kind>
  struct Expression {
    Expression (UTAP::expression_t& e) : expr(e) {
      assert(expr.getKind () == kind);
    }
    

    auto& getLeft ()  requires(is_binary_v<kind>) {return expr[0];}
    auto& getRight ()  requires(is_binary_v<kind>) {return expr[1];}
    auto getValue () const requires (kind ==UTAP::Constants::kind_t::CONSTANT) {return expr.getValue ();}
    
	
  private:
    UTAP::expression_t& expr;
  };


  
  template<class T, UTAP::Constants::kind_t k>
  concept Visitor = requires (T t, Expression<k> e) {
      t  (e);
  };


#define SUPPORTED_EXPR				\
  X(PLUS)					\
  X(MINUS)					\
  X(MULT)					\
  X(DIV)					\
  X(CONSTANT)					\

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
SUPPORTED_EXPR
      #undef X
     
    default:
      throw std::logic_error ("Unsupported");
    };
  }
  
}
