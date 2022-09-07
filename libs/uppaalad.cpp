#include "uppaalad.hpp"
#include "ExprWrapper.hpp"
#include "utap/document.h"
#include "utap/DocumentBuilder.hpp"

#include <string>
#include <unordered_set>
#include <iostream>

namespace UppaalAD {

  struct SystemCopier::Inner {
    Inner () : builder(doc) {}
    UTAP::Document doc;
    UTAP::DocumentBuilder builder;
  };
  
  SystemCopier::SystemCopier (std::unordered_set<std::string>&& s) : attackerActions(std::move(s))  {
    _impl = std::make_unique<Inner> ();
  }
  
  SystemCopier::~SystemCopier () {
    
  }

  UTAP::Document& SystemCopier::getDocument () {
    return _impl->doc;
  }

  bool SystemCopier::copyGlobalDeclarations (const std::string& pref, const UTAP::declarations_t& d) {
    for (auto& var : d.variables) {
      auto& symbol = var.uid;
      _impl->builder.typePush (symbol.getType ());
      _impl->builder.addPosition (0,0,0,"");
      if (!symbol.getType ().isChannel () || !attackerActions.count(symbol.getName ()))  
	_impl->builder.declVar ((pref+symbol.getName ()).c_str (),false);
      else
	_impl->builder.declVar (symbol.getName ().c_str(),false);
    }
    return true;
  }

  class ExpressionModifier {
  public:
    ExpressionModifier (UTAP::DocumentBuilder& builder,
			std::string pref,
			const std::unordered_set<std::string>& attackerActions) : attackerActions(attackerActions),
										  builder(builder),
										  pref(std::move(pref))
    {}

    bool Modify (UTAP::expression_t expr) {
      if (expr.empty()) {
	std::cerr << "Empty" << std::endl;
	return false;
      }
      else {
	visit (*this,expr);
	return true;
      }
    }
    
    template<UTAP::Constants::kind_t kind>
    void operator() (Expression<kind> wrapper ) requires is_binary_v<kind> {
      visit (*this,wrapper.getLeft ());
      visit (*this,wrapper.getRight ());
      builder.exprBinary (kind);
    }

    void operator() (UppaalAD::Expression<UTAP::Constants::kind_t::CONSTANT> wrapper ) {
      if (wrapper.isDouble ()) {
	builder.exprDouble (wrapper.getDoubleValue ());
      }
      else {
	builder.exprNat (wrapper.getValue ());
      }
    }

    void operator() (UppaalAD::Expression<UTAP::Constants::kind_t::IDENTIFIER> wrapper ) {
      if (attackerActions.count (wrapper.getName ()))
	builder.exprId (wrapper.getName ().c_str());
      else {
	builder.exprId ((pref+wrapper.getName ()).c_str());
      }
    }

    void operator() (UppaalAD::Expression<UTAP::Constants::kind_t::SYNC> wrapper ) {
      visit (*this,wrapper.getChannelExpr ());
      builder.procSync (wrapper.getSyncType ());
    }
    
  private:
    const std::unordered_set<std::string>& attackerActions;
    UTAP::DocumentBuilder& builder;
    const std::string pref;
  };
  
  bool SystemCopier::copyTemplate (const std::string& pref, const UTAP::template_t& templ) {
    ExpressionModifier modifier (_impl->builder,pref,attackerActions);
    auto namer = [pref](const auto& orig){return pref+orig.getName();};
    
    _impl->builder.procBegin ((pref+templ.uid.getName ()).c_str(),templ.isTA,templ.type,templ.mode);

    for (auto&  state : templ.states) {
      
      bool hasInvariant = modifier.Modify (state.invariant);
      bool hasExponentialRate = modifier.Modify (state.exponentialRate);
      _impl->builder.procState (namer(state.uid).c_str (),hasInvariant,hasExponentialRate);
    }

    for (auto& edge : templ.edges) {
      UTAP::symbol_t beg;
      UTAP::symbol_t end;
      
      if (edge.src)
	beg = edge.src->uid;
      else
	beg = edge.srcb->uid;
      if (edge.src)
	end = edge.dst->uid;
      else
	end = edge.dstb->uid;
      
      _impl->builder.procEdgeBegin (namer(beg).c_str(),
				    namer(end).c_str(),
				    edge.control,
				    edge.actname.c_str ());

      modifier.Modify (edge.guard);
      _impl->builder.procGuard ();

      modifier.Modify (edge.assign);
      _impl->builder.procUpdate ();

      modifier.Modify (edge.prob);
      _impl->builder.procProb ();

      if (!edge.sync.empty ()) {
	modifier.Modify (edge.sync);
      }
      
      
      _impl->builder.procEdgeEnd (namer(beg).c_str(),
				  namer(end).c_str());
    }
    _impl->builder.procStateInit (namer(templ.init).c_str ());
    
    _impl->builder.procEnd ();
    return true;
  }

    

  
  
  
}

