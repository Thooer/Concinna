export module Foundation.IRSystem:IR.IR.IRDumper;
import <string>;
import <sstream>;
import Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:IR.IR.MorphismIRMetadata;

export namespace Foundation::IRSystem {

inline std::string to_text(const MorphismIR& ir) {
    std::ostringstream os;
    os << "entry " << ir.entry_block << " blocks " << ir.blocks.size() << "\n";
    for (const auto& b : ir.blocks) {
        os << "block " << (&b - &ir.blocks[0]) << "\n";
        for (const auto& n : b.nodes) {
            os << static_cast<int>(n.opcode) << " " << n.result_reg << "\n";
        }
        os << static_cast<int>(b.terminator.opcode) << " -1\n";
    }
    return os.str();
}

inline std::string meta_to_text(const MorphismIRMetadata& meta) {
    std::ostringstream os;
    os << "functor:" << meta.functor_name << " two_cell_id:" << meta.two_cell_id << " naturality_square_id:" << meta.naturality_square_id << "\n";
    os << "transforms:";
    for (auto s : meta.transform_chain) os << " " << s;
    os << "\npasses:";
    for (auto s : meta.pass_chain) os << " " << s;
    os << "\nhyperedges:";
    for (const auto& he : meta.hyperedges) os << " " << he.capability;
    return os.str();
}

inline std::string to_json(const MorphismIR& ir) {
    std::ostringstream os;
    os << "{";
    os << "\"entry\":" << ir.entry_block << ",";
    os << "\"blocks\":[";
    for (size_t i = 0; i < ir.blocks.size(); ++i) {
        const auto& b = ir.blocks[i];
        os << "{";
        os << "\"nodes\":[";
        for (size_t j = 0; j < b.nodes.size(); ++j) {
            const auto& n = b.nodes[j];
            os << "{\"op\":" << static_cast<int>(n.opcode) << ",\"res\":" << n.result_reg << "}";
            if (j + 1 < b.nodes.size()) os << ",";
        }
        os << "],";
        os << "\"term\":" << static_cast<int>(b.terminator.opcode);
        os << "}";
        if (i + 1 < ir.blocks.size()) os << ",";
    }
    os << "]}";
    return os.str();
}

inline std::string meta_to_json(const MorphismIRMetadata& meta) {
    std::ostringstream os;
    os << "{";
    os << "\"functor\":\"" << meta.functor_name << "\",";
    os << "\"two_cell_id\":" << meta.two_cell_id << ",";
    os << "\"naturality_square_id\":" << meta.naturality_square_id << ",";
    os << "\"transforms\":[";
    for (size_t i = 0; i < meta.transform_chain.size(); ++i) { os << "\"" << meta.transform_chain[i] << "\""; if (i + 1 < meta.transform_chain.size()) os << ","; }
    os << "],\"passes\":[";
    for (size_t i = 0; i < meta.pass_chain.size(); ++i) { os << "\"" << meta.pass_chain[i] << "\""; if (i + 1 < meta.pass_chain.size()) os << ","; }
    os << "],\"hyperedges\":[";
    for (size_t i = 0; i < meta.hyperedges.size(); ++i) { os << "\"" << meta.hyperedges[i].capability << "\""; if (i + 1 < meta.hyperedges.size()) os << ","; }
    os << "]}";
    return os.str();
}

}