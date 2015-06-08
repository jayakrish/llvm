//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormattedStream.h"
using namespace llvm;

#define DEBUG_TYPE "hello"

STATISTIC(HelloCounter, "Counts number of functions greeted");
STATISTIC(InstructionCounter, "Counts number of functions seen");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct Hello : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    Hello() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      ++HelloCounter;
      errs() << "Hello: ";
      errs().write_escaped(F.getName()) << '\n';
      return false;
    }
  };
}

char Hello::ID = 0;
static RegisterPass<Hello> X("hello", "Hello World Pass");

namespace {
  // Hello2 - The second implementation with getAnalysisUsage implemented.
  struct Hello2 : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    Hello2() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      ++HelloCounter;
      errs() << "Hello: ";
      errs().write_escaped(F.getName()) << '\n';
      return false;
    }

    // We don't modify the program, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
  };
}

char Hello2::ID = 0;
static RegisterPass<Hello2>
Y("hello2", "Hello World Pass (with getAnalysisUsage implemented)");


namespace {
    /// CountStaticInstructions: Counts the number of instructions in a module
    struct CountStaticInstructions : public ModulePass {
        static char ID; // Pass identification, replacement for typeid
        DenseMap<const char*, uint> instCountMap;
        CountStaticInstructions() : ModulePass(ID) {}
        
        bool runOnModule(Module &M)
        {
            /// Iterate over functions in a module, then, over basic blocks in a function, finally, over instructions
            /// in a basic block
            for(Module::iterator MItr = M.begin(); MItr != M.end(); MItr++)
            {
                for(Function::iterator FItr = MItr->begin(); FItr != MItr->end(); FItr++)
                {
                    errs() << "Basic Block (name = " << FItr->getName() << ") has " << FItr->size() << " instructions.\n";
                    
                    for(BasicBlock::iterator BBItr = FItr->begin(); BBItr != FItr->end(); BBItr++)
                    {
                        const char* instName  = BBItr->getOpcodeName();
                        uint        instCount = instCountMap[instName];
                        
                        instCountMap[instName] = ++instCount;
                        
                    }
                    
                    InstructionCounter = InstructionCounter + FItr->size();
                }
            }
            
            errs() << "Total number of instructions in this module = " << InstructionCounter <<".\n";
           
            return false;
        }
        
        
        void print(raw_ostream& O, const Module* M) const
        {
            formatted_raw_ostream Out(O);
            DenseMap<const char*, uint>::const_iterator mapItr;
            for(mapItr = instCountMap.begin(); mapItr != instCountMap.end(); mapItr++)
            {
                Out.PadToColumn(5)  << mapItr->first;
                Out.PadToColumn(50) << mapItr->second;
                Out << "\n";
            }
        }
        
    };
}

char CountStaticInstructions::ID = 0;
static RegisterPass<CountStaticInstructions>
Z("CountStaticInstructions", "Counts the number of static instructions in a module");
