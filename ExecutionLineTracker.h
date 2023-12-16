#ifndef ELT_H
#define ELT_H

#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Rewrite/Core/Rewriter.h"

class ELTrackerMatcher
    : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
  ELTrackerMatcher(clang::Rewriter &ELTRewriter) : ELTRewriter(ELTRewriter) {}
  void run(const clang::ast_matchers::MatchFinder::MatchResult &) override;
  void onEndOfTranslationUnit() override;

private:
  clang::Rewriter ELTRewriter;
  llvm::SmallSet<clang::FullSourceLoc, 8> EditedLocations;
};

class ELTrackerASTConsumer : public clang::ASTConsumer
{
public:
  ELTrackerASTConsumer(clang::Rewriter &R);
  void HandleTranslationUnit(clang::ASTContext &Ctx) override
  {
    Finder.matchAST(Ctx);
  }

private:
  clang::ast_matchers::MatchFinder Finder;
  ELTrackerMatcher ELTHandler;
};

#endif
