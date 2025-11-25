API(Primitive.Window ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Element;Flow;Semantics;Paradigm;Text)
Impl(Primitive.Window.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Platform)
Impl(Primitive.Window.Noop ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Platform)
Build(Window TYPE STATIC BACKEND Primitive.Window.Windows)

