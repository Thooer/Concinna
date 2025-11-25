#pragma once
import <string>;
import <vector>;
import <optional>;

namespace Foundation::IRSystem {

enum class TermKind {
    Var,
    Const,
    Abs,
    App,
    Let,
    If,
    Seq,
    Perform,
    Handle
};

struct Term {
    TermKind kind;
    std::string symbol;
    int32_t index = -1;
    std::vector<Term> children;
};

}