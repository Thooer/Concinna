module Prm.Input;

import Prm.Input;
import Element;
import Flow;

using namespace Prm;

Status Input::Initialize() noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
void   Input::Shutdown() noexcept {}
Expect<GamepadState> Gamepad::GetState(UInt32) noexcept { return Expect<GamepadState>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
