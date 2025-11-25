API(Primitive.Debug ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Prm)
Impl(Primitive.Debug.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Platform)
Impl(Primitive.Debug.Noop ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Platform)
Build(Debug TYPE STATIC BACKEND Primitive.Debug.Windows)

