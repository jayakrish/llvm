// Ex 2: GCD Example To Show CFG
// WorkshopExample2GCD.cpp

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/IR/Constants.h"

using namespace llvm;

Module* makeLLVMModule();
Function* gcd;

int main(int argc, char**argv) {

    // Create an LLVM Module by calling makeLLVMModule()
    // Access the LLVM Module through a module owner
    std::unique_ptr<Module> Owner(makeLLVMModule());
    Module* Mod = Owner.get();
    std::error_code EC;
    std::string errStr;

    // create an ostream to write the module to
    raw_fd_ostream bcOut("gcd_example.bc", EC, sys::fs::F_None);

    // Verify the LLVM Module - check syntax and semantics
    verifyModule(*Mod, &errs());

    // Create a 'Pass Manager' for the compiler pipeline
    // Configure the Pass Manager with available / custom passes
    legacy::PassManager PM;
    PM.add(createPrintModulePass(outs()));
    PM.add(createPrintModulePass(bcOut));

    // Run the LLVM Module through the pipeline managed by the Pass Manager.
    // The pipeline has a single 'print' pass which will stream out the LLVM Module as LLVM IR
    PM.run(*Mod);

    // Now let's check if the LLVM Module is functionally correct. To do this, we will test the module
    // with concrete values, execute the module on an 'ExecutionEngine' (which can be configured as a
    // JIT compiler or an interpreter), and verify if the output matches the expected result.
    ExecutionEngine *Engine = EngineBuilder(std::move(Owner)).setErrorStr(&errStr).create();

    if (!Engine) {
        errs() << argv[0] << ": Failed to construct the Execution Engine: " << errStr << "\n";
        return -1; // Add a error macro
    }

    // Input values 8,12
    // Expected value: gcd(8,12) = 4
    std::vector<GenericValue> Args(2);
    Args[0].IntVal = APInt(32, 8);
    Args[1].IntVal = APInt(32, 12);

    GenericValue GV = Engine->runFunction(gcd, Args);

    outs() << "\nCalling  gcd( " << Args[0].IntVal << ", " << Args[1].IntVal << " )";
    outs() << "\nResult: " << GV.IntVal << "\n";

    bcOut.close();

    delete Mod;
    return 0;
}

// This function creates an in-memory LLVM module. The module will have
// a basic block and a function that takes in 3 arguments, multiplies the first
// two and adds the result to the third.
Module* makeLLVMModule() {

    // Module Construction
    // Create a new module "MySecondModule" in an LLVM Context
    Module* Mod = new Module("MySecondModule", getGlobalContext());

    // Insert a function "gcd" to module "MySecondModule"
    // "gcd" has 2 input parameters and 1 return value
    Constant* c = Mod->getOrInsertFunction("gcd",
        /*ret type*/ IntegerType::get(getGlobalContext(), 32),
        /*arg 0*/ IntegerType::get(getGlobalContext(), 32),
        /*arg 1*/ IntegerType::get(getGlobalContext(), 32),
        /*varags terminated by NULL*/ NULL);
    gcd = cast<Function>(c);

    // setup the arguments for "gcd"
    Function::arg_iterator args = gcd->arg_begin();
    Value* x = args++;
    x->setName("x");
    Value* y = args++;
    y->setName("y");

    // Create a basic block with label "entry". Add it to the function "gcd"
    BasicBlock* entry = BasicBlock::Create(getGlobalContext(),"entry", gcd);

    // Create other basic blocks to be used in this function
    BasicBlock* cond_false = BasicBlock::Create(getGlobalContext(), "cond_false", gcd);
    BasicBlock* cond_true = BasicBlock::Create(getGlobalContext(), "cond_true", gcd);
    BasicBlock* cond_false_2 = BasicBlock::Create(getGlobalContext(), "cond_false_2", gcd);
    BasicBlock* ret = BasicBlock::Create(getGlobalContext(), "return", gcd);

    // Create an IR builder for this basic block. IRBuilder is a utility that helps create
    // IR instructions in the basic block.
    IRBuilder<> builder(entry);

    // compare x and y. if x==y then goto ret branch else goto cond_false branch
    Value* xEqualsY = builder.CreateICmpEQ(x, y, "tmp");
    builder.CreateCondBr(xEqualsY, ret, cond_false);

    // SetInsertPoint to cond_false branch. All instructions inserted with builder will be 
    // inserted in cond_false branch
    builder.SetInsertPoint(cond_false);

    // compare if x < y then goto branch cond_true else goto cond_false_2 branch
    Value* xLessThanY = builder.CreateICmpULT(x, y, "tmp");
    builder.CreateCondBr(xLessThanY, cond_true, cond_false_2);

    // now set insert point to cond_true branch
    builder.SetInsertPoint(cond_true);

    // yMinusX = y -x 
    Value* yMinusX = builder.CreateSub(y, x, "tmp");

    // recur_1 = gcd(x, y-x)
    std::vector<Value*> args1;
    args1.push_back(x);
    args1.push_back(yMinusX);
    Value* recur_1 = builder.CreateCall(gcd, args1, "tmp");

    // return recur_1
    builder.CreateRet(recur_1);

    // now set insert point to cond_false2 branch
    builder.SetInsertPoint(cond_false_2);

    // xMinusY = x - y
    Value* xMinusY = builder.CreateSub(x, y, "tmp");

    // recur_2 = gcd(x-y, y);
    std::vector<Value*> args2;
    args2.push_back(xMinusY);
    args2.push_back(y);
    Value* recur_2 = builder.CreateCall(gcd, args2, "tmp");

    // return recur_2
    builder.CreateRet(recur_2);

    // set insert point to return basic block
    builder.SetInsertPoint(ret);
    builder.CreateRet(x);

    // Done with all the instructions, now return the LLVM IR module with the "mul_add" function
    return Mod;
}