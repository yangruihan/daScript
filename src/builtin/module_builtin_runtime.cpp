#include "daScript/misc/platform.h"

#include "module_builtin.h"

#include "daScript/ast/ast_interop.h"

#include "daScript/simulate/runtime_array.h"
#include "daScript/simulate/runtime_table.h"
#include "daScript/simulate/runtime_profile.h"
#include "daScript/simulate/hash.h"

namespace das
{
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

    struct MarkFunctionAnnotation : FunctionAnnotation {
        MarkFunctionAnnotation(const string & na) : FunctionAnnotation(na) { }
        virtual bool apply(ExprBlock *, const AnnotationArgumentList &, string & err) override {
            err = "not supported for block";
            return false;
        }
        virtual bool finalize(ExprBlock *, const AnnotationArgumentList &, string &) override {
            return true;
        }
        virtual bool finalize(const FunctionPtr &, const AnnotationArgumentList &, string &) override {
            return true;
        }
    };

    struct ExportFunctionAnnotation : MarkFunctionAnnotation {
        ExportFunctionAnnotation() : MarkFunctionAnnotation("export") { }
        virtual bool apply(const FunctionPtr & func, const AnnotationArgumentList &, string &) override {
            func->exports = true;
            return true;
        };
    };

    struct SideEffectsFunctionAnnotation : MarkFunctionAnnotation {
        SideEffectsFunctionAnnotation() : MarkFunctionAnnotation("sideeffects") { }
        virtual bool apply(const FunctionPtr & func, const AnnotationArgumentList &, string &) override {
            func->sideEffectFlags |= uint32_t(SideEffects::userScenario);
            return true;
        };
    };
    
    struct RunAtCompileTimeFunctionAnnotation : MarkFunctionAnnotation {
        RunAtCompileTimeFunctionAnnotation() : MarkFunctionAnnotation("run") { }
        virtual bool apply(const FunctionPtr & func, const AnnotationArgumentList &, string &) override {
            func->hasToRunAtCompileTime = true;
            return true;
        };
    };

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    // core functions

    void builtin_throw ( char * text, Context * context ) {
        context->throw_error(text);
    }

    void builtin_print ( char * text, Context * context ) {
        context->to_out(text);
    }

    vec4f builtin_breakpoint ( Context & context, SimNode_CallBase * call, vec4f * ) {
        context.breakPoint(call->debug.column, call->debug.line);
        return v_zero();
    }

    void builtin_stackwalk ( Context * context) {
        context->stackWalk();
    }

    void builtin_terminate ( Context * context ) {
        context->throw_error("terminate");
    }

    int builtin_table_size ( const Table * arr ) {
        return arr->size;
    }

    int builtin_table_capacity ( const Table * arr ) {
        return arr->capacity;
    }

    void builtin_table_clear ( Table * arr, Context * context ) {
        table_clear(*context, *arr);
    }

    vec4f _builtin_hash ( Context & context, SimNode_CallBase * call, vec4f * args ) {
        auto uhash = hash_value(context, args[0], call->types[0]);
        return cast<uint32_t>::from(uhash);
    }

    void Module_BuiltIn::addRuntime(ModuleLibrary & lib) {
        // function annotations
        addAnnotation(make_shared<ExportFunctionAnnotation>());
        addAnnotation(make_shared<SideEffectsFunctionAnnotation>());
        addAnnotation(make_shared<RunAtCompileTimeFunctionAnnotation>());
        // functions
        addExtern<DAS_BIND_FUN(builtin_throw)>         (*this, lib, "throw", SideEffects::modifyExternal);
        addExtern<DAS_BIND_FUN(builtin_print)>         (*this, lib, "print", SideEffects::modifyExternal);
        addExtern<DAS_BIND_FUN(builtin_terminate)> (*this, lib, "terminate", SideEffects::modifyExternal);
        addExtern<DAS_BIND_FUN(builtin_stackwalk)> (*this, lib, "stackwalk", SideEffects::modifyExternal);
        addInterop<builtin_breakpoint,void>     (*this, lib, "breakpoint", SideEffects::modifyExternal);
        // function-like expresions
        addCall<ExprAssert>         ("assert");
        addCall<ExprStaticAssert>   ("static_assert");
        addCall<ExprDebug>          ("debug");
        // hash
        addInterop<_builtin_hash,uint32_t,vec4f>(*this, lib, "hash", SideEffects::none);
        // table functions
        addExtern<DAS_BIND_FUN(builtin_table_clear)>(*this, lib, "clear", SideEffects::modifyArgument);
        addExtern<DAS_BIND_FUN(builtin_table_size)>(*this, lib, "length", SideEffects::none);
        addExtern<DAS_BIND_FUN(builtin_table_capacity)>(*this, lib, "capacity", SideEffects::none);
        // table expressions
        addCall<ExprErase>("__builtin_table_erase");
        addCall<ExprFind>("__builtin_table_find");
        addCall<ExprKeys>("keys");
        addCall<ExprValues>("values");
        // blocks
        addCall<ExprInvoke>("invoke");
        // profile
        addExtern<DAS_BIND_FUN(builtin_profile)>(*this,lib,"profile", SideEffects::modifyExternal);
        addString(lib);
    }
}
