export module Foundation.IRSystem:IR.Core.ObjectRegistry;
import <unordered_map>;
import <mutex>;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.Core.TypeMeta;
import Foundation.IRSystem:IR.Core.ObjectDescriptor;

export namespace Foundation::IRSystem {

struct ObjectRegistry {
    static void register_object(const ObjectDescriptor& od) {
        std::lock_guard<std::mutex> lock(m());
        map()[od.formType.value] = od;
    }
    static const ObjectDescriptor* find(TypeID t) {
        std::lock_guard<std::mutex> lock(m());
        auto it = map().find(t.value);
        if (it == map().end()) return nullptr;
        return &it->second;
    }
    static bool exists(TypeID t) {
        std::lock_guard<std::mutex> lock(m());
        return map().find(t.value) != map().end();
    }
    static void remove(TypeID t) {
        std::lock_guard<std::mutex> lock(m());
        map().erase(t.value);
    }
private:
    static std::unordered_map<UInt64, ObjectDescriptor>& map() {
        static std::unordered_map<UInt64, ObjectDescriptor> r;
        return r;
    }
    static std::mutex& m() {
        static std::mutex mx;
        return mx;
    }
};

}