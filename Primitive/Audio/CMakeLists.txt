API(Primitive.Audio ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Element;Flow;Semantics;Paradigm;Text)
Impl(Primitive.Audio.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl)
Impl(Primitive.Audio.Noop ${CMAKE_CURRENT_SOURCE_DIR}/Impl)
Build(Audio TYPE STATIC BACKEND Primitive.Audio.Windows)
