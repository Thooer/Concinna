API(Primitive.File ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Element;Flow;Semantics;Paradigm;Text)
Impl(Primitive.File.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl)
Impl(Primitive.File.Noop ${CMAKE_CURRENT_SOURCE_DIR}/Impl)
Build(File TYPE STATIC BACKEND Primitive.File.Windows)
