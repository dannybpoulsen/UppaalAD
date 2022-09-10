#include "utap/utap.h"
#include "uppaalad.hpp"

#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>


int main (int argc, char* argv[]) {
  std::string model;
  std::string output;
  std::string sys1;
  std::string sys2;
  
  std::unordered_set<std::string> attackertemplates;
  std::unordered_set<std::string> attackersymbols;
  
  namespace po = boost::program_options;
  po::options_description description {"ADTree Composer"};
  description.add_options()
    ("help,h", "Display this message")
    ("atttemplate,a",po::value<std::vector<std::string>> (),"Attacker Templates")
    ("attsymbols,s",po::value<std::vector<std::string>> (),"Attacker Channels")
    
    ("model",po::value<std::string> (&model),"Model")
    ("sys1",po::value<std::string> (&sys1)->default_value("SYS1"),"System1 Name")
    ("sys2",po::value<std::string> (&sys2)->default_value("SYS2"),"System2 Name")
    
    ("output,o",po::value<std::string> (&output)->default_value ("out.xml"),"Output")
    ;

  
  
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
  po::notify(vm);

  if (vm.count("atttemplate")) {
    auto vals = vm["atttemplate"].as<std::vector<std::string>> ();
    std::copy (vals.begin(),vals.end(),std::inserter (attackertemplates,attackertemplates.begin()));
  }

  if (vm.count("attsymbols")) {
    auto vals = vm["attsymbols"].as<std::vector<std::string>> ();
    std::copy (vals.begin(),vals.end(),std::inserter (attackersymbols,attackersymbols.begin()));
  }
  
  for (auto& s : attackertemplates)
    std::cerr << s << std::endl;
  
  UTAP::Document document;
  UppaalAD::SystemCopier copier{std::move(attackersymbols)};

  
  if (parseXMLFile (model.c_str(),&document,true,{std::filesystem::path {"."}}) == 0 ) {
    copier.copyDeclarations (sys1,document.getGlobals (),true);
    copier.copyDeclarations (sys2,document.getGlobals (),false);
    
    for (auto& t: document.getTemplates ()) {
      if (attackertemplates.count (std::string {t.uid.getName ()})) {	  
	copier.copyAttackerTemplate (sys1,t,UppaalAD::AttType::Aggressor);
	copier.copyAttackerTemplate (sys2,t,UppaalAD::AttType::Defender);
	
      }
      else {
	copier.copyTemplate (sys1,t);
	copier.copyTemplate (sys2,t);
      }
    }
    writeXMLFile (output.c_str (),&copier.getDocument ());
    std::cerr << "Parsed " << std::endl;
  }

  else {
    std::cerr << "Fail Parsed " << model  << std::endl;
  
  }
}
