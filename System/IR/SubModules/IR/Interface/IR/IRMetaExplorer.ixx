export module Foundation.IRSystem:IR.IR.IRMetaExplorer;
import <string>;
import <vector>;
import <unordered_set>;
import Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:IR.IR.MorphismIRMetadata;

export namespace Foundation::IRSystem::meta_explorer {

struct Summary { unsigned blocks; unsigned nodes; std::vector<std::string> capabilities; std::vector<std::string> transforms; std::vector<std::string> passes; };

inline Summary summarize(const MorphismIR& ir, const MorphismIRMetadata& meta) {
    Summary s{};
    s.blocks = static_cast<unsigned>(ir.blocks.size());
    unsigned nn = 0;
    for (const auto& b : ir.blocks) nn += static_cast<unsigned>(b.nodes.size());
    s.nodes = nn;
    std::unordered_set<std::string> caps;
    for (const auto& he : meta.hyperedges) if (he.capability) caps.insert(he.capability);
    for (const auto& c : caps) s.capabilities.push_back(c);
    for (auto t : meta.transform_chain) s.transforms.emplace_back(t);
    for (auto p : meta.pass_chain) s.passes.emplace_back(p);
    return s;
}

}