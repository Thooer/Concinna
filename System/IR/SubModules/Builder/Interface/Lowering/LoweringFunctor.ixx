#pragma once
import Foundation.IRSystem;

namespace Foundation::IRSystem {

class LoweringFunctor {
public:
    MorphismIR run(const Term& term);
};

}