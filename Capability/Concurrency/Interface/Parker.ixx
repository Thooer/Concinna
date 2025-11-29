export module Cap.Concurrency:Parker;

import Lang;
// no import of Prm.Threading here to avoid cross-partition module issues

export namespace Cap {
    struct Parker {
        void* m_ev{};
        bool Init() noexcept;
        void Destroy() noexcept;
        void Park(UInt32 timeoutMs) noexcept;
        void Unpark() noexcept;
    };
}
