API(Primitive.Window ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Element;Flow;Semantics;Paradigm;Text;HID)
Impl(Primitive.Window.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK HID)
Impl(Primitive.Window.Noop ${CMAKE_CURRENT_SOURCE_DIR}/Impl)
Build(Window TYPE STATIC BACKEND Primitive.Window.Windows)
