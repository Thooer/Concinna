module Prm.Input;

import Prm.Input;
import Lang.Element;
import Lang.Flow;

using namespace Prm;

Status Input::Initialize() noexcept { return Ok(StatusDomain::System()); }
void   Input::Shutdown() noexcept {}
Expect<GamepadState> Gamepad::GetState(UInt32) noexcept { return Expect<GamepadState>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
