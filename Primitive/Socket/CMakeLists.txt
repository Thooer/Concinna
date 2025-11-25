API(Primitive.Socket ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Element;Flow;Semantics;Paradigm;Text)
Impl(Primitive.Socket.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Platform)
Impl(Primitive.Socket.Noop ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Platform)
Build(Socket TYPE STATIC BACKEND Primitive.Socket.Windows)

