module Prm.Input;

import Prm.Input;
import Element;
import Flow;

using namespace Prm;

Status Input::Initialize() noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
void   Input::Shutdown() noexcept {}
Expect<MouseState> Mouse::GetState() noexcept { return Expect<MouseState>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
Expect<KeyboardState> Keyboard::GetState() noexcept { return Expect<KeyboardState>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
