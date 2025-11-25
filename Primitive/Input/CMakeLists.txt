API(Primitive.Input ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Prm)
Impl(Primitive.Input.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Platform)
Impl(Primitive.Input.Noop ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Platform)
Build(Input TYPE STATIC BACKEND Primitive.Input.Windows)

