#include "uppaalad.hpp"
#include "ExprWrapper.hpp"
#include "utap/document.h"
#include "utap/statement.h"
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

    

    template<UTAP::Constants::kind_t kind>
    void operator() (Expression<kind> wrapper ) requires is_assign_v<kind> {
      visit (*this,wrapper.getAssignee ());
      visit (*this,wrapper.getExpr ());
      builder.exprBinary (kind);
    }

    template<UTAP::Constants::kind_t kind>
    void operator() (Expression<kind> wrapper ) requires is_nary_v<kind> {
      for (std::size_t i = 0; i < wrapper.size ();++i) {
	visit (*this,wrapper[i]);
      }
      
      builder.exprNary (kind,wrapper.size ());
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

    
    template<UTAP::Constants::kind_t kind>
    void operator() (UppaalAD::Expression<kind> wrapper ) requires is_unary_v<kind> {
      visit (*this,wrapper.getInner ());
      builder.exprUnary (kind);
    }

    
    
  private:
    const std::unordered_set<std::string>& attackerActions;
    UTAP::DocumentBuilder& builder;
    const std::string pref;
  };

  class StatementModifier : private UTAP::StatementVisitor {
    public:
        StatementModifier (std::string pref,UTAP::DocumentBuilder& builder, ExpressionModifier& mod,const std::unordered_set<std::string>& attackerActions) : pref(pref),exprMod(mod),builder(builder),attackerActions(attackerActions) {}
    
    void BuildStatement (UTAP::Statement* stmt) {
      stmt->accept (this);
    }
    
    int32_t visitEmptyStatement(UTAP::EmptyStatement* stat) override {
      builder.emptyStatement ();
      return 0;
    }
    
    int32_t visitExprStatement(UTAP::ExprStatement* stat) override {
      std::cerr << "In visit" << stat->toString ("ML") << std::endl;
      exprMod.Modify (stat->expr);
      builder.exprStatement ();
      return 0;
    }
    int32_t visitAssertStatement(UTAP::AssertStatement* stat) override {
      exprMod.Modify (stat->expr);
      builder.assertStatement ();
      return 0;
    }
    int32_t visitForStatement(UTAP::ForStatement* stat) override {
      builder.forBegin ();
      exprMod.Modify (stat->init);
      exprMod.Modify (stat->cond);
      exprMod.Modify (stat->step);
      stat->stat.get()->accept (this);
      builder.forEnd ();
      return 0;
    }
    
    int32_t visitIterationStatement(UTAP::IterationStatement* stat) override {return 0;}
    int32_t visitWhileStatement(UTAP::WhileStatement* stat) override {
      builder.whileBegin ();
      exprMod.Modify (stat->cond);
      std::cerr << "Build" << stat->stat.get ()->toString ("PP");
      stat->stat.get()->accept (this);
      builder.whileEnd ();
      return 0;
    }
    int32_t visitDoWhileStatement(UTAP::DoWhileStatement* stat) override {return 0;}
    int32_t visitBlockStatement(UTAP::BlockStatement* stat) override {
      builder.blockBegin ();
      for (auto& symbol : stat->getFrame ()) {
	builder.typePush (symbol.getType ());
	builder.addPosition (0,0,0,"");
	bool initialiser = false;
	if (!symbol.getType ().isChannel () || !attackerActions.count(symbol.getName ()))    
	  builder.declVar ((pref+symbol.getName ()).c_str (),initialiser); 
	else 
	  builder.declVar (symbol.getName ().c_str(),initialiser);
      }
      for (auto& bb : *stat) {
	bb->accept(this);
      }
      builder.blockEnd ();
      return 0;
    }
    int32_t visitSwitchStatement(UTAP::SwitchStatement* stat) override {return 0;}
    int32_t visitCaseStatement(UTAP::CaseStatement* stat) override {return 0;}
    int32_t visitDefaultStatement(UTAP::DefaultStatement* stat) override {return 0;}
    int32_t visitIfStatement(UTAP::IfStatement* stat) override {return 0;}
    int32_t visitBreakStatement(UTAP::BreakStatement* stat) override {return 0;}
    int32_t visitContinueStatement(UTAP::ContinueStatement* stat) override {return 0;}
    int32_t visitReturnStatement(UTAP::ReturnStatement* stat) override {
      bool val = false;
      if (!stat->value.empty ()) {
	exprMod.Modify (stat->value);
	val = true;
      }
      builder.returnStatement (val);
      return 0;
    }
  private:
    const std::string pref;
    ExpressionModifier& exprMod;
    UTAP::DocumentBuilder& builder;
    const std::unordered_set<std::string> attackerActions;
  };
  
  
  SystemCopier::SystemCopier (std::unordered_set<std::string>&& s) : attackerActions(std::move(s))  {
    _impl = std::make_unique<Inner> ();
  }
  
  SystemCopier::~SystemCopier () {
    
  }

  UTAP::Document& SystemCopier::getDocument () {
    return _impl->doc;
  }

  bool SystemCopier::copyFunction (const std::string& pref, const UTAP::function_t& function) {
    auto namer = [pref](const auto& orig){return pref+orig;};
    ExpressionModifier modifier (_impl->builder,pref,attackerActions);
    StatementModifier stmtMod (pref,_impl->builder,modifier,attackerActions);
    //COPY functions

    auto type = function.uid.getType ();

    auto rettype = type[0];
    for (int i = 1; i<type.size (); ++i) {
      auto paramtype = type[i];
      auto label = type.getLabel (i);
      _impl->builder.typePush (paramtype);
      _impl->builder.declParameter (namer(label).c_str (),false);
    }
    _impl->builder.typePush (rettype);
    
    _impl->builder.declFuncBegin (namer (function.uid.getName ()).c_str());

    
    for (std::size_t i = type.size ()-1; i < function.body->getFrame().getSize (); ++i) {
      auto& symbol = function.body->getFrame()[i];
      _impl->builder.typePush (symbol.getType ());
      _impl->builder.addPosition (0,0,0,"");
      bool initialiser = false;
      if (!symbol.getType ().isChannel () || !attackerActions.count(symbol.getName ()))    
	_impl->builder.declVar ((pref+symbol.getName ()).c_str (),initialiser); 
      else 
	_impl->builder.declVar (symbol.getName ().c_str(),initialiser);
    }
    
    for(auto& stmt : *function.body)
      stmtMod.BuildStatement (stmt.get ());
    
    _impl->builder.declFuncEnd ();
    
    
    std::cerr << type << std::endl;
    
  
    
    return true;
  }
    

 const  std::unordered_set<std::string> blacklist_symbols {
   "INT8_MIN",
   "INT8_MAX",
   "UINT8_MAX",
   "UINT16_MAX",
   "INT16_MIN",
   "INT16_MAX",
   "INT32_MIN",
   "INT32_MAX",
   "FLT_MIN",
   "FLT_MAX",
   "DBL_MIN",
   "DBL_MAX" ,
   "M_PI" ,
   "M_PI_2" ,
   "M_PI_4" ,
   "M_E" ,
   "M_LOG2E" ,
   "M_LOG10E" ,
   "M_LN2" ,
   "M_LN10" ,
   "M_1_PI" ,
   "M_2_PI" ,
   "M_2_SQRTPI" ,
   "M_SQRT2" ,
   "M_SQRT1_2"
 };
  
  bool SystemCopier::copyDeclarations (const std::string& pref, const UTAP::declarations_t& d,bool copyAtt,std::size_t skip) {
    ExpressionModifier modifier (_impl->builder,pref,attackerActions);
    std::size_t i = 0; 
    for (auto& var : d.variables) {
      if (i++ < skip)
	continue;
      auto& symbol = var.uid;
      if (blacklist_symbols.count(symbol.getName ()))
	continue;
      _impl->builder.typePush (symbol.getType ());
      _impl->builder.addPosition (0,0,0,"");
      bool initialiser = false;
      if (!var.expr.empty ()) {
	modifier.Modify (var.expr);
	initialiser = true;
      }
      if (!symbol.getType ().isChannel () || !attackerActions.count(symbol.getName ()))    
	_impl->builder.declVar ((pref+symbol.getName ()).c_str (),initialiser); 
      else {
	if (copyAtt)
	  _impl->builder.declVar (symbol.getName ().c_str(),initialiser);
      }
    }
    
    for (auto& f : d.functions) {
      copyFunction (pref,f);
    }
    
    return true;
  }


  bool SystemCopier::copyAttackerTemplate (const std::string& pref, const UTAP::template_t& templ, AttType type) {
    if (type == AttType::Aggressor) {
      return copyTemplate (pref,templ);
    }
    
    ExpressionModifier modifier (_impl->builder,pref,attackerActions);
    auto namer = [pref](const auto& orig){return pref+orig.getName();};
    
    _impl->builder.procBegin ((pref+templ.uid.getName ()).c_str(),templ.isTA,templ.type,templ.mode);
    for (auto& t : templ.parameters) {
      _impl->builder.typePush (t.getType ());
      _impl->builder.declParameter (namer (t).c_str(),false);
    }
    
    copyDeclarations (pref,templ,false);    
    _impl->builder.procState ("RefineFail",false,false);

   
    
    for (auto&  state : templ.states) {      
      bool hasInvariant = modifier.Modify (state.invariant);
      bool hasExponentialRate = modifier.Modify (state.exponentialRate);
      _impl->builder.procState (namer(state.uid).c_str (),hasInvariant,hasExponentialRate);
    }

    for (auto& edge : templ.edges) {
      bool refineEdge = false;
      UTAP::symbol_t beg;
      UTAP::symbol_t end;
      
      if (edge.src)
	beg = edge.src->uid;
      else
	beg = edge.srcb->uid;
      if (edge.dst)
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
      
	if (edge.sync.getSync () == UTAP::Constants::synchronisation_t::SYNC_QUE) {
	  modifier.Modify (edge.sync);
	}

	else {
	  modifier.Modify (edge.sync[0]);
	  _impl->builder.procSync (UTAP::Constants::synchronisation_t::SYNC_QUE);
	  refineEdge = true;
	}
      }
      
      
      
      
      _impl->builder.procEdgeEnd (namer(beg).c_str(),
				  namer(end).c_str());
      if (refineEdge) {
	_impl->builder.procEdgeBegin (namer(beg).c_str (),"RefineFail",edge.control,edge.actname.c_str());
	modifier.Modify (edge.guard);
	_impl->builder.exprUnary (UTAP::Constants::kind_t::NOT);
	_impl->builder.procGuard ();
	
	modifier.Modify (edge.sync[0]);
	_impl->builder.procSync (UTAP::Constants::synchronisation_t::SYNC_QUE);  
	
	_impl->builder.procEdgeEnd (namer(beg).c_str (),"RefineFail");
	
      }

    }
      
    _impl->builder.procStateInit (namer(templ.init).c_str ());
    
    _impl->builder.procEnd ();
    return true;
    
  }
    
  
  bool SystemCopier::copyTemplate (const std::string& pref, const UTAP::template_t& templ) {
    ExpressionModifier modifier (_impl->builder,pref,attackerActions);
    auto namer = [pref](const auto& orig){return pref+orig.getName();};
    for (auto& t : templ.parameters) {
      _impl->builder.typePush (t.getType ());
      _impl->builder.declParameter (namer (t).c_str(),false);
    }
    _impl->builder.procBegin ((pref+templ.uid.getName ()).c_str(),templ.isTA,templ.type,templ.mode);
    
    copyDeclarations (pref,templ,false);
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
      if (edge.dst)
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

