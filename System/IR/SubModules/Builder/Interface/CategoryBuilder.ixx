export module Foundation.IRSystem:Builder.CategoryBuilder;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:IR.Core.ObjectDescriptor;
import Foundation.IRSystem:IR.IR.FlowBuilder;
import Foundation.IRSystem:Builder.Capabilities.Synthesis;
import Foundation.IRSystem:IR.CPO;
import <fstream>;
import <sstream>;

export namespace Foundation::IRSystem {

struct Term { bool return_arg{false}; bool return_const_true{false}; };

inline Foundation::IRSystem::MorphismIR compile_full(const Term& t) {
    FlowBuilder fb; fb.ir.expected_args = 1;
    if (t.return_arg) { auto a0 = fb.arg(0); fb.ret(a0); }
    else if (t.return_const_true) { auto c = fb.add_const(true); fb.ret(c); }
    else { fb.ret_void(); }
    return fb.build();
}

export inline Foundation::IRSystem::MorphismIR compile_from_text(std::string_view text, const ObjectDescriptor& od) {
    FlowBuilder fb; fb.ir.expected_args = 1; auto obj = fb.arg(0);
    auto idx_of = [&](Foundation::IRSystem::TypeID rid){ int i = od.find_role_index(rid); return i < 0 ? -1 : static_cast<Foundation::IRSystem::UInt32>(i); };
    if (text == "HasValue()") { auto idx = idx_of(Foundation::IRSystem::GetTypeID<Foundation::IRSystem::get_has_value_index_tag>()); if (idx == static_cast<Foundation::IRSystem::UInt32>(-1)) { fb.ret_void(); return fb.build(); } auto val = fb.load_member(obj, idx); fb.ret(val); return fb.build(); }
    if (text == "Value()" || text == "UncheckedGet()") { auto idx = idx_of(Foundation::IRSystem::GetTypeID<Foundation::IRSystem::get_value_index_tag>()); if (idx == static_cast<Foundation::IRSystem::UInt32>(-1)) { fb.ret_void(); return fb.build(); } auto val = fb.load_member(obj, idx); fb.ret(val); return fb.build(); }
    if (text == "Reset()") { auto idx = idx_of(Foundation::IRSystem::GetTypeID<Foundation::IRSystem::get_has_value_index_tag>()); if (idx == static_cast<Foundation::IRSystem::UInt32>(-1)) { fb.ret_void(); return fb.build(); } auto zero = fb.add_const(false); fb.store_member(obj, idx, zero); fb.ret_void(); return fb.build(); }
    auto c = fb.add_const(true); fb.ret(c); return fb.build();
}

export inline Foundation::IRSystem::MorphismIR compile_from_text_file(const std::string& path, const ObjectDescriptor& od) {
    std::ifstream ifs(path); std::stringstream ss; ss << ifs.rdbuf();
    return compile_from_text(ss.str(), od);
}

}