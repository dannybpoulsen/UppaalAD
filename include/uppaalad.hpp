#ifndef _UPPAAL_AD__
#define _UPPAAL_AD__

#include "utap/document.h"
#include "utap/builder.h"

#include <memory>
#include <unordered_set>
#include <string>


namespace UppaalAD {
  enum class AttType {
    Aggressor,
    Defender
  };

  class ExpressionModifier;
  
  class SystemCopier {
  public:
    SystemCopier (std::unordered_set<std::string>&&);
    ~SystemCopier ();
    bool copyDeclarations (const std::string&, const UTAP::declarations_t&,bool copyAttackerSymbols,std::size_t skipFirst = 0);
    bool copyTemplate (const std::string&, const UTAP::template_t&);
    bool copyAttackerTemplate (const std::string&, const UTAP::template_t&,AttType );
    void copyType (const UTAP::type_t& t, ExpressionModifier&, UTAP::ParserBuilder::PREFIX = UTAP::ParserBuilder::PREFIX::PREFIX_NONE);
    void copyInstance (const UTAP::instance_t& instance, const std::string& pref);
    UTAP::Document& getDocument ();
  private:
    bool copyFunction (const std::string&, const UTAP::function_t&);
    void declParameter (const std::string&, const UTAP::type_t&,ExpressionModifier&);
    struct Inner;
    std::unique_ptr<Inner> _impl;
    std::unordered_set<std::string> attackerActions;
    void copyDeclarationTo (const std::string&, const UTAP::declarations_t&, UTAP::declarations_t& target);
  };
  
};

#endif
