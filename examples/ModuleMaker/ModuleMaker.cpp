//===- examples/ModuleMaker/ModuleMaker.cpp - Example project ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This programs is a simple example that creates an LLVM module "from scratch",
// emitting it as a bitcode file to standard out.  This is just to show how
// LLVM projects work and to demonstrate some of the LLVM APIs.
//
//===----------------------------------------------------------------------===//

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRPrintingPasses.h"

//Added headers
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h" //important
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;

Function* mul_add;
int main() {
    LLVMContext Context;

    // Create the "module" or "program" or "translation unit" to hold the
    // function
    Module *Mod = new Module("test", Context);

    // Create the main function: first create the type 'int ()'
    FunctionType *FT =
        FunctionType::get(Type::getInt32Ty(Context), /*not vararg*/false);

    // By passing a module as the last parameter to the Function constructor,
    // it automatically gets appended to the Module.
    Function *F = Function::Create(FT, Function::ExternalLinkage, "main", Mod);

    // Add a basic block to the function... again, it automatically inserts
    // because of the last argument.
    BasicBlock *BB = BasicBlock::Create(Context, "EntryBlock", F);

    // Get pointers to the constant integers...
    Value *Two = ConstantInt::get(Type::getInt32Ty(Context), 2);
    Value *Three = ConstantInt::get(Type::getInt32Ty(Context), 3);

    // Create the add instruction... does not insert...
    Instruction *Add = BinaryOperator::Create(Instruction::Add, Two, Three,
        "addresult");

    // explicitly insert it into the basic block...
    BB->getInstList().push_back(Add);

    // Create the return instruction and add it to the basic block
    BB->getInstList().push_back(ReturnInst::Create(Context, Add));


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
    BasicBlock* Block = BasicBlock::Create(Context, "entry", mul_add);

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
    std::unique_ptr<Module> owner(new Module("test", Context));
    ExecutionEngine *Engine = EngineBuilder(std::move(owner)).setErrorStr(&errStr)
        .create();

    if (!Engine) {
        errs() << ": Failed to construct the Execution Engine: " << errStr << "\n";
        return -1; // Add a error macro
    }

    // Input values 2,3,4.
    // Expected value: mul_add(2,3,4) = 2 * 3 + 4 = 10
    std::vector<GenericValue> FuncArgs(3);
    FuncArgs[0].IntVal = APInt(32, 2);
    FuncArgs[1].IntVal = APInt(32, 3);
    FuncArgs[2].IntVal = APInt(32, 4);

    GenericValue GV = Engine->runFunction(mul_add, FuncArgs);

    outs() << "\nCalling mul_add( " << FuncArgs[0].IntVal << ", " << FuncArgs[1].IntVal << ", " << FuncArgs[2].IntVal << " )";
    outs() << "\nResult: " << GV.IntVal << "\n";
    if (GV.IntVal == (FuncArgs[0].IntVal * FuncArgs[1].IntVal + FuncArgs[2].IntVal))
    {
        outs() << "\n\nResult matches expectation!!";
        outs() << "\nMyFirstModule is a perfect LLVM Module :)";
    }
    else
    {
        outs() << "\n\nOops! Result doesn't match expectation. Check mul_add again.";
    }

    const char *Path = "moduleMaker.bc";
    std::error_code EC;
    raw_fd_ostream OS(Path, EC, sys::fs::F_None);
    WriteBitcodeToFile(Mod, OS);

    // Delete the module and all of its contents.
    delete Mod;
    return 0;
}
