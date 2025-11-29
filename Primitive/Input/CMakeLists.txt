API(Primitive.Input ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Element;Flow;Semantics;Paradigm;Text;HID)
Impl(Primitive.Input.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK HID)
Impl(Primitive.Input.Noop ${CMAKE_CURRENT_SOURCE_DIR}/Impl)
Build(Input TYPE STATIC BACKEND Primitive.Input.Windows)
