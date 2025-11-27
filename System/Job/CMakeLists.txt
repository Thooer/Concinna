API(System.Job ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Language;Memory;Concurrency)
Impl(System.Job.Core ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Language;Memory;Platform;System;Threading;Tools.Tracy)
Build(SystemJob TYPE STATIC BACKEND System.Job.Core LINK Concurrency)
