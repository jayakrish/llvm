set(LLVM_LINK_COMPONENTS
  BitWriter
  Core
  ExecutionEngine
  Interpreter
  MC
  Support
  nativecodegen
  )

add_llvm_example(WorkshopExample1MulAdd
  WorkshopExample1MulAdd.cpp
  )
