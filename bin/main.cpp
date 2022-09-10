#include "utap/utap.h"
#include "uppaalad.hpp"

#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <unordered_set>


int main (int argc, char* argv[]) {
  std::string model;
  std::unordered_set<std::string> attackertemplates;
  
  namespace po = boost::program_options;
  po::options_description description {"ADTree Composer"};
  description.add_options()
    ("help,h", "Display this message")
    ("atttemplate,a",po::value<std::vector<std::string>> ()->notifier([&attackertemplates](auto& s){attackertemplates.insert (s);}),"Attacker Templates")
    ("model",po::value<std::string> (&model),"Model")
    ;

  
  
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
  po::notify(vm);

  for (auto& s : attackertemplates) {
    std::cerr << s << std::endl;
  }
  
  std::cerr << "Parse" << model << std::endl;
  
  
  UTAP::Document document;
  UppaalAD::SystemCopier copier{std::unordered_set<std::string> {}};

  
  if (parseXMLFile (model.c_str(),&document,true,{std::filesystem::path {"."}}) == 0 ) {
    copier.copyDeclarations ("K_",document.getGlobals ());
    for (auto& t: document.getTemplates ()) {
      copier.copyTemplate ("K_",t);
      copier.copyAttackerTemplate ("K_",t,UppaalAD::AttType::Defender);
    }
    
    writeXMLFile ("out.xml",&copier.getDocument ());
    std::cerr << "Parsed " << std::endl;
  }

  else {
    std::cerr << "Fail Parsed " << model  << std::endl;
  
  }
}
