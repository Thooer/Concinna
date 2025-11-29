module Prm.HID;

import Prm.HID;
import Lang.Element;
import Lang.Flow;

using namespace Prm;

Status HID::RegisterDevices(void* nativeWindowHandle) noexcept {
    return Err(StatusDomain::System(), StatusCode::Unsupported);
}

Status HID::UnregisterDevices() noexcept {
    return Err(StatusDomain::System(), StatusCode::Unsupported);
}

Status HID::UpdateFromRawEvent(const void* data, USize size) noexcept {
    return Err(StatusDomain::System(), StatusCode::Unsupported);
}

Expect<KeyboardState> HID::GetKeyboardState() noexcept {
    return Expect<KeyboardState>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
}

Expect<MouseState> HID::GetMouseState() noexcept {
    return Expect<MouseState>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
}

Expect<GamepadState> HID::GetGamepadState(UInt32) noexcept {
    return Expect<GamepadState>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
}

