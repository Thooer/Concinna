export module Foundation.IRSystem:Builder.Capabilities.Synthesis;
import Foundation.IRSystem:IR.Term.Term;
import Foundation.IRSystem:IR.Capabilities;
import <variant>;

export namespace Foundation::IRSystem::synthesis {

inline Foundation::IRSystem::term::Node synthesize_has_value() { return Foundation::IRSystem::term::make_perform_tag<Foundation::IRSystem::HasValueTag>(); }
inline Foundation::IRSystem::term::Node synthesize_value() { return Foundation::IRSystem::term::make_perform_tag<Foundation::IRSystem::ValueTag>(); }
inline Foundation::IRSystem::term::Node synthesize_unchecked_get() { return Foundation::IRSystem::term::make_perform_tag<Foundation::IRSystem::UncheckedGetTag>(); }
inline Foundation::IRSystem::term::Node synthesize_emplace(const std::variant<bool,int,double,void*>& v) { Foundation::IRSystem::term::Node c; c.kind = Foundation::IRSystem::term::Kind::Const; c.data = Foundation::IRSystem::term::Const{ v }; return Foundation::IRSystem::term::make_perform_tag<Foundation::IRSystem::EmplaceTag>({ std::move(c) }); }
inline Foundation::IRSystem::term::Node synthesize_reset() { return Foundation::IRSystem::term::make_perform_tag<Foundation::IRSystem::ResetTag>(); }
inline Foundation::IRSystem::term::Node synthesize_iter_begin() { return Foundation::IRSystem::term::make_perform_tag<Foundation::IRSystem::BeginIterTag>(); }
inline Foundation::IRSystem::term::Node synthesize_iter_has_next() { return Foundation::IRSystem::term::make_perform_tag<Foundation::IRSystem::HasNextTag>(); }
inline Foundation::IRSystem::term::Node synthesize_iter_get_current() { return Foundation::IRSystem::term::make_perform_tag<Foundation::IRSystem::GetCurrentTag>(); }
inline Foundation::IRSystem::term::Node synthesize_iter_advance() { return Foundation::IRSystem::term::make_perform_tag<Foundation::IRSystem::AdvanceTag>(); }
inline Foundation::IRSystem::term::Node synthesize_iter_foreach_sum() { return Foundation::IRSystem::term::make_perform_tag<Foundation::IRSystem::GetSumTag>(); }

}