(module
  (import "host" "log" (func $log (param i32 i32)))
  (memory (export "memory") 1)
  (data (i32.const 0) "Hallo Concinna")
  (func (export "run")
    (call $log (i32.const 0) (i32.const 15))
  )
)

