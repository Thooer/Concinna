module;
export module Cap.Stream;
import Lang;
import Prm.IO;

export namespace Cap {
    export struct StreamReadResult { USize bytes{0}; bool eof{false}; };
    export struct StreamWriteResult { USize bytes{0}; };

    export class IReader {
    public:
        virtual ~IReader() = default;
        virtual Expect<StreamReadResult> ReadBytes(Span<Byte, DynamicExtent> dst) noexcept = 0;
    };

    export class IWriter {
    public:
        virtual ~IWriter() = default;
        virtual Expect<StreamWriteResult> WriteBytes(Span<const Byte, DynamicExtent> src) noexcept = 0;
    };

    export class ISeekable {
    public:
        virtual ~ISeekable() = default;
        virtual Expect<UInt64> Seek(Int64 offset, UInt32 whence) noexcept = 0;
    };

    export class BinaryReader {
    public:
        explicit BinaryReader(IReader* src) noexcept : m_src(src) {}
        template<typename T>
        Expect<T> Read() noexcept;
        Expect<USize> ReadBytes(Span<Byte, DynamicExtent> dst) noexcept;
    private:
        IReader* m_src{nullptr};
    };

    export class FileReader : public IReader, public ISeekable {
    public:
        explicit FileReader(Prm::FileHandle h) noexcept : m_h(h) {}
        ~FileReader() noexcept { if (m_h) (void)Prm::File::Close(m_h); }
        static Expect<FileReader> Open(Span<const Char8, DynamicExtent> path) noexcept;
        Expect<StreamReadResult> ReadBytes(Span<Byte, DynamicExtent> dst) noexcept override;
        Expect<UInt64> Seek(Int64 offset, UInt32 whence) noexcept override;
        Prm::FileHandle Handle() const noexcept { return m_h; }
    private:
        Prm::FileHandle m_h{};
    };

    export class FileWriter : public IWriter, public ISeekable {
    public:
        explicit FileWriter(Prm::FileHandle h) noexcept : m_h(h) {}
        ~FileWriter() noexcept { if (m_h) (void)Prm::File::Close(m_h); }
        static Expect<FileWriter> Open(Span<const Char8, DynamicExtent> path, bool append) noexcept;
        Expect<StreamWriteResult> WriteBytes(Span<const Byte, DynamicExtent> src) noexcept override;
        Expect<UInt64> Seek(Int64 offset, UInt32 whence) noexcept override;
        Prm::FileHandle Handle() const noexcept { return m_h; }
    private:
        Prm::FileHandle m_h{};
    };

    export class MemoryMappedFile {
    public:
        static Expect<MemoryMappedFile> Map(Prm::FileHandle h, UInt64 offset, USize size, Prm::MapAccess access) noexcept;
        void Unmap() noexcept;
        void* Address() const noexcept { return m_map.address; }
        USize Size() const noexcept { return m_map.length; }
    private:
        Prm::Mapping m_map{};
    };
}
