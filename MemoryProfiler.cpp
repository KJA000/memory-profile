#ifndef LLVM_TUTORIAL_OPTIMIZATION_MEMORY_PROFILER_H
#define LLVM_TUTORIAL_OPTIMIZATION_MEMORY_PROFILER_H

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace {
  struct MemoryProfiler : public ModulePass {
    static char ID; 
    MemoryProfiler() : ModulePass(ID) {}

    bool runOnModule(Module &M) override;

    void getAnalysisUsage(AnalysisUsage &AU) const override;
  };
}

#endif

using namespace llvm;

bool MemoryProfiler::runOnModule(Module &M) {
    LLVMContext &Context = M.getContext();
    const DataLayout &Layout = M.getDataLayout();

    FunctionType* traceMallocTy = FunctionType::get(
        Type::getVoidTy(Context),
        {Type::getInt8PtrTy(Context), Type::getInt64Ty(Context)},
        false
    );
    Function* traceMalloc = Function::Create(
        traceMallocTy, Function::ExternalLinkage, "traceMalloc", &M
    );

    FunctionType* traceLoadTy = FunctionType::get(
        Type::getVoidTy(Context),
        {Type::getInt8PtrTy(Context), Type::getInt64Ty(Context)},
        false
    );
    Function* traceLoad = Function::Create(
        traceLoadTy, Function::ExternalLinkage, "traceLoad", &M
    );

    FunctionType* traceStoreTy = FunctionType::get(
        Type::getVoidTy(Context),
        {Type::getInt8PtrTy(Context), Type::getInt64Ty(Context)},
        false
    );
    Function* traceStore = Function::Create(
        traceStoreTy, Function::ExternalLinkage, "traceStore", &M
    );

    for (Function &F : M) {
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                IRBuilder<> Builder(&I);

                if (CallInst *CI = dyn_cast<CallInst>(&I)) {
                    Function *callee = CI->getCalledFunction();
                    if (callee && callee->getName() == "malloc") {
                        Value *size = CI->getArgOperand(0);
                        IRBuilder<> InsertAfterMalloc(CI->getNextNode());
        		InsertAfterMalloc.CreateCall(traceMalloc, {CI, size});
                    }
                } if (LoadInst *LI = dyn_cast<LoadInst>(&I)) {
                        Type *type = LI->getType();
                        uint64_t size = Layout.getTypeSizeInBits(type)/8;
                        Value *addr = Builder.CreatePointerCast(LI->getPointerOperand(), Type::getInt8PtrTy(Context));
                        errs() << "Load: " << *LI << " (" << LI->getDebugLoc().getLine() << ")\n"; // Print the Load instruction and its line number
                        Builder.CreateCall(traceLoad, {addr, ConstantInt::get(Type::getInt64Ty(Context), size)});
                    } else if (StoreInst *SI = dyn_cast<StoreInst>(&I)) {
                        Type *type = SI->getValueOperand()->getType();
                        uint64_t size = Layout.getTypeSizeInBits(type)/8;
                        Value *addr = Builder.CreatePointerCast(SI->getPointerOperand(), Type::getInt8PtrTy(Context));
                        errs() << "Store: " << *SI << " (" << SI->getDebugLoc().getLine() << ")\n"; // Print the Store instruction and its line number
                        Builder.CreateCall(traceStore, {addr, ConstantInt::get(Type::getInt64Ty(Context), size)});
                    }
            }
        }
    }

    return true;
}

void MemoryProfiler::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
}

char MemoryProfiler::ID = 0;
static RegisterPass<MemoryProfiler> Y("mempf", "MemoryProfiler Pass");

