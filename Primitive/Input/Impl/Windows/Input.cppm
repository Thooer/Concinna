module Platform;

import Prm;
import :Input;

using namespace Platform;

Status Input::Initialize() noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
void   Input::Shutdown() noexcept {}
Expect<MouseState> Mouse::GetState() noexcept { return Expect<MouseState>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
Expect<KeyboardState> Keyboard::GetState() noexcept { return Expect<KeyboardState>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }

