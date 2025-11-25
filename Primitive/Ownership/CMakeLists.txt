API(Primitive.Ownership ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Element;Flow;Semantics;Paradigm;Text)
Impl(Primitive.Ownership.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Platform)
Impl(Primitive.Ownership.Noop ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Platform)
Build(Ownership TYPE STATIC BACKEND Primitive.Ownership.Windows)

