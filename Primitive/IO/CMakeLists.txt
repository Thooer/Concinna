API(Primitive.IO ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Element;Flow;Semantics;Paradigm;Text)
Impl(Primitive.IO.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl)
Impl(Primitive.IO.Noop ${CMAKE_CURRENT_SOURCE_DIR}/Impl)
Build(IO TYPE STATIC BACKEND Primitive.IO.Windows)
