API(Primitive.HID ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Element;Flow)
Impl(Primitive.HID.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl)
Build(HID TYPE STATIC BACKEND Primitive.HID.Windows)

