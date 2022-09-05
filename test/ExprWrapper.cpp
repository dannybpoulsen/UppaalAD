#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "utap/expression.h"
#include "ExprWrapper.hpp"

TEST_CASE ("Blah") {
  auto left = UTAP::expression_t::createConstant (1);
  auto right = UTAP::expression_t::createConstant (2);
  
  auto expr = UTAP::expression_t::createBinary (UTAP::Constants::kind_t::PLUS,left,right);

  UppaalAD::Expression<UTAP::Constants::kind_t::PLUS> wrapper {expr};
  
  CHECK (wrapper.getLeft ().equal(expr[0]));
  CHECK (wrapper.getRight ().equal(expr[1]));
  
}


TEST_CASE ("Visitor") {
  auto left = UTAP::expression_t::createConstant (1);
  auto right = UTAP::expression_t::createConstant (2);
  
  auto expr = UTAP::expression_t::createBinary (UTAP::Constants::kind_t::PLUS,left,right);
  
  UppaalAD::visit (UppaalAD::overloaded {
      [expr](UppaalAD::Expression<UTAP::Constants::kind_t::PLUS> wrapper) {
	CHECK (wrapper.getLeft ().equal(expr[0]));
	CHECK (wrapper.getRight ().equal(expr[1]));
	
      }
	}
    
    , expr);
}


TEST_CASE ("Visitor VM") {
  auto left = UTAP::expression_t::createConstant (1);
  auto right = UTAP::expression_t::createConstant (2);
  
  auto expr = UTAP::expression_t::createBinary (UTAP::Constants::kind_t::PLUS,left,right);

  
  struct {
    auto operator () (UppaalAD::Expression<UTAP::Constants::kind_t::PLUS> wrapper) -> int {
      return UppaalAD::visit (*this,wrapper.getLeft()) + UppaalAD::visit (*this,wrapper.getRight ());
    }

    auto operator () (UppaalAD::Expression<UTAP::Constants::kind_t::CONSTANT> wrapper) ->int {
      return wrapper.getValue ();
    }
    
  } visitor;

  
  CHECK (UppaalAD::visit (visitor
			  , expr) == 3);
  
}
