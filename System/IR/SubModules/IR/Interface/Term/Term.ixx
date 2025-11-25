export module Foundation.IRSystem:IR.Term.Term;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.Core.TypeMeta;
import <string>;
import <vector>;
import <variant>;

export namespace Foundation::IRSystem::term {

enum class Kind { Var, Const, Abs, App, Let, If, Seq, Perform, Handle };

struct Var { Foundation::IRSystem::Int32 index{}; };
struct Const { std::variant<bool,int,double,void*> value{}; };
struct Abs { Foundation::IRSystem::Int32 param{}; };
struct App { };
struct Let { Foundation::IRSystem::Int32 bind{}; };
struct If { };
struct Seq { };
struct Perform { Foundation::IRSystem::TypeID op; };
struct Handle { std::string op; };

struct Node {
  Kind kind{Kind::Const};
  std::variant<Var, Const, Abs, App, Let, If, Seq, Perform, Handle> data{};
  std::vector<Node> children{};
};

inline Node make_perform_id(Foundation::IRSystem::TypeID op, std::vector<Node> args = {}) {
  Node n; n.kind = Kind::Perform; n.data = Perform{ op }; n.children = std::move(args); return n;
}
template<typename TCapabilityTag>
inline Node make_perform_tag(std::vector<Node> args = {}) {
return make_perform_id(Foundation::IRSystem::GetTypeID<TCapabilityTag>(), std::move(args));
}

}