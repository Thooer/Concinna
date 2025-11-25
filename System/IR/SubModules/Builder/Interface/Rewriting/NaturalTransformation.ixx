#pragma once
import <string>;
import <vector>;
import Foundation.IRSystem;

namespace Foundation::IRSystem {

struct MorphismRule { std::string name; };

class RewriteEngine {
public:
    Term apply_morphisms(const Term& term, const std::vector<MorphismRule>& rules, const std::string& strategy);
};

}