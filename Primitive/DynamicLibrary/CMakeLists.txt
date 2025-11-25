API(Primitive.DynamicLibrary ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Element;Flow;Semantics;Paradigm;Text;Platform)
Build(DynamicLibrary TYPE STATIC)
target_link_libraries(DynamicLibrary PUBLIC Platform Element Flow Semantics Paradigm Text)
