module;
export module Engine.Scene:View;

import Language;
import :Types;
import :World;
import :Component;

export namespace Engine::Scene {
    struct ViewRange {
        struct Iterator {
            SceneWorld const* w{};
            Language::UInt32 mask{};
            Language::USize idx{};
            Iterator(SceneWorld const* ww, Language::UInt32 m, Language::USize i) noexcept : w(ww), mask(m), idx(i) {}
            bool operator!=(const Iterator& other) const noexcept { return idx != other.idx; }
            void operator++() noexcept { for (;;) { idx += static_cast<Language::USize>(1); if (idx >= w->Count()) break; if (mask == 0) break; bool ok = true; if (mask & Component_Transform) { ok = w->HasTransform(static_cast<Entity>(idx)); } if (ok) break; } }
            Entity operator*() const noexcept { return static_cast<Entity>(idx); }
        };
        SceneWorld const* w{};
        Language::UInt32 mask{};
        Language::USize beginIndex{};
        Language::USize endIndex{};
        Iterator begin() const noexcept { return Iterator{ w, mask, beginIndex }; }
        Iterator end() const noexcept { return Iterator{ w, mask, endIndex }; }
    };

    export ViewRange GetView(SceneWorld const& w, Language::UInt32 componentMask) noexcept { ViewRange r{ &w, componentMask, static_cast<Language::USize>(0), w.Count() }; if (componentMask) { while (r.beginIndex < r.endIndex) { bool ok = true; if (componentMask & Component_Transform) { ok = w.HasTransform(static_cast<Entity>(r.beginIndex)); } if (ok) break; r.beginIndex += static_cast<Language::USize>(1); } } return r; }
}