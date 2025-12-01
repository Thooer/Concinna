module;
export module Engine.Resource:Manager;

import Lang;
import Cap.Memory;
import Cap.Containers;
import :Handle;
import Foundation.IO;
import Foundation.Serialization;
import :Cache;
import Engine.Runtime;

export namespace Engine::Resource {
    struct MeshRecord { MeshHandle h; UInt32 ref{0}; };
    struct MaterialRecord { MaterialHandle h; UInt32 ref{0}; };
    struct TextureRecord { TextureHandle h; UInt32 ref{0}; };

    class ResourceManager {
    public:
        explicit ResourceManager(Cap::IAllocator* a) noexcept
            : m_alloc(a),
              m_meshRec(nullptr), m_matRec(nullptr), m_texRec(nullptr), m_meshCount(0), m_matCount(0), m_texCount(0),
              m_meshCap(0), m_matCap(0), m_texCap(0),
              m_meshCache(a, static_cast<USize>(1024)),
              m_matCache(a, static_cast<USize>(1024)),
              m_texCache(a, static_cast<USize>(1024)) {}

        MeshHandle LoadMesh(StringView path) noexcept {
            for (USize i=0;i<m_meshKeyCount;++i) {
                if (m_meshKeySizes[static_cast<size_t>(i)]==path.size()) {
                    const Char8* p = m_meshKeyPtrs[static_cast<size_t>(i)];
                    const Char8* s = path.data();
                    USize n=path.size();
                    USize j=0; for (;j<n;++j) { if (p[static_cast<size_t>(j)]!=s[static_cast<size_t>(j)]) break; }
                    if (j==n) return MeshHandle{ static_cast<UInt32>(i) };
                }
            }
            if (m_meshCount == m_meshCap) { growMesh(); }
            UInt32 id = static_cast<UInt32>(m_meshCount);
            m_meshRec[static_cast<size_t>(m_meshCount)] = MeshRecord{ MeshHandle{id}, 1 };
            m_meshCount++;
            USize len = path.size();
            auto ar = m_alloc->Allocate((len+1) * static_cast<USize>(sizeof(Char8)), static_cast<USize>(alignof(Char8)));
            if (ar.IsOk()) {
                auto* p = reinterpret_cast<Char8*>(ar.OkValue());
                const Char8* src = path.data();
                for (USize i=0;i<len;++i) p[static_cast<size_t>(i)] = src[static_cast<size_t>(i)];
                p[static_cast<size_t>(len)] = static_cast<Char8>(0);
                if (m_meshKeyCount == m_meshKeyCap) growMeshKeys();
                m_meshKeyPtrs[static_cast<size_t>(m_meshKeyCount)] = p;
                m_meshKeySizes[static_cast<size_t>(m_meshKeyCount)] = len;
                m_meshKeyCount++;
                (void)len;
            }
            (void)m_meshCache.Add(path, id);
            return MeshHandle{id};
        }

        MaterialHandle LoadMaterial(StringView path) noexcept {
            for (USize i=0;i<m_matKeyCount;++i) {
                if (m_matKeySizes[static_cast<size_t>(i)]==path.size()) {
                    const Char8* p = m_matKeyPtrs[static_cast<size_t>(i)];
                    const Char8* s = path.data();
                    USize n=path.size();
                    USize j=0; for (;j<n;++j) { if (p[static_cast<size_t>(j)]!=s[static_cast<size_t>(j)]) break; }
                    if (j==n) return MaterialHandle{ static_cast<UInt32>(i) };
                }
            }
            if (m_matCount == m_matCap) { growMat(); }
            UInt32 id = static_cast<UInt32>(m_matCount);
            m_matRec[static_cast<size_t>(m_matCount)] = MaterialRecord{ MaterialHandle{id}, 1 };
            m_matCount++;
            USize len = path.size();
            auto ar = m_alloc->Allocate((len+1) * static_cast<USize>(sizeof(Char8)), static_cast<USize>(alignof(Char8)));
            if (ar.IsOk()) {
                auto* p = reinterpret_cast<Char8*>(ar.OkValue());
                const Char8* src = path.data();
                for (USize i=0;i<len;++i) p[static_cast<size_t>(i)] = src[static_cast<size_t>(i)];
                p[static_cast<size_t>(len)] = static_cast<Char8>(0);
                if (m_matKeyCount == m_matKeyCap) growMatKeys();
                m_matKeyPtrs[static_cast<size_t>(m_matKeyCount)] = p;
                m_matKeySizes[static_cast<size_t>(m_matKeyCount)] = len;
                m_matKeyCount++;
                (void)len;
            }
            (void)m_matCache.Add(path, id);
            return MaterialHandle{id};
        }

        TextureHandle LoadTexture(StringView path) noexcept {
            for (USize i=0;i<m_texKeyCount;++i) {
                if (m_texKeySizes[static_cast<size_t>(i)]==path.size()) {
                    const Char8* p = m_texKeyPtrs[static_cast<size_t>(i)];
                    const Char8* s = path.data();
                    USize n=path.size();
                    USize j=0; for (;j<n;++j) { if (p[static_cast<size_t>(j)]!=s[static_cast<size_t>(j)]) break; }
                    if (j==n) return TextureHandle{ static_cast<UInt32>(i) };
                }
            }
            if (m_texCount == m_texCap) { growTex(); }
            UInt32 id = static_cast<UInt32>(m_texCount);
            m_texRec[static_cast<size_t>(m_texCount)] = TextureRecord{ TextureHandle{id}, 1 };
            m_texCount++;
            USize len = path.size();
            auto ar = m_alloc->Allocate((len+1) * static_cast<USize>(sizeof(Char8)), static_cast<USize>(alignof(Char8)));
            if (ar.IsOk()) {
                auto* p = reinterpret_cast<Char8*>(ar.OkValue());
                const Char8* src = path.data();
                for (USize i=0;i<len;++i) p[static_cast<size_t>(i)] = src[static_cast<size_t>(i)];
                p[static_cast<size_t>(len)] = static_cast<Char8>(0);
                if (m_texKeyCount == m_texKeyCap) growTexKeys();
                m_texKeyPtrs[static_cast<size_t>(m_texKeyCount)] = p;
                m_texKeySizes[static_cast<size_t>(m_texKeyCount)] = len;
                m_texKeyCount++;
                (void)len;
            }
            (void)m_texCache.Add(path, id);
            return TextureHandle{id};
        }

        MeshHandle GetMesh(UInt32 id) const noexcept { return MeshHandle{ id }; }
        MaterialHandle GetMaterial(UInt32 id) const noexcept { return MaterialHandle{ id }; }
        TextureHandle GetTexture(UInt32 id) const noexcept { return TextureHandle{ id }; }

        void AddRef(MeshHandle h) noexcept { if (h.id < m_meshCount) m_meshRec[static_cast<size_t>(h.id)].ref++; }
        void AddRef(MaterialHandle h) noexcept { if (h.id < m_matCount) m_matRec[static_cast<size_t>(h.id)].ref++; }
        void AddRef(TextureHandle h) noexcept { if (h.id < m_texCount) m_texRec[static_cast<size_t>(h.id)].ref++; }

        void Release(MeshHandle h) noexcept { if (h.id < m_meshCount) { auto& r = m_meshRec[static_cast<size_t>(h.id)]; if (r.ref) r.ref--; } }
        void Release(MaterialHandle h) noexcept { if (h.id < m_matCount) { auto& r = m_matRec[static_cast<size_t>(h.id)]; if (r.ref) r.ref--; } }
        void Release(TextureHandle h) noexcept { if (h.id < m_texCount) { auto& r = m_texRec[static_cast<size_t>(h.id)]; if (r.ref) r.ref--; } }

        [[nodiscard]] Result<MeshHandle> LoadMeshAsync(StringView path) noexcept {
            auto h = LoadMesh(path);
            auto id = h.id;
            auto fn = [this, path, id]() noexcept { m_meshCache.Touch(path); };
            (void)Engine::EnqueuePriorityT(fn, 1);
            return Result<MeshHandle>::Ok(h);
        }
        [[nodiscard]] Result<MaterialHandle> LoadMaterialAsync(StringView path) noexcept {
            auto h = LoadMaterial(path);
            auto id = h.id;
            auto fn2 = [this, path, id]() noexcept { m_matCache.Touch(path); };
            (void)Engine::EnqueuePriorityT(fn2, 1);
            return Result<MaterialHandle>::Ok(h);
        }
        [[nodiscard]] Result<TextureHandle> LoadTextureAsync(StringView path) noexcept {
            auto h = LoadTexture(path);
            auto id = h.id;
            auto fn3 = [this, path, id]() noexcept { m_texCache.Touch(path); };
            (void)Engine::EnqueuePriorityT(fn3, 1);
            return Result<TextureHandle>::Ok(h);
        }

        [[nodiscard]] Status Prefetch(Span<const StringView> keys) noexcept {
            for (USize i=0;i<keys.size();++i) { (void)m_meshCache.Add(keys[static_cast<size_t>(i)], static_cast<Handle>(0)); }
            return Ok(StatusDomain::Resource());
        }
        [[nodiscard]] Result<UInt32> LoadBatch(Span<const StringView> keys) noexcept {
            UInt32 okc = 0;
            for (USize i=0;i<keys.size();++i) {
                auto k = keys[static_cast<size_t>(i)];
                auto r = LoadMeshAsync(k);
                if (r.IsOk()) okc++;
            }
            return Result<UInt32>::Ok(okc);
        }

    private:
        Cap::IAllocator* m_alloc{};
        const Char8** m_meshKeyPtrs{nullptr}; USize m_meshKeyCount{0}; USize m_meshKeyCap{0};
        USize* m_meshKeySizes{nullptr};
        const Char8** m_matKeyPtrs{nullptr}; USize m_matKeyCount{0}; USize m_matKeyCap{0};
        USize* m_matKeySizes{nullptr};
        const Char8** m_texKeyPtrs{nullptr}; USize m_texKeyCount{0}; USize m_texKeyCap{0};
        USize* m_texKeySizes{nullptr};
        UInt32 pad0{0};
        MeshRecord* m_meshRec; USize m_meshCount; USize m_meshCap;
        MaterialRecord* m_matRec; USize m_matCount; USize m_matCap;
        TextureRecord* m_texRec; USize m_texCount; USize m_texCap;
        LRUCache m_meshCache;
        LRUCache m_matCache;
        LRUCache m_texCache;

        void growMesh() noexcept {
            USize newCap = (m_meshCap==0)?8:(m_meshCap*2);
            auto r = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(MeshRecord)), static_cast<USize>(alignof(MeshRecord)));
            if (!r.IsOk()) return; auto* np = reinterpret_cast<MeshRecord*>(r.OkValue());
            for (USize i=0;i<m_meshCount;++i) np[static_cast<size_t>(i)] = m_meshRec ? m_meshRec[static_cast<size_t>(i)] : MeshRecord{MeshHandle{0},0};
            if (m_meshRec) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(m_meshRec), m_meshCap * static_cast<USize>(sizeof(MeshRecord)), static_cast<USize>(alignof(MeshRecord)));
            m_meshRec=np; m_meshCap=newCap;
        }
        void growMat() noexcept {
            USize newCap = (m_matCap==0)?8:(m_matCap*2);
            auto r = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(MaterialRecord)), static_cast<USize>(alignof(MaterialRecord)));
            if (!r.IsOk()) return; auto* np = reinterpret_cast<MaterialRecord*>(r.OkValue());
            for (USize i=0;i<m_matCount;++i) np[static_cast<size_t>(i)] = m_matRec ? m_matRec[static_cast<size_t>(i)] : MaterialRecord{MaterialHandle{0},0};
            if (m_matRec) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(m_matRec), m_matCap * static_cast<USize>(sizeof(MaterialRecord)), static_cast<USize>(alignof(MaterialRecord)));
            m_matRec=np; m_matCap=newCap;
        }
        void growTex() noexcept {
            USize newCap = (m_texCap==0)?8:(m_texCap*2);
            auto r = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(TextureRecord)), static_cast<USize>(alignof(TextureRecord)));
            if (!r.IsOk()) return; auto* np = reinterpret_cast<TextureRecord*>(r.OkValue());
            for (USize i=0;i<m_texCount;++i) np[static_cast<size_t>(i)] = m_texRec ? m_texRec[static_cast<size_t>(i)] : TextureRecord{TextureHandle{0},0};
            if (m_texRec) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(m_texRec), m_texCap * static_cast<USize>(sizeof(TextureRecord)), static_cast<USize>(alignof(TextureRecord)));
            m_texRec=np; m_texCap=newCap;
        }
        void growMeshKeys() noexcept {
            USize newCap = (m_meshKeyCap==0)?8:(m_meshKeyCap*2);
            auto r1 = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(const Char8*)), static_cast<USize>(alignof(const Char8*)));
            if (!r1.IsOk()) return; auto* np1 = reinterpret_cast<const Char8**>(r1.OkValue());
            for (USize i=0;i<m_meshKeyCount;++i) np1[static_cast<size_t>(i)] = m_meshKeyPtrs ? m_meshKeyPtrs[static_cast<size_t>(i)] : nullptr;
            if (m_meshKeyPtrs) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(const_cast<Char8**>(m_meshKeyPtrs)), m_meshKeyCap * static_cast<USize>(sizeof(const Char8*)), static_cast<USize>(alignof(const Char8*)));
            m_meshKeyPtrs = np1;
            auto r2 = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(USize)), static_cast<USize>(alignof(USize)));
            if (!r2.IsOk()) return; auto* np2 = reinterpret_cast<USize*>(r2.OkValue());
            for (USize i=0;i<m_meshKeyCount;++i) np2[static_cast<size_t>(i)] = m_meshKeySizes ? m_meshKeySizes[static_cast<size_t>(i)] : 0;
            if (m_meshKeySizes) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(m_meshKeySizes), m_meshKeyCap * static_cast<USize>(sizeof(USize)), static_cast<USize>(alignof(USize)));
            m_meshKeySizes = np2; m_meshKeyCap=newCap;
        }
        void growMatKeys() noexcept {
            USize newCap = (m_matKeyCap==0)?8:(m_matKeyCap*2);
            auto r1 = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(const Char8*)), static_cast<USize>(alignof(const Char8*)));
            if (!r1.IsOk()) return; auto* np1 = reinterpret_cast<const Char8**>(r1.OkValue());
            for (USize i=0;i<m_matKeyCount;++i) np1[static_cast<size_t>(i)] = m_matKeyPtrs ? m_matKeyPtrs[static_cast<size_t>(i)] : nullptr;
            if (m_matKeyPtrs) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(const_cast<Char8**>(m_matKeyPtrs)), m_matKeyCap * static_cast<USize>(sizeof(const Char8*)), static_cast<USize>(alignof(const Char8*)));
            m_matKeyPtrs = np1;
            auto r2 = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(USize)), static_cast<USize>(alignof(USize)));
            if (!r2.IsOk()) return; auto* np2 = reinterpret_cast<USize*>(r2.OkValue());
            for (USize i=0;i<m_matKeyCount;++i) np2[static_cast<size_t>(i)] = m_matKeySizes ? m_matKeySizes[static_cast<size_t>(i)] : 0;
            if (m_matKeySizes) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(m_matKeySizes), m_matKeyCap * static_cast<USize>(sizeof(USize)), static_cast<USize>(alignof(USize)));
            m_matKeySizes = np2; m_matKeyCap=newCap;
        }
        void growTexKeys() noexcept {
            USize newCap = (m_texKeyCap==0)?8:(m_texKeyCap*2);
            auto r1 = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(const Char8*)), static_cast<USize>(alignof(const Char8*)));
            if (!r1.IsOk()) return; auto* np1 = reinterpret_cast<const Char8**>(r1.OkValue());
            for (USize i=0;i<m_texKeyCount;++i) np1[static_cast<size_t>(i)] = m_texKeyPtrs ? m_texKeyPtrs[static_cast<size_t>(i)] : nullptr;
            if (m_texKeyPtrs) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(const_cast<Char8**>(m_texKeyPtrs)), m_texKeyCap * static_cast<USize>(sizeof(const Char8*)), static_cast<USize>(alignof(const Char8*)));
            m_texKeyPtrs = np1;
            auto r2 = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(USize)), static_cast<USize>(alignof(USize)));
            if (!r2.IsOk()) return; auto* np2 = reinterpret_cast<USize*>(r2.OkValue());
            for (USize i=0;i<m_texKeyCount;++i) np2[static_cast<size_t>(i)] = m_texKeySizes ? m_texKeySizes[static_cast<size_t>(i)] : 0;
            if (m_texKeySizes) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(m_texKeySizes), m_texKeyCap * static_cast<USize>(sizeof(USize)), static_cast<USize>(alignof(USize)));
            m_texKeySizes = np2; m_texKeyCap=newCap;
        }
    };

    [[nodiscard]] inline Result<MeshHandle> LoadMesh(ResourceManager& mgr, StringView path) noexcept {
        auto h = mgr.LoadMesh(path);
        return Result<MeshHandle>::Ok(h);
    }
    [[nodiscard]] inline Result<MaterialHandle> LoadMaterial(ResourceManager& mgr, StringView path) noexcept {
        auto h = mgr.LoadMaterial(path);
        return Result<MaterialHandle>::Ok(h);
    }
    [[nodiscard]] inline Result<TextureHandle> LoadTexture(ResourceManager& mgr, StringView path) noexcept {
        auto h = mgr.LoadTexture(path);
        return Result<TextureHandle>::Ok(h);
    }

    [[nodiscard]] inline Status Unload(ResourceManager& mgr, MeshHandle h) noexcept {
        mgr.Release(h);
        return Ok(StatusDomain::Memory());
    }
    [[nodiscard]] inline Status Unload(ResourceManager& mgr, MaterialHandle h) noexcept {
        mgr.Release(h);
        return Ok(StatusDomain::Memory());
    }
    [[nodiscard]] inline Status Unload(ResourceManager& mgr, TextureHandle h) noexcept {
        mgr.Release(h);
        return Ok(StatusDomain::Memory());
    }
}