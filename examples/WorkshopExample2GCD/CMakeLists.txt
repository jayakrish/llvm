set(LLVM_LINK_COMPONENTS
  BitWriter
  Core
  ExecutionEngine
  Interpreter
  MC
  Support
  nativecodegen
  )

add_llvm_example(WorkshopExample2GCD
  WorkshopExample2GCD.cpp
  )
