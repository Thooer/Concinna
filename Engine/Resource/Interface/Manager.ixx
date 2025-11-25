module;
export module Engine.Resource:Manager;

import Language;
import Foundation.Memory;
import Foundation.Containers;
import :Handle;
import Foundation.IO;
import Foundation.Serialization;
import :Cache;
import Engine.Runtime;

export namespace Engine::Resource {
    struct MeshRecord { MeshHandle h; Language::UInt32 ref{0}; };
    struct MaterialRecord { MaterialHandle h; Language::UInt32 ref{0}; };
    struct TextureRecord { TextureHandle h; Language::UInt32 ref{0}; };

    class ResourceManager {
    public:
        explicit ResourceManager(::Foundation::Memory::IAllocator* a) noexcept
            : m_alloc(a),
              m_meshRec(nullptr), m_matRec(nullptr), m_texRec(nullptr), m_meshCount(0), m_matCount(0), m_texCount(0),
              m_meshCap(0), m_matCap(0), m_texCap(0),
              m_meshCache(a, static_cast<Language::USize>(1024)),
              m_matCache(a, static_cast<Language::USize>(1024)),
              m_texCache(a, static_cast<Language::USize>(1024)) {}

        MeshHandle LoadMesh(Language::StringView path) noexcept {
            for (Language::USize i=0;i<m_meshKeyCount;++i) {
                if (m_meshKeySizes[static_cast<size_t>(i)]==path.size()) {
                    const Language::Char8* p = m_meshKeyPtrs[static_cast<size_t>(i)];
                    const Language::Char8* s = path.data();
                    Language::USize n=path.size();
                    Language::USize j=0; for (;j<n;++j) { if (p[static_cast<size_t>(j)]!=s[static_cast<size_t>(j)]) break; }
                    if (j==n) return MeshHandle{ static_cast<Language::UInt32>(i) };
                }
            }
            if (m_meshCount == m_meshCap) { growMesh(); }
            Language::UInt32 id = static_cast<Language::UInt32>(m_meshCount);
            m_meshRec[static_cast<size_t>(m_meshCount)] = MeshRecord{ MeshHandle{id}, 1 };
            m_meshCount++;
            Language::USize len = path.size();
            auto ar = m_alloc->Allocate((len+1) * static_cast<Language::USize>(sizeof(Language::Char8)), static_cast<Language::USize>(alignof(Language::Char8)));
            if (ar.IsOk()) {
                auto* p = reinterpret_cast<Language::Char8*>(ar.OkValue());
                const Language::Char8* src = path.data();
                for (Language::USize i=0;i<len;++i) p[static_cast<size_t>(i)] = src[static_cast<size_t>(i)];
                p[static_cast<size_t>(len)] = static_cast<Language::Char8>(0);
                if (m_meshKeyCount == m_meshKeyCap) growMeshKeys();
                m_meshKeyPtrs[static_cast<size_t>(m_meshKeyCount)] = p;
                m_meshKeySizes[static_cast<size_t>(m_meshKeyCount)] = len;
                m_meshKeyCount++;
                (void)len;
            }
            (void)m_meshCache.Add(path, id);
            return MeshHandle{id};
        }

        MaterialHandle LoadMaterial(Language::StringView path) noexcept {
            for (Language::USize i=0;i<m_matKeyCount;++i) {
                if (m_matKeySizes[static_cast<size_t>(i)]==path.size()) {
                    const Language::Char8* p = m_matKeyPtrs[static_cast<size_t>(i)];
                    const Language::Char8* s = path.data();
                    Language::USize n=path.size();
                    Language::USize j=0; for (;j<n;++j) { if (p[static_cast<size_t>(j)]!=s[static_cast<size_t>(j)]) break; }
                    if (j==n) return MaterialHandle{ static_cast<Language::UInt32>(i) };
                }
            }
            if (m_matCount == m_matCap) { growMat(); }
            Language::UInt32 id = static_cast<Language::UInt32>(m_matCount);
            m_matRec[static_cast<size_t>(m_matCount)] = MaterialRecord{ MaterialHandle{id}, 1 };
            m_matCount++;
            Language::USize len = path.size();
            auto ar = m_alloc->Allocate((len+1) * static_cast<Language::USize>(sizeof(Language::Char8)), static_cast<Language::USize>(alignof(Language::Char8)));
            if (ar.IsOk()) {
                auto* p = reinterpret_cast<Language::Char8*>(ar.OkValue());
                const Language::Char8* src = path.data();
                for (Language::USize i=0;i<len;++i) p[static_cast<size_t>(i)] = src[static_cast<size_t>(i)];
                p[static_cast<size_t>(len)] = static_cast<Language::Char8>(0);
                if (m_matKeyCount == m_matKeyCap) growMatKeys();
                m_matKeyPtrs[static_cast<size_t>(m_matKeyCount)] = p;
                m_matKeySizes[static_cast<size_t>(m_matKeyCount)] = len;
                m_matKeyCount++;
                (void)len;
            }
            (void)m_matCache.Add(path, id);
            return MaterialHandle{id};
        }

        TextureHandle LoadTexture(Language::StringView path) noexcept {
            for (Language::USize i=0;i<m_texKeyCount;++i) {
                if (m_texKeySizes[static_cast<size_t>(i)]==path.size()) {
                    const Language::Char8* p = m_texKeyPtrs[static_cast<size_t>(i)];
                    const Language::Char8* s = path.data();
                    Language::USize n=path.size();
                    Language::USize j=0; for (;j<n;++j) { if (p[static_cast<size_t>(j)]!=s[static_cast<size_t>(j)]) break; }
                    if (j==n) return TextureHandle{ static_cast<Language::UInt32>(i) };
                }
            }
            if (m_texCount == m_texCap) { growTex(); }
            Language::UInt32 id = static_cast<Language::UInt32>(m_texCount);
            m_texRec[static_cast<size_t>(m_texCount)] = TextureRecord{ TextureHandle{id}, 1 };
            m_texCount++;
            Language::USize len = path.size();
            auto ar = m_alloc->Allocate((len+1) * static_cast<Language::USize>(sizeof(Language::Char8)), static_cast<Language::USize>(alignof(Language::Char8)));
            if (ar.IsOk()) {
                auto* p = reinterpret_cast<Language::Char8*>(ar.OkValue());
                const Language::Char8* src = path.data();
                for (Language::USize i=0;i<len;++i) p[static_cast<size_t>(i)] = src[static_cast<size_t>(i)];
                p[static_cast<size_t>(len)] = static_cast<Language::Char8>(0);
                if (m_texKeyCount == m_texKeyCap) growTexKeys();
                m_texKeyPtrs[static_cast<size_t>(m_texKeyCount)] = p;
                m_texKeySizes[static_cast<size_t>(m_texKeyCount)] = len;
                m_texKeyCount++;
                (void)len;
            }
            (void)m_texCache.Add(path, id);
            return TextureHandle{id};
        }

        MeshHandle GetMesh(Language::UInt32 id) const noexcept { return MeshHandle{ id }; }
        MaterialHandle GetMaterial(Language::UInt32 id) const noexcept { return MaterialHandle{ id }; }
        TextureHandle GetTexture(Language::UInt32 id) const noexcept { return TextureHandle{ id }; }

        void AddRef(MeshHandle h) noexcept { if (h.id < m_meshCount) m_meshRec[static_cast<size_t>(h.id)].ref++; }
        void AddRef(MaterialHandle h) noexcept { if (h.id < m_matCount) m_matRec[static_cast<size_t>(h.id)].ref++; }
        void AddRef(TextureHandle h) noexcept { if (h.id < m_texCount) m_texRec[static_cast<size_t>(h.id)].ref++; }

        void Release(MeshHandle h) noexcept { if (h.id < m_meshCount) { auto& r = m_meshRec[static_cast<size_t>(h.id)]; if (r.ref) r.ref--; } }
        void Release(MaterialHandle h) noexcept { if (h.id < m_matCount) { auto& r = m_matRec[static_cast<size_t>(h.id)]; if (r.ref) r.ref--; } }
        void Release(TextureHandle h) noexcept { if (h.id < m_texCount) { auto& r = m_texRec[static_cast<size_t>(h.id)]; if (r.ref) r.ref--; } }

        [[nodiscard]] Language::Result<MeshHandle> LoadMeshAsync(Language::StringView path) noexcept {
            auto h = LoadMesh(path);
            auto id = h.id;
            auto fn = [this, path, id]() noexcept { m_meshCache.Touch(path); };
            (void)Engine::EnqueuePriorityT(fn, 1);
            return Language::Result<MeshHandle>::Ok(h);
        }
        [[nodiscard]] Language::Result<MaterialHandle> LoadMaterialAsync(Language::StringView path) noexcept {
            auto h = LoadMaterial(path);
            auto id = h.id;
            auto fn2 = [this, path, id]() noexcept { m_matCache.Touch(path); };
            (void)Engine::EnqueuePriorityT(fn2, 1);
            return Language::Result<MaterialHandle>::Ok(h);
        }
        [[nodiscard]] Language::Result<TextureHandle> LoadTextureAsync(Language::StringView path) noexcept {
            auto h = LoadTexture(path);
            auto id = h.id;
            auto fn3 = [this, path, id]() noexcept { m_texCache.Touch(path); };
            (void)Engine::EnqueuePriorityT(fn3, 1);
            return Language::Result<TextureHandle>::Ok(h);
        }

        [[nodiscard]] Language::Status Prefetch(Language::Span<const Language::StringView> keys) noexcept {
            for (Language::USize i=0;i<keys.size();++i) { (void)m_meshCache.Add(keys[static_cast<size_t>(i)], static_cast<Handle>(0)); }
            return Language::Ok(Language::StatusDomain::Resource());
        }
        [[nodiscard]] Language::Result<Language::UInt32> LoadBatch(Language::Span<const Language::StringView> keys) noexcept {
            Language::UInt32 okc = 0;
            for (Language::USize i=0;i<keys.size();++i) {
                auto k = keys[static_cast<size_t>(i)];
                auto r = LoadMeshAsync(k);
                if (r.IsOk()) okc++;
            }
            return Language::Result<Language::UInt32>::Ok(okc);
        }

    private:
        ::Foundation::Memory::IAllocator* m_alloc{};
        const Language::Char8** m_meshKeyPtrs{nullptr}; Language::USize m_meshKeyCount{0}; Language::USize m_meshKeyCap{0};
        Language::USize* m_meshKeySizes{nullptr};
        const Language::Char8** m_matKeyPtrs{nullptr}; Language::USize m_matKeyCount{0}; Language::USize m_matKeyCap{0};
        Language::USize* m_matKeySizes{nullptr};
        const Language::Char8** m_texKeyPtrs{nullptr}; Language::USize m_texKeyCount{0}; Language::USize m_texKeyCap{0};
        Language::USize* m_texKeySizes{nullptr};
        Language::UInt32 pad0{0};
        MeshRecord* m_meshRec; Language::USize m_meshCount; Language::USize m_meshCap;
        MaterialRecord* m_matRec; Language::USize m_matCount; Language::USize m_matCap;
        TextureRecord* m_texRec; Language::USize m_texCount; Language::USize m_texCap;
        LRUCache m_meshCache;
        LRUCache m_matCache;
        LRUCache m_texCache;

        void growMesh() noexcept {
            Language::USize newCap = (m_meshCap==0)?8:(m_meshCap*2);
            auto r = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(MeshRecord)), static_cast<Language::USize>(alignof(MeshRecord)));
            if (!r.IsOk()) return; auto* np = reinterpret_cast<MeshRecord*>(r.OkValue());
            for (Language::USize i=0;i<m_meshCount;++i) np[static_cast<size_t>(i)] = m_meshRec ? m_meshRec[static_cast<size_t>(i)] : MeshRecord{MeshHandle{0},0};
            if (m_meshRec) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_meshRec), m_meshCap * static_cast<Language::USize>(sizeof(MeshRecord)), static_cast<Language::USize>(alignof(MeshRecord)));
            m_meshRec=np; m_meshCap=newCap;
        }
        void growMat() noexcept {
            Language::USize newCap = (m_matCap==0)?8:(m_matCap*2);
            auto r = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(MaterialRecord)), static_cast<Language::USize>(alignof(MaterialRecord)));
            if (!r.IsOk()) return; auto* np = reinterpret_cast<MaterialRecord*>(r.OkValue());
            for (Language::USize i=0;i<m_matCount;++i) np[static_cast<size_t>(i)] = m_matRec ? m_matRec[static_cast<size_t>(i)] : MaterialRecord{MaterialHandle{0},0};
            if (m_matRec) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_matRec), m_matCap * static_cast<Language::USize>(sizeof(MaterialRecord)), static_cast<Language::USize>(alignof(MaterialRecord)));
            m_matRec=np; m_matCap=newCap;
        }
        void growTex() noexcept {
            Language::USize newCap = (m_texCap==0)?8:(m_texCap*2);
            auto r = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(TextureRecord)), static_cast<Language::USize>(alignof(TextureRecord)));
            if (!r.IsOk()) return; auto* np = reinterpret_cast<TextureRecord*>(r.OkValue());
            for (Language::USize i=0;i<m_texCount;++i) np[static_cast<size_t>(i)] = m_texRec ? m_texRec[static_cast<size_t>(i)] : TextureRecord{TextureHandle{0},0};
            if (m_texRec) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_texRec), m_texCap * static_cast<Language::USize>(sizeof(TextureRecord)), static_cast<Language::USize>(alignof(TextureRecord)));
            m_texRec=np; m_texCap=newCap;
        }
        void growMeshKeys() noexcept {
            Language::USize newCap = (m_meshKeyCap==0)?8:(m_meshKeyCap*2);
            auto r1 = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(const Language::Char8*)), static_cast<Language::USize>(alignof(const Language::Char8*)));
            if (!r1.IsOk()) return; auto* np1 = reinterpret_cast<const Language::Char8**>(r1.OkValue());
            for (Language::USize i=0;i<m_meshKeyCount;++i) np1[static_cast<size_t>(i)] = m_meshKeyPtrs ? m_meshKeyPtrs[static_cast<size_t>(i)] : nullptr;
            if (m_meshKeyPtrs) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(const_cast<Language::Char8**>(m_meshKeyPtrs)), m_meshKeyCap * static_cast<Language::USize>(sizeof(const Language::Char8*)), static_cast<Language::USize>(alignof(const Language::Char8*)));
            m_meshKeyPtrs = np1;
            auto r2 = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(Language::USize)), static_cast<Language::USize>(alignof(Language::USize)));
            if (!r2.IsOk()) return; auto* np2 = reinterpret_cast<Language::USize*>(r2.OkValue());
            for (Language::USize i=0;i<m_meshKeyCount;++i) np2[static_cast<size_t>(i)] = m_meshKeySizes ? m_meshKeySizes[static_cast<size_t>(i)] : 0;
            if (m_meshKeySizes) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_meshKeySizes), m_meshKeyCap * static_cast<Language::USize>(sizeof(Language::USize)), static_cast<Language::USize>(alignof(Language::USize)));
            m_meshKeySizes = np2; m_meshKeyCap=newCap;
        }
        void growMatKeys() noexcept {
            Language::USize newCap = (m_matKeyCap==0)?8:(m_matKeyCap*2);
            auto r1 = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(const Language::Char8*)), static_cast<Language::USize>(alignof(const Language::Char8*)));
            if (!r1.IsOk()) return; auto* np1 = reinterpret_cast<const Language::Char8**>(r1.OkValue());
            for (Language::USize i=0;i<m_matKeyCount;++i) np1[static_cast<size_t>(i)] = m_matKeyPtrs ? m_matKeyPtrs[static_cast<size_t>(i)] : nullptr;
            if (m_matKeyPtrs) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(const_cast<Language::Char8**>(m_matKeyPtrs)), m_matKeyCap * static_cast<Language::USize>(sizeof(const Language::Char8*)), static_cast<Language::USize>(alignof(const Language::Char8*)));
            m_matKeyPtrs = np1;
            auto r2 = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(Language::USize)), static_cast<Language::USize>(alignof(Language::USize)));
            if (!r2.IsOk()) return; auto* np2 = reinterpret_cast<Language::USize*>(r2.OkValue());
            for (Language::USize i=0;i<m_matKeyCount;++i) np2[static_cast<size_t>(i)] = m_matKeySizes ? m_matKeySizes[static_cast<size_t>(i)] : 0;
            if (m_matKeySizes) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_matKeySizes), m_matKeyCap * static_cast<Language::USize>(sizeof(Language::USize)), static_cast<Language::USize>(alignof(Language::USize)));
            m_matKeySizes = np2; m_matKeyCap=newCap;
        }
        void growTexKeys() noexcept {
            Language::USize newCap = (m_texKeyCap==0)?8:(m_texKeyCap*2);
            auto r1 = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(const Language::Char8*)), static_cast<Language::USize>(alignof(const Language::Char8*)));
            if (!r1.IsOk()) return; auto* np1 = reinterpret_cast<const Language::Char8**>(r1.OkValue());
            for (Language::USize i=0;i<m_texKeyCount;++i) np1[static_cast<size_t>(i)] = m_texKeyPtrs ? m_texKeyPtrs[static_cast<size_t>(i)] : nullptr;
            if (m_texKeyPtrs) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(const_cast<Language::Char8**>(m_texKeyPtrs)), m_texKeyCap * static_cast<Language::USize>(sizeof(const Language::Char8*)), static_cast<Language::USize>(alignof(const Language::Char8*)));
            m_texKeyPtrs = np1;
            auto r2 = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(Language::USize)), static_cast<Language::USize>(alignof(Language::USize)));
            if (!r2.IsOk()) return; auto* np2 = reinterpret_cast<Language::USize*>(r2.OkValue());
            for (Language::USize i=0;i<m_texKeyCount;++i) np2[static_cast<size_t>(i)] = m_texKeySizes ? m_texKeySizes[static_cast<size_t>(i)] : 0;
            if (m_texKeySizes) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_texKeySizes), m_texKeyCap * static_cast<Language::USize>(sizeof(Language::USize)), static_cast<Language::USize>(alignof(Language::USize)));
            m_texKeySizes = np2; m_texKeyCap=newCap;
        }
    };

    [[nodiscard]] inline Language::Result<MeshHandle> LoadMesh(ResourceManager& mgr, Language::StringView path) noexcept {
        auto h = mgr.LoadMesh(path);
        return Language::Result<MeshHandle>::Ok(h);
    }
    [[nodiscard]] inline Language::Result<MaterialHandle> LoadMaterial(ResourceManager& mgr, Language::StringView path) noexcept {
        auto h = mgr.LoadMaterial(path);
        return Language::Result<MaterialHandle>::Ok(h);
    }
    [[nodiscard]] inline Language::Result<TextureHandle> LoadTexture(ResourceManager& mgr, Language::StringView path) noexcept {
        auto h = mgr.LoadTexture(path);
        return Language::Result<TextureHandle>::Ok(h);
    }

    [[nodiscard]] inline Language::Status Unload(ResourceManager& mgr, MeshHandle h) noexcept {
        mgr.Release(h);
        return Language::Ok(Language::StatusDomain::Memory());
    }
    [[nodiscard]] inline Language::Status Unload(ResourceManager& mgr, MaterialHandle h) noexcept {
        mgr.Release(h);
        return Language::Ok(Language::StatusDomain::Memory());
    }
    [[nodiscard]] inline Language::Status Unload(ResourceManager& mgr, TextureHandle h) noexcept {
        mgr.Release(h);
        return Language::Ok(Language::StatusDomain::Memory());
    }
}