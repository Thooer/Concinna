export module Foundation.IRSystem:Builder.Frontend.TextFrontend;
import Foundation.IRSystem:IR.Term.Term;
import Foundation.IRSystem:Builder.Capabilities.Synthesis;
import <string_view>;

export namespace Foundation::IRSystem::frontend {
export inline Foundation::IRSystem::term::Node Parse(std::string_view sv) {
  using namespace Foundation::IRSystem::synthesis;
  if (sv == "HasValue()") return synthesize_has_value();
  if (sv == "Value()") return synthesize_value();
  if (sv == "UncheckedGet()") return synthesize_unchecked_get();
  if (sv == "Reset()") return synthesize_reset();
  return synthesize_value();
}
}