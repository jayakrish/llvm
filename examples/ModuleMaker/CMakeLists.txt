set(LLVM_LINK_COMPONENTS
  BitWriter
  Core
  Support
  ExecutionEngine
  Interpreter
  MC
  nativecodegen
  )

add_llvm_example(ModuleMaker
  ModuleMaker.cpp
  )
