#pragma once

#include "ast.h"

namespace das {
    class Module_BuiltIn : public Module {
    public:
        Module_BuiltIn();
    protected:
        void addRuntime(ModuleLibrary & lib);
        void addVectorTypes(ModuleLibrary & lib);
        void addTime(ModuleLibrary & lib);
    };
}
