API(Primitive.Debug ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Element;Flow;Semantics;Paradigm;Text)
Impl(Primitive.Debug.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl)
Impl(Primitive.Debug.Noop ${CMAKE_CURRENT_SOURCE_DIR}/Impl)
Build(Debug TYPE STATIC BACKEND Primitive.Debug.Windows)
