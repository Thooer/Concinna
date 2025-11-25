export module Foundation.IRSystem:Builder.Compiler.LoweringPipeline;
import Foundation.IRSystem:IR.Term.Term;
import Foundation.IRSystem:Builder.LawsTerm.RewriteEngine;
import <functional>;
import <algorithm>;
import <array>;
import <unordered_set>;
import <unordered_map>;

export namespace Foundation::IRSystem::compiler {

struct LoweringPipeline {
  using Node = Foundation::IRSystem::term::Node;
  using Kind = Foundation::IRSystem::term::Kind;
  using Var  = Foundation::IRSystem::term::Var;
  using Abs  = Foundation::IRSystem::term::Abs;
  using App  = Foundation::IRSystem::term::App;
  using Let  = Foundation::IRSystem::term::Let;
  using If   = Foundation::IRSystem::term::If;
  using Strategy = Foundation::IRSystem::term::Strategy;
  using Rule = Foundation::IRSystem::term::Rule;
  static Foundation::IRSystem::term::Node canonicalize(const Foundation::IRSystem::term::Node& t) {
    using namespace Foundation::IRSystem::term;
    std::array<Rule, 8> rules{
      make_rule_if_const_bool(),
      make_rule_beta_reduce(),
      make_rule_eta_reduce(),
      make_rule_let_dead_binding(),
      make_rule_seq_drop_const_left(),
      make_rule_let_inline_trivial_binding(),
      make_rule_seq_flatten_left(),
      make_rule_seq_flatten_right()
    };
    auto inn = apply_laws(t, rules, Strategy::Innermost);
    return apply_laws(inn, rules, Strategy::Outermost);
  }

  static std::pair<Foundation::IRSystem::term::Node, std::vector<const char*>> canonicalize_with_trace(const Foundation::IRSystem::term::Node& t) {
    using namespace Foundation::IRSystem::term;
    std::array<Rule, 8> rules{
      make_rule_if_const_bool(),
      make_rule_beta_reduce(),
      make_rule_eta_reduce(),
      make_rule_let_dead_binding(),
      make_rule_seq_drop_const_left(),
      make_rule_let_inline_trivial_binding(),
      make_rule_seq_flatten_left(),
      make_rule_seq_flatten_right()
    };
    auto inn = apply_laws_with_trace(t, rules, Strategy::Innermost);
    auto out = apply_laws_with_trace(inn.first, rules, Strategy::Outermost);
    std::vector<const char*> names{};
    names.insert(names.end(), inn.second.names.begin(), inn.second.names.end());
    names.insert(names.end(), out.second.names.begin(), out.second.names.end());
    return { out.first, std::move(names) };
  }

  static Foundation::IRSystem::term::Node anf(const Foundation::IRSystem::term::Node& t) {
    using namespace Foundation::IRSystem::term;
    auto is_trivial = [&](const Node& n) {
      return n.kind == Kind::Var || n.kind == Kind::Const || n.kind == Kind::Abs;
    };
    auto max_idx = [&](const Node& n) {
      Foundation::IRSystem::Int32 m = -1;
      std::function<void(const Node&)> walk = [&](const Node& x){
        switch (x.kind) {
          case Kind::Var: m = std::max(m, std::get<Var>(x.data).index); break;
          case Kind::Abs: m = std::max(m, std::get<Abs>(x.data).param); break;
          case Kind::Let: m = std::max(m, std::get<Let>(x.data).bind); break;
          default: break;
        }
        for (const auto& ch : x.children) walk(ch);
      };
      walk(n);
      return m;
    };
    struct State { Foundation::IRSystem::Int32 next{}; } st{ max_idx(t) + 1 };
    std::function<Node(const Node&)> lift = [&](const Node& n) -> Node {
      if (n.kind == Kind::App && n.children.size() == 2) {
        Node f = lift(n.children[0]);
        Node a = lift(n.children[1]);
        std::vector<std::pair<Foundation::IRSystem::Int32, Node>> binds;
        Node f_use = f;
        if (!is_trivial(f)) { Foundation::IRSystem::Int32 v = st.next++; binds.emplace_back(v, f); Node vnode; vnode.kind = Kind::Var; vnode.data = Var{ v }; f_use = vnode; }
        Node a_use = a;
        if (!is_trivial(a)) { Foundation::IRSystem::Int32 v = st.next++; binds.emplace_back(v, a); Node vnode; vnode.kind = Kind::Var; vnode.data = Var{ v }; a_use = vnode; }
        Node app; app.kind = Kind::App; app.data = App{}; app.children = { f_use, a_use };
        Node body = app;
        for (int i = static_cast<int>(binds.size()) - 1; i >= 0; --i) { Node let; let.kind = Kind::Let; let.data = Let{ binds[i].first }; let.children = { binds[i].second, body }; body = let; }
        return body;
      } else if (n.kind == Kind::Seq && n.children.size() == 2) {
        Node a = lift(n.children[0]);
        Node b = lift(n.children[1]);
        if (!is_trivial(a)) { Foundation::IRSystem::Int32 v = st.next++; Node vnode; vnode.kind = Kind::Var; vnode.data = Var{ v }; Node let; let.kind = Kind::Let; let.data = Let{ v }; let.children = { a, b }; return let; }
        Node out = n; out.children = { a, b }; return out;
      } else if (n.kind == Kind::If && n.children.size() == 3) {
        Node c = lift(n.children[0]); Node tbr = lift(n.children[1]); Node ebr = lift(n.children[2]);
        if (!is_trivial(c)) { Foundation::IRSystem::Int32 v = st.next++; Node vnode; vnode.kind = Kind::Var; vnode.data = Var{ v }; Node ifn; ifn.kind = Kind::If; ifn.data = If{}; ifn.children = { vnode, tbr, ebr }; Node let; let.kind = Kind::Let; let.data = Let{ v }; let.children = { c, ifn }; return let; }
        Node out = n; out.children = { c, tbr, ebr }; return out;
      } else {
        Node out = n; for (auto& ch : out.children) ch = lift(ch); return out;
      }
    };
    return lift(t);
  }

  static Foundation::IRSystem::term::Node closure_conv(const Foundation::IRSystem::term::Node& t) {
    using namespace Foundation::IRSystem::term;
    auto max_idx = [&](const Node& n) {
      Foundation::IRSystem::Int32 m = -1;
      std::function<void(const Node&)> walk = [&](const Node& x){
        switch (x.kind) {
          case Kind::Var: m = std::max(m, std::get<Var>(x.data).index); break;
          case Kind::Abs: m = std::max(m, std::get<Abs>(x.data).param); break;
          case Kind::Let: m = std::max(m, std::get<Let>(x.data).bind); break;
          default: break;
        }
        for (const auto& ch : x.children) walk(ch);
      };
      walk(n);
      return m;
    };
    auto collect_free_vars = [&](const Node& n, std::unordered_set<Foundation::IRSystem::Int32>& out, std::unordered_set<Foundation::IRSystem::Int32> bound) {
      std::function<void(const Node&, std::unordered_set<Foundation::IRSystem::Int32>&, std::unordered_set<Foundation::IRSystem::Int32>)> walk = [&](const Node& x, std::unordered_set<Foundation::IRSystem::Int32>& acc, std::unordered_set<Foundation::IRSystem::Int32> bnd){
        switch (x.kind) {
          case Kind::Var: { Foundation::IRSystem::Int32 vi = std::get<Var>(x.data).index; if (bnd.find(vi) == bnd.end()) acc.insert(vi); break; }
          case Kind::Abs: { Foundation::IRSystem::Int32 p = std::get<Abs>(x.data).param; if (!x.children.empty()) { auto bnd2 = bnd; bnd2.insert(p); walk(x.children[0], acc, std::move(bnd2)); } break; }
          case Kind::Let: { Foundation::IRSystem::Int32 b = std::get<Let>(x.data).bind; if (x.children.size() == 2) { walk(x.children[0], acc, bnd); auto bnd2 = bnd; bnd2.insert(b); walk(x.children[1], acc, std::move(bnd2)); } break; }
          default: { for (const auto& ch : x.children) walk(ch, acc, bnd); break; }
        }
      };
      walk(n, out, std::move(bound));
    };
    Foundation::IRSystem::Int32 next = max_idx(t) + 1;
    std::function<Node(const Node&, std::unordered_map<Foundation::IRSystem::Int32, Foundation::IRSystem::Int32>&)> conv = [&](const Node& n, std::unordered_map<Foundation::IRSystem::Int32, Foundation::IRSystem::Int32>& env) -> Node {
      if (n.kind == Kind::Var) { const auto& v = std::get<Var>(n.data); auto it = env.find(v.index); if (it != env.end()) { Node out = n; out.data = Var{ it->second }; return out; } return n; }
      if (n.kind == Kind::Abs) {
        const auto& a = std::get<Abs>(n.data); Foundation::IRSystem::Int32 fresh = next++;
        std::unordered_map<Foundation::IRSystem::Int32, Foundation::IRSystem::Int32> env2 = env; env2[a.param] = fresh;
        Node body = n.children.empty() ? Node{} : n.children[0];
        std::unordered_set<Foundation::IRSystem::Int32> fvset{}; std::unordered_set<Foundation::IRSystem::Int32> bound{}; bound.insert(a.param); collect_free_vars(body, fvset, bound);
        Node wrapped = body;
        for (Foundation::IRSystem::Int32 vi : fvset) { Foundation::IRSystem::Int32 cap = next++; env2[vi] = cap; Node bind_expr; bind_expr.kind = Kind::Var; bind_expr.data = Var{ vi }; Node let; let.kind = Kind::Let; let.data = Let{ cap }; let.children = { bind_expr, wrapped }; wrapped = let; }
        Node out = n; out.data = Abs{ fresh }; if (!out.children.empty()) out.children[0] = conv(wrapped, env2); return out;
      }
      if (n.kind == Kind::Let && n.children.size() == 2) {
        const auto& l = std::get<Let>(n.data); Foundation::IRSystem::Int32 fresh = next++;
        Node bind_expr = conv(n.children[0], env);
        std::unordered_map<Foundation::IRSystem::Int32, Foundation::IRSystem::Int32> env2 = env; env2[l.bind] = fresh;
        Node body_expr = conv(n.children[1], env2);
        Node out = n; out.data = Let{ fresh }; out.children = { bind_expr, body_expr }; return out;
      }
      Node out = n; for (auto& ch : out.children) ch = conv(ch, env); return out;
    };
    std::unordered_map<Foundation::IRSystem::Int32, Foundation::IRSystem::Int32> env{};
    return conv(t, env);
  }

  static Foundation::IRSystem::term::Node effects(const Foundation::IRSystem::term::Node& t) {
    using namespace Foundation::IRSystem::term;
    std::function<Node(const Node&)> elim = [&](const Node& n) -> Node {
      if (n.kind == Kind::Handle && n.children.size() >= 1) return elim(n.children[0]);
      Node out = n; for (auto& ch : out.children) ch = elim(ch); return out;
    }; return elim(t);
  }

  static Foundation::IRSystem::term::Node run(const Foundation::IRSystem::term::Node& t) {
    auto a = canonicalize(t); auto b = anf(a); auto c = closure_conv(b); auto d = effects(c); return d;
  }

  struct TraceResult { Foundation::IRSystem::term::Node node; std::vector<const char*> law_names; };
  static TraceResult run_with_trace(const Foundation::IRSystem::term::Node& t) {
    auto a = canonicalize_with_trace(t); auto b = anf(a.first); auto c = closure_conv(b); auto d = effects(c); return TraceResult{ d, std::move(a.second) };
  }
};

}