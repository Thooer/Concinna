export module Foundation.IRSystem:Builder.LawsTerm.RewriteEngine;
import <span>;
import <optional>;
import <functional>;
import <utility>;
import Foundation.IRSystem:IR.Term.Term;

export namespace Foundation::IRSystem::term {

enum class Strategy { Innermost, Outermost };

struct Rule {
  const char* name{};
  std::function<std::optional<Node>(const Node&)> try_apply;
};

inline bool is_const_bool(const Node& n, bool* out = nullptr) {
  if (n.kind != Kind::Const) return false;
  const auto& c = std::get<Const>(n.data);
  if (std::holds_alternative<bool>(c.value)) { if (out) *out = std::get<bool>(c.value); return true; }
  return false;
}

inline Node substitute(const Node& body, Foundation::IRSystem::Int32 param, const Node& arg) {
  switch (body.kind) {
    case Kind::Var: {
      const auto& v = std::get<Var>(body.data);
      if (v.index == param) return arg; else return body;
    }
    case Kind::Const:
    case Kind::Perform:
      return body;
    case Kind::Abs: {
      const auto& a = std::get<Abs>(body.data);
      if (a.param == param) return body;
      Node out = body;
      if (!out.children.empty()) out.children[0] = substitute(out.children[0], param, arg);
      return out;
    }
    default: {
      Node out = body;
      for (auto& ch : out.children) ch = substitute(ch, param, arg);
      return out;
    }
  }
}

inline bool contains_var(const Node& n, Foundation::IRSystem::Int32 idx) {
  if (n.kind == Kind::Var) return std::get<Var>(n.data).index == idx;
  for (const auto& ch : n.children) if (contains_var(ch, idx)) return true;
  return false;
}

inline Node apply_laws(const Node& t, std::span<const Rule> laws, Strategy s = Strategy::Innermost) {
  using Pair = std::pair<Node, bool>;
  auto rewrite_once = [&](const Node& n) -> Pair {
    for (const auto& rule : laws) {
      if (!rule.try_apply) continue;
      if (auto r = rule.try_apply(n)) return { *r, true };
    }
    return { n, false };
  };

  struct Recursor { std::function<Pair(const Node&)> f; } R;
  R.f = [&](const Node& n) -> Pair {
    bool changed = false;
    if (s == Strategy::Innermost) {
      Node rebuilt = n;
      for (auto& ch : rebuilt.children) { auto p = R.f(ch); ch = std::move(p.first); changed = changed || p.second; }
      auto r = rewrite_once(rebuilt);
      return { r.first, changed || r.second };
    } else {
      auto r0 = rewrite_once(n);
      bool ch0 = r0.second;
      Node rebuilt = r0.first;
      for (auto& ch : rebuilt.children) { auto p = R.f(ch); ch = std::move(p.first); changed = changed || p.second; }
      return { rebuilt, ch0 || changed };
    }
  };

  Node cur = t;
  while (true) {
    auto p = R.f(cur);
    if (!p.second) return p.first;
    cur = std::move(p.first);
  }
}

struct LawTrace { std::vector<const char*> names; };

inline std::pair<Node, LawTrace> apply_laws_with_trace(const Node& t, std::span<const Rule> laws, Strategy s = Strategy::Innermost) {
  using Pair = std::pair<Node, bool>;
  LawTrace trace{};
  auto rewrite_once = [&](const Node& n) -> std::tuple<Node, bool, const char*> {
    for (const auto& rule : laws) {
      if (!rule.try_apply) continue;
      if (auto r = rule.try_apply(n)) return { *r, true, rule.name };
    }
    return { n, false, nullptr };
  };

  struct Recursor { std::function<std::pair<Node, bool>(const Node&)> f; } R;
  R.f = [&](const Node& n) -> std::pair<Node, bool> {
    bool changed = false;
    if (s == Strategy::Innermost) {
      Node rebuilt = n;
      for (auto& ch : rebuilt.children) { auto p = R.f(ch); ch = std::move(p.first); changed = changed || p.second; }
      auto r = rewrite_once(rebuilt);
      if (std::get<bool>(r)) { if (std::get<const char*>(r) != nullptr) trace.names.push_back(std::get<const char*>(r)); }
      return { std::get<Node>(r), changed || std::get<bool>(r) };
    } else {
      auto r0 = rewrite_once(n);
      bool ch0 = std::get<bool>(r0);
      if (ch0) { if (std::get<const char*>(r0) != nullptr) trace.names.push_back(std::get<const char*>(r0)); }
      Node rebuilt = std::get<Node>(r0);
      for (auto& ch : rebuilt.children) { auto p = R.f(ch); ch = std::move(p.first); changed = changed || p.second; }
      return { rebuilt, ch0 || changed };
    }
  };

  Node cur = t;
  while (true) {
    auto p = R.f(cur);
    if (!p.second) return { p.first, std::move(trace) };
    cur = std::move(p.first);
  }
}

inline Rule make_rule_if_const_bool() {
  Rule r{ "if-const-bool",
    [](const Node& n) -> std::optional<Node> {
      if (n.kind == Kind::If && n.children.size() == 3) {
        bool b{};
        if (is_const_bool(n.children[0], &b)) return b ? std::optional<Node>(n.children[1]) : std::optional<Node>(n.children[2]);
      }
      return std::nullopt;
    }
  }; return r;
}

inline Rule make_rule_beta_reduce() {
  Rule r{ "beta-reduce",
    [](const Node& n) -> std::optional<Node> {
      if (n.kind == Kind::App && n.children.size() == 2) {
        const auto& f = n.children[0];
        const auto& a = n.children[1];
        if (f.kind == Kind::Abs && !f.children.empty()) {
          const auto& ab = std::get<Abs>(f.data);
          return std::optional<Node>(substitute(f.children[0], ab.param, a));
        }
      }
      return std::nullopt;
    }
  }; return r;
}

inline Rule make_rule_eta_reduce() {
  Rule r{ "eta-reduce",
    [](const Node& n) -> std::optional<Node> {
      if (n.kind == Kind::Abs && !n.children.empty()) {
        const auto& ab = std::get<Abs>(n.data);
        const auto& b = n.children[0];
        if (b.kind == Kind::App && b.children.size() == 2 && b.children[1].kind == Kind::Var) {
          const auto& v = std::get<Var>(b.children[1].data);
          if (v.index == ab.param) {
            if (!contains_var(b.children[0], ab.param)) return std::optional<Node>(b.children[0]);
          }
        }
      }
      return std::nullopt;
    }
  }; return r;
}

inline Rule make_rule_let_dead_binding() {
  Rule r{ "let-dead-binding",
    [](const Node& n) -> std::optional<Node> {
      if (n.kind == Kind::Let && n.children.size() == 2) {
        const auto& let = std::get<Let>(n.data);
        const auto& body = n.children[1];
        if (!contains_var(body, let.bind)) return std::optional<Node>(body);
      }
      return std::nullopt;
    }
  }; return r;
}

inline Rule make_rule_seq_drop_const_left() {
  Rule r{ "seq-drop-const-left",
    [](const Node& n) -> std::optional<Node> {
      if (n.kind == Kind::Seq && n.children.size() == 2) {
        if (n.children[0].kind == Kind::Const) return std::optional<Node>(n.children[1]);
      }
      return std::nullopt;
    }
  }; return r;
}

inline Rule make_rule_let_inline_trivial_binding() {
  Rule r{ "let-inline-trivial",
    [](const Node& n) -> std::optional<Node> {
      if (n.kind == Kind::Let && n.children.size() == 2) {
        const auto& bind_expr = n.children[0];
        const auto& let = std::get<Let>(n.data);
        if (bind_expr.kind == Kind::Const || bind_expr.kind == Kind::Var) {
          Node body = n.children[1];
          Node out = substitute(body, let.bind, bind_expr);
          return std::optional<Node>(out);
        }
      }
      return std::nullopt;
    }
  }; return r;
}

inline Rule make_rule_seq_flatten_left() {
  Rule r{ "seq-flatten-left",
    [](const Node& n) -> std::optional<Node> {
      if (n.kind == Kind::Seq && n.children.size() == 2) {
        const auto& a = n.children[0]; const auto& b = n.children[1];
        if (a.kind == Kind::Seq && a.children.size() == 2) {
          Node out; out.kind = Kind::Seq; out.data = Seq{};
          Node right; right.kind = Kind::Seq; right.data = Seq{}; right.children = { a.children[1], b };
          out.children = { a.children[0], right };
          return std::optional<Node>(out);
        }
      }
      return std::nullopt;
    }
  }; return r;
}

inline Rule make_rule_seq_flatten_right() {
  Rule r{ "seq-flatten-right",
    [](const Node& n) -> std::optional<Node> {
      if (n.kind == Kind::Seq && n.children.size() == 2) {
        const auto& a = n.children[0]; const auto& b = n.children[1];
        if (b.kind == Kind::Seq && b.children.size() == 2) {
          Node out; out.kind = Kind::Seq; out.data = Seq{};
          Node left; left.kind = Kind::Seq; left.data = Seq{}; left.children = { a, b.children[0] };
          out.children = { left, b.children[1] };
          return std::optional<Node>(out);
        }
      }
      return std::nullopt;
    }
  }; return r;
}

}