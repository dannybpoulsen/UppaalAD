#include "utap/utap.h"
#include "uppaalad.hpp"

#include <iostream>



int main (int argc, char* argv[]) {
  UTAP::Document document;
  UppaalAD::SystemCopier copier{std::unordered_set<std::string> {}};
  
  if (parseXMLFile (argv[1],&document,true,{std::filesystem::path {"."}}) == 0 ) {
    copier.copyGlobalDeclarations ("K_",document.getGlobals ());
    for (auto& t: document.getTemplates ())
      copier.copyTemplate ("K_",t);
    writeXMLFile ("out.xml",&copier.getDocument ());
    std::cerr << "Parsed " << std::endl;
  }

  else {
    std::cerr << "Fail Parsed " << argv[1]  << std::endl;
  
  }
}
