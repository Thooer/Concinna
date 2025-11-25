export module Concurrency:Parker;

import Language;
import Platform;

export namespace Concurrency {
    struct Parker {
        Platform::EventHandle m_ev{};
        bool Init() noexcept;
        void Destroy() noexcept;
        void Park(UInt32 timeoutMs) noexcept;
        void Unpark() noexcept;
    };
}