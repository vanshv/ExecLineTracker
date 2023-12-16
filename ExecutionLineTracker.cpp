#include "ExecutionLineTracker.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace ast_matchers;

void ELTrackerMatcher::run(const MatchFinder::MatchResult &Result)
{
  ASTContext *Ctx = Result.Context;
  const Stmt *MatchedStmt = Result.Nodes.getNodeAs<Stmt>("stmt");

  assert(MatchedStmt && "The matcher matched, so the statement should be non-null");

  SourceRange StmtRange = MatchedStmt->getSourceRange();
  StringRef StmtCode = Lexer::getSourceText(
      CharSourceRange::getTokenRange(StmtRange),
      Ctx->getSourceManager(),
      Ctx->getLangOpts());

  std::string AppendedCode = "cout<<__LINE__";
  ELTRewriter.InsertTextAfter(StmtRange.getEnd(), AppendedCode);
}

void ELTrackerMatcher::onEndOfTranslationUnit()
{
  ELTRewriter.getEditBuffer(ELTRewriter.getSourceMgr().getMainFileID())
      .write(llvm::outs());
}

ELTrackerASTConsumer::ELTrackerASTConsumer(Rewriter &R) : ELTHandler(R)
{
  StatementMatcher CallSiteMatcher = stmt(
                                         anyOf(
                                             returnStmt(),
                                             compoundStmt()))
                                         .bind("stmt");

  Finder.addMatcher(CallSiteMatcher, &ELTHandler);
}

class ELTPluginAction : public PluginASTAction
{
public:
  bool ParseArgs(const CompilerInstance &,
                 const std::vector<std::string> &) override
  {
    return true;
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(
      CompilerInstance &CI, StringRef file) override
  {

    RewriterForELT.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());

    return std::make_unique<ELTrackerASTConsumer>(RewriterForELT);
  }

private:
  Rewriter RewriterForELT;
};

static FrontendPluginRegistry::Add<ELTPluginAction> X("ELT", "Execution Line Tracker");