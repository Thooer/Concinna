export module Foundation.IRSystem:IR.IR.FlowPipeline;
import Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:IR.IR.MorphismIRMetadata;
import Foundation.IRSystem:IR.Core.ObjectDescriptor;
import Foundation.IRSystem:IR.Core.TypeMeta;
import Foundation.IRSystem:IR.CPO;
import Foundation.IRSystem:IR.Term.Term;
import Foundation.IRSystem:IR.Capabilities;
import Foundation.IRSystem:IR.IR.FlowBuilder;
import <variant>;
import <vector>;
import <optional>;

export namespace Foundation::IRSystem {

enum class PGKind { ReturnRole, ReturnCmpLT_Roles, ReturnLoadElem_Roles, EmplaceConst, ResetHas, AdvanceIndex, ReturnSum_Roles, ComputeRole, IfReturnConst, StoreRole, ComputeLoadElem_Roles, ComputeAdd_I32_Roles, ComputeCmp_LT_Roles };
struct PG_ReturnRole { TypeID role; };
struct PG_ReturnCmpLT_Roles { TypeID lhs_role; TypeID rhs_role; };
struct PG_ReturnLoadElem_Roles { TypeID base_role; TypeID index_role; };
struct PG_EmplaceConst { TypeID value_role; TypeID has_role; Value v; };
struct PG_ResetHas { TypeID has_role; };
struct PG_AdvanceIndex { TypeID index_role; };
struct PG_ReturnSum_Roles { TypeID data_role; TypeID size_role; TypeID index_role; TypeID sum_role; };
struct PG_ComputeRole { TypeID role; };
struct PG_IfReturnConst { Value then_v; Value else_v; };
struct PG_StoreRole { TypeID role; };
struct PG_ComputeLoadElem_Roles { TypeID base_role; TypeID index_role; };
struct PG_ComputeAdd_I32_Roles { TypeID lhs_role; TypeID rhs_role; };
struct PG_ComputeCmp_LT_Roles { TypeID lhs_role; TypeID rhs_role; };

struct PrimitiveGraph { PGKind kind{ PGKind::ReturnRole }; std::variant<PG_ReturnRole, PG_ReturnCmpLT_Roles, PG_ReturnLoadElem_Roles, PG_EmplaceConst, PG_ResetHas, PG_AdvanceIndex, PG_ReturnSum_Roles, PG_ComputeRole, PG_IfReturnConst, PG_StoreRole, PG_ComputeLoadElem_Roles, PG_ComputeAdd_I32_Roles, PG_ComputeCmp_LT_Roles> data{}; const char* cap{ nullptr }; };
struct PrimitiveGraphSeq { std::vector<PrimitiveGraph> items{}; };
struct FlowInstance { MorphismIR ir{}; MorphismIRMetadata meta{}; };

inline PrimitiveGraph lower_term_to_pg(const Foundation::IRSystem::term::Node& term) {
    using namespace Foundation::IRSystem::term; PrimitiveGraph pg{};
    if (term.kind == Kind::Perform) {
        const auto& perf = std::get<Perform>(term.data);
        if (perf.op == GetTypeID<Foundation::IRSystem::HasValueTag>()) { pg.kind = PGKind::ReturnRole; pg.data = PG_ReturnRole{ GetTypeID<get_has_value_index_tag>() }; pg.cap = "HasValue"; }
        else if (perf.op == GetTypeID<Foundation::IRSystem::ValueTag>()) { pg.kind = PGKind::ReturnRole; pg.data = PG_ReturnRole{ GetTypeID<get_value_index_tag>() }; pg.cap = "Value"; }
        else if (perf.op == GetTypeID<Foundation::IRSystem::UncheckedGetTag>()) { pg.kind = PGKind::ReturnRole; pg.data = PG_ReturnRole{ GetTypeID<get_value_index_tag>() }; pg.cap = "UncheckedGet"; }
        else if (perf.op == GetTypeID<Foundation::IRSystem::HasNextTag>()) { pg.kind = PGKind::ReturnCmpLT_Roles; pg.data = PG_ReturnCmpLT_Roles{ GetTypeID<get_index_index_tag>(), GetTypeID<get_size_index_tag>() }; pg.cap = "IterHasNext"; }
        else if (perf.op == GetTypeID<Foundation::IRSystem::GetCurrentTag>()) { pg.kind = PGKind::ReturnLoadElem_Roles; pg.data = PG_ReturnLoadElem_Roles{ GetTypeID<get_data_index_tag>(), GetTypeID<get_index_index_tag>() }; pg.cap = "IterGetCurrent"; }
        else if (perf.op == GetTypeID<Foundation::IRSystem::EmplaceTag>()) { Value v{}; if (!term.children.empty() && term.children[0].kind == Kind::Const) { v = std::get<Const>(term.children[0].data).value; } pg.kind = PGKind::EmplaceConst; pg.data = PG_EmplaceConst{ GetTypeID<get_value_index_tag>(), GetTypeID<get_has_value_index_tag>(), v }; pg.cap = "Emplace"; }
        else if (perf.op == GetTypeID<Foundation::IRSystem::ResetTag>()) { pg.kind = PGKind::ResetHas; pg.data = PG_ResetHas{ GetTypeID<get_has_value_index_tag>() }; pg.cap = "Reset"; }
        else if (perf.op == GetTypeID<Foundation::IRSystem::AdvanceTag>()) { pg.kind = PGKind::AdvanceIndex; pg.data = PG_AdvanceIndex{ GetTypeID<get_index_index_tag>() }; pg.cap = "IterAdvance"; }
        else if (perf.op == GetTypeID<Foundation::IRSystem::GetSumTag>()) { pg.kind = PGKind::ReturnSum_Roles; pg.data = PG_ReturnSum_Roles{ GetTypeID<get_data_index_tag>(), GetTypeID<get_size_index_tag>(), GetTypeID<get_index_index_tag>(), GetTypeID<get_sum_index_tag>() }; pg.cap = "IterForEachSum"; }
    } return pg;
}

inline PrimitiveGraphSeq lower_term_to_pg_seq(const Foundation::IRSystem::term::Node& term) {
    using namespace Foundation::IRSystem::term; PrimitiveGraphSeq seq{};
    if (term.kind == Kind::If && term.children.size() == 3) {
        const auto& cond = term.children[0]; const auto& tbr = term.children[1]; const auto& ebr = term.children[2];
        if (cond.kind == Kind::Perform) {
            const auto& perf = std::get<Perform>(cond.data);
            if (perf.op == GetTypeID<Foundation::IRSystem::HasValueTag>()) { PrimitiveGraph pgc{}; pgc.kind = PGKind::ComputeRole; pgc.data = PG_ComputeRole{ GetTypeID<get_has_value_index_tag>() }; pgc.cap = "ComputeHasValue"; seq.items.push_back(pgc); if (tbr.kind == Kind::Const && ebr.kind == Kind::Const) { PrimitiveGraph pgi{}; pgi.kind = PGKind::IfReturnConst; pgi.data = PG_IfReturnConst{ std::get<Const>(tbr.data).value, std::get<Const>(ebr.data).value }; pgi.cap = "IfReturnConst"; seq.items.push_back(pgi); return seq; } }
            if (perf.op == GetTypeID<Foundation::IRSystem::HasNextTag>()) { PrimitiveGraph pgc{}; pgc.kind = PGKind::ComputeCmp_LT_Roles; pgc.data = PG_ComputeCmp_LT_Roles{ GetTypeID<get_index_index_tag>(), GetTypeID<get_size_index_tag>() }; pgc.cap = "IterHasNextCond"; seq.items.push_back(pgc); if (tbr.kind == Kind::Const && ebr.kind == Kind::Const) { PrimitiveGraph pgi{}; pgi.kind = PGKind::IfReturnConst; pgi.data = PG_IfReturnConst{ std::get<Const>(tbr.data).value, std::get<Const>(ebr.data).value }; pgi.cap = "IfReturnConst"; seq.items.push_back(pgi); return seq; } }
        }
    }
    if (term.kind == Kind::Seq && term.children.size() == 2) { auto a = lower_term_to_pg_seq(term.children[0]); auto b = lower_term_to_pg_seq(term.children[1]); seq.items.insert(seq.items.end(), a.items.begin(), a.items.end()); seq.items.insert(seq.items.end(), b.items.begin(), b.items.end()); return seq; }
    if (term.kind == Kind::Let && term.children.size() == 2) { auto a = lower_term_to_pg_seq(term.children[0]); auto b = lower_term_to_pg_seq(term.children[1]); seq.items.insert(seq.items.end(), a.items.begin(), a.items.end()); seq.items.insert(seq.items.end(), b.items.begin(), b.items.end()); return seq; }
    seq.items.push_back(lower_term_to_pg(term)); return seq;
}

inline MorphismIR materialize_with_object(const PrimitiveGraph& pg, const ObjectDescriptor& desc, MorphismIRMetadata& meta) {
    FlowBuilder fb; fb.ir.expected_args = 1; auto obj = fb.arg(0); auto idx_of = [&](TypeID rid){ int i = desc.find_role_index(rid); return i < 0 ? -1 : static_cast<UInt32>(i); }; UInt64 eid = 0; meta.functor_name = "M: C_term→C_ir"; meta.two_cell_id = 0; meta.naturality_square_id = 0;
    switch (pg.kind) {
        case PGKind::ReturnRole: { const auto& d = std::get<PG_ReturnRole>(pg.data); auto idx = idx_of(d.role); if (idx == static_cast<UInt32>(-1)) { fb.ret_void(); meta.transform_chain.push_back("materialize"); return fb.build(); } auto val = fb.load_member(obj, idx); fb.ret(val); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx }, pg.cap }); break; }
        case PGKind::ReturnCmpLT_Roles: { const auto& d = std::get<PG_ReturnCmpLT_Roles>(pg.data); auto idx_r = idx_of(d.rhs_role); auto idx_l = idx_of(d.lhs_role); if (idx_r == static_cast<UInt32>(-1) || idx_l == static_cast<UInt32>(-1)) { fb.ret_void(); meta.transform_chain.push_back("materialize"); return fb.build(); } auto sz = fb.load_member(obj, idx_r); auto ix = fb.load_member(obj, idx_l); auto cond = fb.cmp_lt_i32(ix, sz); fb.ret(cond); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_l, idx_r }, pg.cap }); break; }
        case PGKind::ReturnLoadElem_Roles: { const auto& d = std::get<PG_ReturnLoadElem_Roles>(pg.data); auto idx_b = idx_of(d.base_role); auto idx_i = idx_of(d.index_role); if (idx_b == static_cast<UInt32>(-1) || idx_i == static_cast<UInt32>(-1)) { fb.ret_void(); meta.transform_chain.push_back("materialize"); return fb.build(); } auto base = fb.load_member(obj, idx_b); auto ix = fb.load_member(obj, idx_i); auto val = fb.load_elem_i32(base, ix); fb.ret(val); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_b, idx_i }, pg.cap }); break; }
        case PGKind::EmplaceConst: { const auto& d = std::get<PG_EmplaceConst>(pg.data); auto idx_v = idx_of(d.value_role); auto idx_h = idx_of(d.has_role); if (idx_v == static_cast<UInt32>(-1) || idx_h == static_cast<UInt32>(-1)) { fb.ret_void(); meta.transform_chain.push_back("materialize"); return fb.build(); } auto c = fb.add_const(d.v); fb.store_member(obj, idx_v, c); auto one = fb.add_const(true); fb.store_member(obj, idx_h, one); fb.ret_void(); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_v, idx_h }, pg.cap }); break; }
        case PGKind::ResetHas: { const auto& d = std::get<PG_ResetHas>(pg.data); auto idx_h = idx_of(d.has_role); if (idx_h == static_cast<UInt32>(-1)) { fb.ret_void(); meta.transform_chain.push_back("materialize"); return fb.build(); } auto zero = fb.add_const(false); fb.store_member(obj, idx_h, zero); fb.ret_void(); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_h }, pg.cap }); break; }
        case PGKind::AdvanceIndex: { const auto& d = std::get<PG_AdvanceIndex>(pg.data); auto idx_i = idx_of(d.index_role); if (idx_i == static_cast<UInt32>(-1)) { fb.ret_void(); meta.transform_chain.push_back("materialize"); return fb.build(); } auto ix = fb.load_member(obj, idx_i); auto one = fb.add_const(static_cast<int>(1)); auto nxt = fb.add_i32(ix, one); fb.store_member(obj, idx_i, nxt); fb.ret_void(); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_i }, pg.cap }); break; }
        case PGKind::ReturnSum_Roles: { const auto& d = std::get<PG_ReturnSum_Roles>(pg.data); auto idx_data = idx_of(d.data_role); auto idx_size = idx_of(d.size_role); auto idx_index = idx_of(d.index_role); auto idx_sum = idx_of(d.sum_role); if (idx_data == static_cast<UInt32>(-1) || idx_size == static_cast<UInt32>(-1) || idx_index == static_cast<UInt32>(-1) || idx_sum == static_cast<UInt32>(-1)) { fb.ret_void(); meta.transform_chain.push_back("materialize"); return fb.build(); } BlockID cond = fb.entry; BlockID body = fb.new_block(); BlockID exit = fb.new_block(); fb.switch_to(cond); auto sz_c = fb.load_member(obj, idx_size); auto ix_c = fb.load_member(obj, idx_index); auto cond_reg = fb.cmp_lt_i32(ix_c, sz_c); fb.br_if(cond_reg, body, exit); fb.switch_to(body); auto base_b = fb.load_member(obj, idx_data); auto ix_b = fb.load_member(obj, idx_index); auto cur_b = fb.load_elem_i32(base_b, ix_b); auto sum_b = fb.load_member(obj, idx_sum); auto s2_b = fb.add_i32(sum_b, cur_b); fb.store_member(obj, idx_sum, s2_b); auto one_b = fb.add_const(static_cast<int>(1)); auto nxt_b = fb.add_i32(ix_b, one_b); fb.store_member(obj, idx_index, nxt_b); fb.br(cond); fb.switch_to(exit); auto sfin = fb.load_member(obj, idx_sum); fb.ret(sfin); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_data, idx_size, idx_index, idx_sum }, pg.cap }); break; }
        default: fb.ret_void(); break;
    } meta.transform_chain.push_back("materialize"); return fb.build();
}

inline std::optional<RegisterID> emit_fragment_with_object(const PrimitiveGraph& pg, const ObjectDescriptor& desc, FlowBuilder& fb, MorphismIRMetadata& meta) {
    auto obj = fb.arg(0); auto idx_of = [&](TypeID rid){ int i = desc.find_role_index(rid); return i < 0 ? -1 : static_cast<UInt32>(i); }; UInt64 eid = 0;
    switch (pg.kind) {
        case PGKind::ReturnRole: { const auto& d = std::get<PG_ReturnRole>(pg.data); auto idx = idx_of(d.role); if (idx == static_cast<UInt32>(-1)) return std::nullopt; auto val = fb.load_member(obj, idx); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx }, pg.cap }); return val; }
        case PGKind::ReturnCmpLT_Roles: { const auto& d = std::get<PG_ReturnCmpLT_Roles>(pg.data); auto idx_r = idx_of(d.rhs_role); auto idx_l = idx_of(d.lhs_role); if (idx_r == static_cast<UInt32>(-1) || idx_l == static_cast<UInt32>(-1)) return std::nullopt; auto sz = fb.load_member(obj, idx_r); auto ix = fb.load_member(obj, idx_l); auto cond = fb.cmp_lt_i32(ix, sz); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_l, idx_r }, pg.cap }); return cond; }
        case PGKind::ReturnLoadElem_Roles: { const auto& d = std::get<PG_ReturnLoadElem_Roles>(pg.data); auto idx_b = idx_of(d.base_role); auto idx_i = idx_of(d.index_role); if (idx_b == static_cast<UInt32>(-1) || idx_i == static_cast<UInt32>(-1)) return std::nullopt; auto base = fb.load_member(obj, idx_b); auto ix = fb.load_member(obj, idx_i); auto val = fb.load_elem_i32(base, ix); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_b, idx_i }, pg.cap }); return val; }
        case PGKind::EmplaceConst: { const auto& d = std::get<PG_EmplaceConst>(pg.data); auto idx_v = idx_of(d.value_role); auto idx_h = idx_of(d.has_role); if (idx_v == static_cast<UInt32>(-1) || idx_h == static_cast<UInt32>(-1)) return std::nullopt; auto c = fb.add_const(d.v); fb.store_member(obj, idx_v, c); auto one = fb.add_const(true); fb.store_member(obj, idx_h, one); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_v, idx_h }, pg.cap }); return std::nullopt; }
        case PGKind::ResetHas: { const auto& d = std::get<PG_ResetHas>(pg.data); auto idx_h = idx_of(d.has_role); if (idx_h == static_cast<UInt32>(-1)) return std::nullopt; auto zero = fb.add_const(false); fb.store_member(obj, idx_h, zero); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_h }, pg.cap }); return std::nullopt; }
        case PGKind::AdvanceIndex: { const auto& d = std::get<PG_AdvanceIndex>(pg.data); auto idx_i = idx_of(d.index_role); if (idx_i == static_cast<UInt32>(-1)) return std::nullopt; auto ix = fb.load_member(obj, idx_i); auto one = fb.add_const(static_cast<int>(1)); auto nxt = fb.add_i32(ix, one); fb.store_member(obj, idx_i, nxt); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_i }, pg.cap }); return std::nullopt; }
        case PGKind::ReturnSum_Roles: { const auto& d = std::get<PG_ReturnSum_Roles>(pg.data); auto idx_data = idx_of(d.data_role); auto idx_size = idx_of(d.size_role); auto idx_index = idx_of(d.index_role); auto idx_sum = idx_of(d.sum_role); if (idx_data == static_cast<UInt32>(-1) || idx_size == static_cast<UInt32>(-1) || idx_index == static_cast<UInt32>(-1) || idx_sum == static_cast<UInt32>(-1)) return std::nullopt; BlockID cond = fb.entry; BlockID body = fb.new_block(); BlockID exit = fb.new_block(); fb.switch_to(cond); auto sz_c = fb.load_member(obj, idx_size); auto ix_c = fb.load_member(obj, idx_index); auto cond_reg = fb.cmp_lt_i32(ix_c, sz_c); fb.br_if(cond_reg, body, exit); fb.switch_to(body); auto base_b = fb.load_member(obj, idx_data); auto ix_b = fb.load_member(obj, idx_index); auto cur_b = fb.load_elem_i32(base_b, ix_b); auto sum_b = fb.load_member(obj, idx_sum); auto s2_b = fb.add_i32(sum_b, cur_b); fb.store_member(obj, idx_sum, s2_b); auto one_b = fb.add_const(static_cast<int>(1)); auto nxt_b = fb.add_i32(ix_b, one_b); fb.store_member(obj, idx_index, nxt_b); fb.br(cond); fb.switch_to(exit); auto sfin = fb.load_member(obj, idx_sum); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_of(d.data_role), idx_of(d.size_role), idx_of(d.index_role), idx_of(d.sum_role) }, pg.cap }); return sfin; }
        default: return std::nullopt;
    }
}

inline MorphismIR materialize_with_object_seq(const PrimitiveGraphSeq& pgs, const ObjectDescriptor& desc, MorphismIRMetadata& meta) {
    FlowBuilder fb; fb.ir.expected_args = 1; auto obj = fb.arg(0); auto idx_of = [&](TypeID rid){ int i = desc.find_role_index(rid); return i < 0 ? -1 : static_cast<UInt32>(i); }; RegisterID last_val = static_cast<RegisterID>(-1); bool did_return = false; UInt64 eid = 0; meta.functor_name = "M: C_term→C_ir"; meta.two_cell_id = 0; meta.naturality_square_id = 0;
    for (const auto& pg : pgs.items) {
        if (did_return) break; switch (pg.kind) {
            case PGKind::ComputeRole: { const auto& d = std::get<PG_ComputeRole>(pg.data); auto idx = idx_of(d.role); if (idx == static_cast<UInt32>(-1)) { meta.errors.push_back("missing role index for ComputeRole"); fb.ret_void(); did_return = true; break; } last_val = fb.load_member(obj, idx); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx }, pg.cap }); break; }
            case PGKind::StoreRole: { const auto& d = std::get<PG_StoreRole>(pg.data); if (last_val != static_cast<RegisterID>(-1)) { auto idx = idx_of(d.role); if (idx == static_cast<UInt32>(-1)) { meta.errors.push_back("missing role index for StoreRole"); fb.ret_void(); did_return = true; break; } fb.store_member(obj, idx, last_val); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx }, pg.cap }); } break; }
            case PGKind::ComputeLoadElem_Roles: { const auto& d = std::get<PG_ComputeLoadElem_Roles>(pg.data); auto idx_b = idx_of(d.base_role); auto idx_i = idx_of(d.index_role); if (idx_b == static_cast<UInt32>(-1) || idx_i == static_cast<UInt32>(-1)) { meta.errors.push_back("missing role index for ComputeLoadElem_Roles"); fb.ret_void(); did_return = true; break; } auto base = fb.load_member(obj, idx_b); auto ix = fb.load_member(obj, idx_i); last_val = fb.load_elem_i32(base, ix); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_b, idx_i }, pg.cap }); break; }
            case PGKind::ComputeAdd_I32_Roles: { const auto& d = std::get<PG_ComputeAdd_I32_Roles>(pg.data); auto idx_l = idx_of(d.lhs_role); auto idx_r = idx_of(d.rhs_role); if (idx_l == static_cast<UInt32>(-1) || idx_r == static_cast<UInt32>(-1)) break; auto l = fb.load_member(obj, idx_l); auto r = fb.load_member(obj, idx_r); last_val = fb.add_i32(l, r); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_l, idx_r }, pg.cap }); break; }
            case PGKind::ComputeCmp_LT_Roles: { const auto& d = std::get<PG_ComputeCmp_LT_Roles>(pg.data); auto idx_l = idx_of(d.lhs_role); auto idx_r = idx_of(d.rhs_role); if (idx_l == static_cast<UInt32>(-1) || idx_r == static_cast<UInt32>(-1)) break; auto l = fb.load_member(obj, idx_l); auto r = fb.load_member(obj, idx_r); last_val = fb.cmp_lt_i32(l, r); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_l, idx_r }, pg.cap }); break; }
            case PGKind::ReturnRole: { const auto& d = std::get<PG_ReturnRole>(pg.data); auto idx = idx_of(d.role); if (idx == static_cast<UInt32>(-1)) { fb.ret_void(); did_return = true; break; } auto val = fb.load_member(obj, idx); fb.ret(val); did_return = true; meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx }, pg.cap }); break; }
            case PGKind::ReturnCmpLT_Roles: { const auto& d = std::get<PG_ReturnCmpLT_Roles>(pg.data); auto idx_r = idx_of(d.rhs_role); auto idx_l = idx_of(d.lhs_role); if (idx_r == static_cast<UInt32>(-1) || idx_l == static_cast<UInt32>(-1)) { fb.ret_void(); did_return = true; break; } auto sz = fb.load_member(obj, idx_r); auto ix = fb.load_member(obj, idx_l); auto cond = fb.cmp_lt_i32(ix, sz); fb.ret(cond); did_return = true; meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_l, idx_r }, pg.cap }); break; }
            case PGKind::ReturnLoadElem_Roles: { const auto& d = std::get<PG_ReturnLoadElem_Roles>(pg.data); auto idx_b = idx_of(d.base_role); auto idx_i = idx_of(d.index_role); if (idx_b == static_cast<UInt32>(-1) || idx_i == static_cast<UInt32>(-1)) { fb.ret_void(); did_return = true; break; } auto base = fb.load_member(obj, idx_b); auto ix = fb.load_member(obj, idx_i); auto val = fb.load_elem_i32(base, ix); fb.ret(val); did_return = true; meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_b, idx_i }, pg.cap }); break; }
            case PGKind::EmplaceConst: { const auto& d = std::get<PG_EmplaceConst>(pg.data); auto idx_v = idx_of(d.value_role); auto idx_h = idx_of(d.has_role); if (idx_v == static_cast<UInt32>(-1) || idx_h == static_cast<UInt32>(-1)) break; auto c = fb.add_const(d.v); fb.store_member(obj, idx_v, c); auto one = fb.add_const(true); fb.store_member(obj, idx_h, one); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_v, idx_h }, pg.cap }); break; }
            case PGKind::ResetHas: { const auto& d = std::get<PG_ResetHas>(pg.data); auto idx_h = idx_of(d.has_role); if (idx_h == static_cast<UInt32>(-1)) break; auto zero = fb.add_const(false); fb.store_member(obj, idx_h, zero); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_h }, pg.cap }); break; }
            case PGKind::AdvanceIndex: { const auto& d = std::get<PG_AdvanceIndex>(pg.data); auto idx_i = idx_of(d.index_role); if (idx_i == static_cast<UInt32>(-1)) break; auto ix = fb.load_member(obj, idx_i); auto one = fb.add_const(static_cast<int>(1)); auto nxt = fb.add_i32(ix, one); fb.store_member(obj, idx_i, nxt); meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { idx_i }, pg.cap }); break; }
            case PGKind::IfReturnConst: { const auto& d = std::get<PG_IfReturnConst>(pg.data); BlockID t = fb.new_block(); BlockID f = fb.new_block(); BlockID j = fb.new_block(); if (last_val == static_cast<RegisterID>(-1)) { fb.ret_void(); did_return = true; break; } fb.br_if(last_val, t, f); fb.switch_to(t); auto tv = fb.add_const(d.then_v); fb.ret(tv); fb.switch_to(f); auto fv = fb.add_const(d.else_v); fb.ret(fv); fb.switch_to(j); did_return = true; meta.hyperedges.push_back(MorphismIRMetadata::HyperEdgeSource{ eid++, { }, pg.cap }); break; }
            default: break;
        }
    } if (!did_return) fb.ret_void(); meta.transform_chain.push_back("materialize_seq"); return fb.build();
}

inline FlowInstance build_flow_from_term_with_object(const Foundation::IRSystem::term::Node& term, const ObjectDescriptor& fd) { MorphismIRMetadata meta{}; auto pg = lower_term_to_pg(term); auto ir = materialize_with_object(pg, fd, meta); return FlowInstance{ std::move(ir), std::move(meta) }; }
inline FlowInstance build_flow_from_term_with_object_multi(const Foundation::IRSystem::term::Node& term, const ObjectDescriptor& fd) { MorphismIRMetadata meta{}; auto pgs = lower_term_to_pg_seq(term); auto ir = materialize_with_object_seq(pgs, fd, meta); return FlowInstance{ std::move(ir), std::move(meta) }; }

}