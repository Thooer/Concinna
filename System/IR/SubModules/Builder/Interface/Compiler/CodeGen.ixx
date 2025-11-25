#pragma once
import Foundation.IRSystem;

namespace Foundation::IRSystem {

inline Foundation::IRSystem::MorphismIR compile_full(const Foundation::IRSystem::Node& term, const Foundation::IRSystem::ObjectDescriptor& od) {
  auto lowered = LoweringPipeline::run(term);
  auto fi = Foundation::IRSystem::build_flow_from_term_with_object_multi(lowered, od);
  return std::move(fi.ir);
}

}