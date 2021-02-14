#pragma once

#include "clang/AST/ASTConsumer.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"

#include <string>

namespace crefl {

class CReflectAction : public clang::PluginASTAction {
public:
    std::string outputFile;
    bool debug, dump;

    CReflectAction();

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer
        (clang::CompilerInstance &ci, llvm::StringRef) override;
    bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string>& args) override;

protected:
    void EndSourceFileAction() override;

    clang::PluginASTAction::ActionType getActionType() override {
      return ReplaceAction;
    }
};

}
