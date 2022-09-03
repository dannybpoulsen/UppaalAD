#include "utap/common.h"
#include "utap/expression.h"

#include <type_traits>


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
  using  is_binary_v = is_binary<kind>::value;
  
  template<UTAP::Constants::kind_t>
  struct Expression {
    Expression (UTAP::expression_t& e) : expr(e) {}
    
  private:
  const UTAP::expression_t& expr;
  };
  

  
  
}
