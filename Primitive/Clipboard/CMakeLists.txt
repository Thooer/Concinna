API(Primitive.Clipboard ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Element;Flow;Semantics;Paradigm;Text)
Impl(Primitive.Clipboard.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Platform)
Impl(Primitive.Clipboard.Noop ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Platform)
Build(Clipboard TYPE STATIC BACKEND Primitive.Clipboard.Windows)
