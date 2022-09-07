#ifndef _UPPAAL_AD__
#define _UPPAAL_AD__

#include "utap/document.h"
#include <memory>
#include <unordered_set>
#include <string>


namespace UppaalAD {
  class SystemCopier {
  public:
    SystemCopier (std::unordered_set<std::string>&&);
    ~SystemCopier ();
    bool copyGlobalDeclarations (const std::string&, const UTAP::declarations_t&);
    bool copyTemplate (const std::string&, const UTAP::template_t&);
    
    UTAP::Document& getDocument ();
  private:
    struct Inner;
    std::unique_ptr<Inner> _impl;
    std::unordered_set<std::string> attackerActions;
    void copyDeclarationTo (const std::string&, const UTAP::declarations_t&, UTAP::declarations_t& target);
  };
  
};

#endif
