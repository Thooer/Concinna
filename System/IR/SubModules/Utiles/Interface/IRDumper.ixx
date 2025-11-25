export module Foundation.IRSystem:Utiles.IRDumper;
import Foundation.IRSystem:IR.IR.MorphismIR;
import <string>;
import <sstream>;

export namespace Foundation::IRSystem {
export inline std::string to_text(const MorphismIR& ir) {
    std::ostringstream os;
    os << "entry " << ir.entry_block << " blocks " << ir.blocks.size() << "\n";
    for (const auto& b : ir.blocks) {
        os << "block " << (&b - &ir.blocks[0]) << "\n";
        for (const auto& n : b.nodes) os << static_cast<int>(n.opcode) << " " << n.result_reg << "\n";
        os << static_cast<int>(b.terminator.opcode) << " -1\n";
    }
    return os.str();
}
}