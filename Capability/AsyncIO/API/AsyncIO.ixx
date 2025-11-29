module;
export module Cap.AsyncIO;
import Lang;
import Prm.IO;

export namespace Cap {
    export struct AsyncRequest { Prm::AsyncRequest req; };
    export class AsyncFile {
    public:
        AsyncFile() noexcept = default;
        explicit AsyncFile(Prm::FileHandle h) noexcept : m_h(h) {}
        ~AsyncFile() noexcept { Close(); }
        static Expect<AsyncFile> OpenRead(Span<const Char8, DynamicExtent> path) noexcept;
        static Expect<AsyncFile> OpenWrite(Span<const Char8, DynamicExtent> path, bool append) noexcept;
        Status Read(Span<Byte, DynamicExtent> buffer, UInt64 offset, AsyncRequest& r) noexcept;
        Status Write(Span<const Byte, DynamicExtent> data, UInt64 offset, AsyncRequest& r) noexcept;
        Status Cancel(const AsyncRequest& r) noexcept;
        Expect<bool> Check(const AsyncRequest& r, bool wait, USize& outBytes) const noexcept;
        void Close() noexcept;
        Prm::FileHandle Handle() const noexcept { return m_h; }
    private:
        Prm::FileHandle m_h{};
    };
}
