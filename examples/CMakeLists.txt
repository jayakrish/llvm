add_subdirectory(BrainF)
add_subdirectory(Fibonacci)
add_subdirectory(HowToUseJIT)
add_subdirectory(Kaleidoscope)
add_subdirectory(ModuleMaker)
add_subdirectory(WorkshopExample1MulAdd)
add_subdirectory(WorkshopExample2GCD)


if( ( NOT WIN32 ) AND ( NOT "${LLVM_NATIVE_ARCH}" STREQUAL "ARM" ) )
    add_subdirectory(ExceptionDemo)
endif()

if( HAVE_PTHREAD_H )
  add_subdirectory(ParallelJIT)
endif( HAVE_PTHREAD_H )
