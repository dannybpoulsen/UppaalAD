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

    template<UTAP::Constants::kind_t kind>
    void operator() (UppaalAD::Expression<kind> wrapper ) requires (is_builtin_1_v<kind>) {
      visit (*this,wrapper.template getParam<0> ());
      builder.exprBuiltinFunction1 (kind);
    }

    void operator() (UppaalAD::Expression<UTAP::Constants::kind_t::DOT> wrapper ) {
      auto base = wrapper.getBase ();
      auto label = wrapper.indexName ();
      visit (*this,base);
      builder.exprDot (label.c_str());
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
      stat->stat.get()->accept (this);
      builder.whileEnd ();
      return 0;
    }
    int32_t visitDoWhileStatement(UTAP::DoWhileStatement* stat) override {
      builder.doWhileBegin ();
      stat->stat.get()->accept (this);
      exprMod.Modify (stat->cond);
      builder.doWhileEnd ();
      return 0;
    }
    
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
    int32_t visitSwitchStatement(UTAP::SwitchStatement* stat) override {
      throw std::logic_error ("Switch Unsupported");
    }
    int32_t visitCaseStatement(UTAP::CaseStatement* stat) override {
      throw std::logic_error ("Case Unsupported");	
    }
    int32_t visitDefaultStatement(UTAP::DefaultStatement* stat) override {
      throw std::logic_error ("Default Unsupported");	
    
    }
    int32_t visitIfStatement(UTAP::IfStatement* stat) override {
      builder.ifBegin ();
      exprMod.Modify (stat->cond);
      builder.ifCondition ();
      stat->trueCase->accept (this);
      
      if (stat->falseCase) {
	stat->falseCase->accept (this);
      }

      builder.ifEnd (stat->falseCase.get());

      return 0;  
    }
    int32_t visitBreakStatement(UTAP::BreakStatement* stat) override {
      throw std::logic_error ("Break Unsupported");
    }
    int32_t visitContinueStatement(UTAP::ContinueStatement* stat) override {
      throw std::logic_error ("Break Unsupported");
    }
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
      copyType(paramtype,modifier);
      _impl->builder.declParameter (namer(label).c_str (),false);
    }
    copyType (rettype,modifier);
    
    _impl->builder.declFuncBegin (namer (function.uid.getName ()).c_str());

    
    for (std::size_t i = type.size ()-1; i < function.body->getFrame().getSize (); ++i) {
      auto& symbol = function.body->getFrame()[i];
      copyType (symbol.getType (),modifier);
      
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

  bool isChannelType (auto&& type) {
    return (type.isChannel () ||
	    type.isArray () && type.getSub().isChannel ());;
  }

  void SystemCopier::copyType (const UTAP::type_t& t, ExpressionModifier& mod, UTAP::ParserBuilder::PREFIX prefix) {
    if (t.isPrefix ()) {
      if (t.isConstant()) {
	prefix = UTAP::ParserBuilder::PREFIX_CONST;
      }
      else if (t.is (UTAP::Constants::URGENT) &&
	       t.is (UTAP::Constants::BROADCAST) ) {
	prefix = UTAP::ParserBuilder::PREFIX_URGENT_BROADCAST;
      }

      else if (t.is (UTAP::Constants::URGENT) ) {
	prefix = UTAP::ParserBuilder::PREFIX_URGENT;
      }

      else if (t.is (UTAP::Constants::BROADCAST) ) {
	prefix = UTAP::ParserBuilder::PREFIX_BROADCAST;
      }

      else {
	std::logic_error ("Unsupported Prefix");
    
      }
      
    }
    
    if (t.is (UTAP::Constants::LABEL)) {
      copyType (t.get(0),mod,prefix);
    }
    
    else if (t.isRange ()) {
      auto exprs = t.getRange ();
      mod.Modify (exprs.first  );
      mod.Modify (exprs.second );
      
      _impl->builder.typeBoundedInt (prefix);  
    }
    
    else if (t.isInteger ()) {
      _impl->builder.typeInt (prefix);
    }

    else if (t.isDouble ()) {
      _impl->builder.typeDouble (prefix); 
    }

    else if (t.isBoolean ()) {
      _impl->builder.typeBool (prefix); 
    }

    else if (t.isString ()) {
       _impl->builder.typeString (prefix);
       
    }

    else if (t.isArray ()) {
      copyType (t.getSub (),mod);
      copyType (t.getArraySize (),mod);
      
      _impl->builder.typeArrayOfType (1);
      
    }

    else if (t.isRecord ()) {
      for (int i = 0; i < t.getRecordSize (); ++i) {
	copyType (t.getSub (i),mod);
	_impl->builder.structField (t.getRecordLabel (i).c_str ());
      }
      _impl->builder.typeStruct (prefix,t.getRecordSize ());
      
    }

    else if (t.isChannel ()) {
      _impl->builder.typeChannel (prefix);
      
    }

    

    else if (t.isClock()) {
      _impl->builder.typeClock (prefix);
    }

    else if (t.isVoid()) {
      _impl->builder.typeVoid ();
    }
    
    else {
      throw std::logic_error ("Unsupported Type");	
    }

    
  }
    
  
  bool SystemCopier::copyDeclarations (const std::string& pref, const UTAP::declarations_t& d,bool copyAtt,std::size_t skip) {
    ExpressionModifier modifier (_impl->builder,pref,attackerActions);
    std::size_t i = 0; 
    for (auto& var : d.variables) {
      if (i++ < skip)
	continue;
      auto& symbol = var.uid;
      if (blacklist_symbols.count(symbol.getName ()))
	continue;
      
      _impl->builder.addPosition (0,0,0,"");
      bool initialiser = false;
      if (!var.expr.empty ()) {
	modifier.Modify (var.expr);
	initialiser = true;
      }
      copyType (symbol.getType (),modifier);
      if (!isChannelType(symbol.getType ())) {
	_impl->builder.declVar ((pref+symbol.getName ()).c_str (),initialiser); 
      }
      else {
	if (!attackerActions.count(symbol.getName ())) {   
	  _impl->builder.declVar ((pref+symbol.getName ()).c_str (),initialiser);
	    
	}
	else if (copyAtt) {
	  _impl->builder.declVar (symbol.getName ().c_str(),initialiser);
	}
      }
    }
    
    for (auto& f : d.functions) {
      copyFunction (pref,f);
    }
    
    return true;
  }

  void SystemCopier::copyInstance (const UTAP::instance_t& instance,const std::string& pref) {
    ExpressionModifier modifier (_impl->builder,pref,attackerActions);
    auto namer = [pref](const auto& orig){return pref+orig.getName();};

    for (auto& p : instance.parameters) {
     modifier.Modify (instance.mapping.at(p));
    }
  
    if (instance.unbound)
      throw std::runtime_error ("No partial instantiations are4 allowed");
    
    
    if (instance.arguments != instance.parameters.getSize () )
      throw std::runtime_error ("No arguments are allowed");

    _impl->builder.instantiationBegin (namer (instance.uid).c_str(),0,namer(instance.templ->uid).c_str());
    _impl->builder.instantiationEnd (namer (instance.uid).c_str(),0,namer(instance.templ->uid).c_str(),instance.arguments);
    _impl->builder.process (namer (instance.uid).c_str());
    
    std::cerr << instance.uid.getName () << std::endl;
  }

  bool SystemCopier::copyAttackerTemplate (const std::string& pref, const UTAP::template_t& templ, AttType type) {
    if (type == AttType::Aggressor) {
      return copyTemplate (pref,templ);
    }
    
    ExpressionModifier modifier (_impl->builder,pref,attackerActions);
    auto namer = [pref](const auto& orig){return pref+orig.getName();};
    
    _impl->builder.procBegin ((pref+templ.uid.getName ()).c_str(),templ.isTA,templ.type,templ.mode);
    for (auto& t : templ.parameters) {
      copyType (t.getType (),modifier);
      _impl->builder.declParameter (namer (t).c_str(),false);
    }
    
    copyDeclarations (pref,templ,false);    
    _impl->builder.procState ("RefineFail",false,false);

    
    
    for (auto&  state : templ.states) {      
      bool hasInvariant = modifier.Modify (state.invariant);
      bool hasExponentialRate = modifier.Modify (state.exponentialRate);
      _impl->builder.procState (namer(state.uid).c_str (),hasInvariant,hasExponentialRate);

      if (state.uid.getType().is (UTAP::Constants::COMMITTED)) {
	_impl->builder.procStateCommit (namer(state.uid).c_str ());
      }

      if (state.uid.getType().is (UTAP::Constants::URGENT)) {
	_impl->builder.procStateUrgent (namer(state.uid).c_str ());
      }
      
    }

    for (auto&  branch : templ.branchpoints) {
      _impl->builder.procBranchpoint (namer(branch.uid).c_str ());
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


      for (auto& t : edge.select) {
	copyType (t.getType (),modifier);
      
	_impl->builder.procSelect (namer (t).c_str());
      }
      
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
	  if (edge.sync.getSync () == UTAP::Constants::synchronisation_t::SYNC_BANG &&
	      !attackerActions.count(edge.sync[0].getSymbol ().getName ()) 
	      ){
	    modifier.Modify (edge.sync);
	    
	  }
	  else {
	      
	    modifier.Modify (edge.sync[0]);
	    _impl->builder.procSync (UTAP::Constants::synchronisation_t::SYNC_QUE);
	    refineEdge = true;
	  }
	}
      }
      
      
      
      
      _impl->builder.procEdgeEnd (namer(beg).c_str(),
				  namer(end).c_str());
      if (refineEdge) {
	_impl->builder.procEdgeBegin (namer(beg).c_str (),"RefineFail",edge.control,edge.actname.c_str());

	for (auto& t : edge.select) {
	  copyType (t.getType (),modifier);
	  _impl->builder.procSelect (namer (t).c_str());
	}
	
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
      copyType (t.getType (),modifier);
      _impl->builder.declParameter (namer (t).c_str(),false);
    }
    _impl->builder.procBegin ((pref+templ.uid.getName ()).c_str(),templ.isTA,templ.type,templ.mode);
    
    copyDeclarations (pref,templ,false);
    for (auto&  state : templ.states) {
      bool hasInvariant = modifier.Modify (state.invariant);
      bool hasExponentialRate = modifier.Modify (state.exponentialRate);
      _impl->builder.procState (namer(state.uid).c_str (),hasInvariant,hasExponentialRate);

      if (state.uid.getType().is (UTAP::Constants::COMMITTED)) {
	_impl->builder.procStateCommit (namer(state.uid).c_str ());
      }

      if (state.uid.getType().is (UTAP::Constants::URGENT)) {
	_impl->builder.procStateUrgent (namer(state.uid).c_str ());
      }
      
    }

    for (auto&  branch : templ.branchpoints) {
      _impl->builder.procBranchpoint (namer(branch.uid).c_str ());
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

      for (auto& t : edge.select) {
	copyType(t.getType (),modifier);
	_impl->builder.procSelect (namer (t).c_str());
      }
      
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

