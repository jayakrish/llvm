// Ex 1: How to write an LLVM Module

// HelloModule.cpp

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Verifier.h"
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
Function* mul_add;

int main(int argc, char**argv) {

    // Create an LLVM Module by calling makeLLVMModule()
    // Access the LLVM Module through a module owner
    std::unique_ptr<Module> Owner(makeLLVMModule());
    Module* Mod = Owner.get();

    // Verify the LLVM Module - check syntax and semantics
    verifyModule(*Mod, &errs());

    // Create a 'Pass Manager' for the compiler pipeline
    // Configure the Pass Manager with available / custom passes
    legacy::PassManager PM;
    PM.add(createPrintModulePass(outs()));

    // Run the LLVM Module through the pipeline managed by the Pass Manager.
    // The pipeline has a single 'print' pass which will stream out the LLVM Module as LLVM IR
    PM.run(*Mod);

    // Now let's check if the LLVM Module is functionally correct. To do this, we will test the module
    // with concrete values, execute the module on an 'ExecutionEngine' (which can be configured as a
    // JIT compiler or an interpreter), and verify if the output matches the expected result.
    std::string errStr;
    ExecutionEngine *Engine = EngineBuilder(std::move(Owner)).setErrorStr(&errStr).create();

    if (!Engine) {
        errs() << argv[0] << ": Failed to construct the Execution Engine: " << errStr << "\n";
        return -1; // Add a error macro
    }

    // Input values 2,3,4.
    // Expected value: mul_add(2,3,4) = 2 * 3 + 4 = 10
    std::vector<GenericValue> Args(3);
    Args[0].IntVal = APInt(32, 2);
    Args[1].IntVal = APInt(32, 3);
    Args[2].IntVal = APInt(32, 4);

    GenericValue GV = Engine->runFunction(mul_add, Args);

    outs() << "\nCalling mul_add( " << Args[0].IntVal << ", " << Args[1].IntVal << ", " << Args[2].IntVal << " )";
    outs() << "\nResult: " << GV.IntVal << "\n";
    if (GV.IntVal == (Args[0].IntVal * Args[1].IntVal + Args[2].IntVal))
    {
        outs() << "\n\nResult matches expectation!!";
        outs() << "\nMyFirstModule is a perfect LLVM Module :)";
    }
    else
    {
        outs() << "\n\nOops! Result doesn't match expectation. Check mul_add again.";
    }

    delete Mod;
    return 0;
}

// This function creates an in-memory LLVM module. The module will have
// a basic block and a function that takes in 3 arguments, multiplies the first
// two and adds the result to the third.
Module* makeLLVMModule() {

    // Module Construction
    // Create a new module "test" in an LLVM Context
    Module* Mod = new Module("MyFirstModule", getGlobalContext());

    // Insert a function "mul_add" to module "test".
    // "mul_add" has 3 input parameters and 1 return value
    Constant* C = Mod->getOrInsertFunction("mul_add",
        /*ret type*/                            IntegerType::get(getGlobalContext(), 32),
        /*arg 0*/                               IntegerType::get(getGlobalContext(), 32),
        /*arg 1*/                               IntegerType::get(getGlobalContext(), 32),
        /*arg 2*/                               IntegerType::get(getGlobalContext(), 32),
        /*varargs terminated with null*/        NULL);

    mul_add = cast<Function>(C);

    // Set up the arguments for "mul_add"
    Function::arg_iterator Args = mul_add->arg_begin();
    Value* x = Args++;
    x->setName("x");
    Value* y = Args++;
    y->setName("y");
    Value* z = Args++;
    z->setName("z");

    // Create a basic block with label "entry". Add it to the function "mul_add"
    BasicBlock* Block = BasicBlock::Create(getGlobalContext(), "entry", mul_add);

    // Create an IR builder for this basic block. IRBuilder is a utility that helps create
    // IR instructions in the basic block.
    IRBuilder<> Builder(Block);

    // Use the IR builder to create a 'mul' binary operation to multiply the first 2 arguments.
    // Store the result in "tmp"
    Value* tmp = Builder.CreateBinOp(Instruction::Mul, x, y, "tmp");

    // Use the IR builder to create an 'add' binary operation to add "tmp" to the 3rd argument.
    // Store the result in "tmp2"
    Value* tmp2 = Builder.CreateBinOp(Instruction::Add, tmp, z, "tmp2");

    // Use the IR builder to create a 'ret' instruction to return "tmp2"
    Builder.CreateRet(tmp2);

    // Done with all the instructions, now return the LLVM IR module with the "mul_add" function
    return Mod;
}