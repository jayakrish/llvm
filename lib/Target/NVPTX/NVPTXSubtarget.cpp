//===- NVPTXSubtarget.cpp - NVPTX Subtarget Information -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the NVPTX specific subclass of TargetSubtarget.
//
//===----------------------------------------------------------------------===//

#include "NVPTXSubtarget.h"
#include "NVPTXTargetMachine.h"

using namespace llvm;

#define DEBUG_TYPE "nvptx-subtarget"

#define GET_SUBTARGETINFO_ENUM
#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "NVPTXGenSubtargetInfo.inc"

// Pin the vtable to this file.
void NVPTXSubtarget::anchor() {}

NVPTXSubtarget &NVPTXSubtarget::initializeSubtargetDependencies(StringRef CPU,
                                                                StringRef FS) {
    // Provide the default CPU if we don't have one.
  if (CPU.empty() && FS.size())
    llvm_unreachable("we are not using FeatureStr");
  TargetName = CPU.empty() ? "sm_20" : CPU;

  ParseSubtargetFeatures(TargetName, FS);

  // Set default to PTX 3.2 (CUDA 5.5)
  if (PTXVersion == 0) {
    PTXVersion = 32;
  }

  return *this;
}

NVPTXSubtarget::NVPTXSubtarget(const Triple &TT, const std::string &CPU,
                               const std::string &FS,
                               const NVPTXTargetMachine &TM)
    : NVPTXGenSubtargetInfo(TT, CPU, FS), PTXVersion(0), SmVersion(20), TM(TM),
      InstrInfo(), TLInfo(TM, initializeSubtargetDependencies(CPU, FS)),
      TSInfo(TM.getDataLayout()), FrameLowering() {}

bool NVPTXSubtarget::hasImageHandles() const {
  // Enable handles for Kepler+, where CUDA supports indirect surfaces and
  // textures
  if (TM.getDrvInterface() == NVPTX::CUDA)
    return (SmVersion >= 30);

  // Disabled, otherwise
  return false;
}
