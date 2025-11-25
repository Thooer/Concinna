#pragma once
import <string>;
import <vector>;
import Foundation.IRSystem;

namespace Foundation::IRSystem {

struct CatObject { std::string name; };
struct CatMorphism { std::string name; };
struct CatFunctor { std::string name; };
struct CatNaturalTransformation { std::string name; };

struct CatAST {
    std::vector<CatObject> objects;
    std::vector<CatMorphism> morphisms;
};

namespace CatLowering {
    Term to_term(const CatAST& ast);
}

}