export module Cap.Concurrency:Parker;

import Language;
import Prm.Threading:Types;

export namespace Cap {
    struct Parker {
        Prm::EventHandle m_ev{};
        bool Init() noexcept;
        void Destroy() noexcept;
        void Park(UInt32 timeoutMs) noexcept;
        void Unpark() noexcept;
    };
}
