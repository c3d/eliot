#ifndef COMPILER_GC_H
#define COMPILER_GC_H
// ****************************************************************************
//  compiler-gc.h                                                 ELFE project
// ****************************************************************************
// 
//   File Description:
// 
//     Information connecting the ELFE/LLVM compiler to the garbage collector
// 
//
// 
// 
// 
// 
// 
// 
// ****************************************************************************
// This document is released under the GNU General Public License, with the
// following clarification and exception.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library. Thus, the terms and conditions of the
// GNU General Public License cover the whole combination.
//
// As a special exception, the copyright holders of this library give you
// permission to link this library with independent modules to produce an
// executable, regardless of the license terms of these independent modules,
// and to copy and distribute the resulting executable under terms of your
// choice, provided that you also meet, for each linked independent module,
// the terms and conditions of the license of that module. An independent
// module is a module which is not derived from or based on this library.
// If you modify this library, you may extend this exception to your version
// of the library, but you are not obliged to do so. If you do not wish to
// do so, delete this exception statement from your version.
//
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "compiler.h"

ELFE_BEGIN

struct CompilerGarbageCollectionListener : TypeAllocator::Listener
// ----------------------------------------------------------------------------
//   Listen to the garbage collection to put away LLVM data structures
// ----------------------------------------------------------------------------
{
    CompilerGarbageCollectionListener(Compiler *compiler)
        : compiler(compiler) {}

    virtual void BeginCollection();
    virtual bool CanDelete (void *obj);
    virtual void EndCollection();

    Compiler *compiler;
};


struct CompilerInfo : Info
// ----------------------------------------------------------------------------
//   Information about compiler-related data structures
// ----------------------------------------------------------------------------
{
    CompilerInfo(Tree *tree): tree(tree), global(0), function(0), closure(0) {}
    ~CompilerInfo();
    Tree *                      tree;
    llvm::GlobalValue *         global;
    llvm::Function *            function;
    llvm::Function *            closure;

    // We must mark builtins in a special way, see bug #991
    bool        IsBuiltin()     { return function && function == closure; }
};

ELFE_END

#endif // COMPILER_GC_H

