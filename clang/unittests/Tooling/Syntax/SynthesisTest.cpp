//===- SynthesisTest.cpp --------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file tests synthesis API for syntax trees.
//
//===----------------------------------------------------------------------===//

#include "TreeTestBase.h"
#include "clang/Tooling/Syntax/BuildTree.h"
#include "clang/Tooling/Syntax/Nodes.h"
#include "gtest/gtest.h"

using namespace clang;
using namespace clang::syntax;

namespace {

class SynthesisTest : public SyntaxTreeTest {
protected:
  ::testing::AssertionResult treeDumpEqual(syntax::Node *Root, StringRef Dump) {
    if (!Root)
      return ::testing::AssertionFailure()
             << "Root was not built successfully.";

    auto Actual = StringRef(Root->dump(Arena->getSourceManager())).trim().str();
    auto Expected = Dump.trim().str();
    // EXPECT_EQ shows the diff between the two strings if they are different.
    EXPECT_EQ(Expected, Actual);
    if (Actual != Expected) {
      return ::testing::AssertionFailure();
    }
    return ::testing::AssertionSuccess();
  }
};

INSTANTIATE_TEST_CASE_P(SynthesisTests, SynthesisTest,
                        ::testing::ValuesIn(allTestClangConfigs()), );

TEST_P(SynthesisTest, Leaf_Punctuation) {
  buildTree("", GetParam());

  auto *Leaf = createLeaf(*Arena, tok::comma);

  EXPECT_TRUE(treeDumpEqual(Leaf, R"txt(
',' Detached synthesized
  )txt"));
}

TEST_P(SynthesisTest, Leaf_Keyword) {
  buildTree("", GetParam());

  auto *Leaf = createLeaf(*Arena, tok::kw_if);

  EXPECT_TRUE(treeDumpEqual(Leaf, R"txt(
'if' Detached synthesized
  )txt"));
}

TEST_P(SynthesisTest, Leaf_Identifier) {
  buildTree("", GetParam());

  auto *Leaf = createLeaf(*Arena, tok::identifier, "a");

  EXPECT_TRUE(treeDumpEqual(Leaf, R"txt(
'a' Detached synthesized
  )txt"));
}

TEST_P(SynthesisTest, Leaf_Number) {
  buildTree("", GetParam());

  auto *Leaf = createLeaf(*Arena, tok::numeric_constant, "1");

  EXPECT_TRUE(treeDumpEqual(Leaf, R"txt(
'1' Detached synthesized
  )txt"));
}

TEST_P(SynthesisTest, Tree_Empty) {
  buildTree("", GetParam());

  auto *Tree = createTree(*Arena, {}, NodeKind::UnknownExpression);

  EXPECT_TRUE(treeDumpEqual(Tree, R"txt(
UnknownExpression Detached synthesized
  )txt"));
}

TEST_P(SynthesisTest, Tree_Flat) {
  buildTree("", GetParam());

  auto *LeafLParen = createLeaf(*Arena, tok::l_paren);
  auto *LeafRParen = createLeaf(*Arena, tok::r_paren);
  auto *TreeParen = createTree(*Arena,
                               {{LeafLParen, NodeRole::LeftHandSide},
                                {LeafRParen, NodeRole::RightHandSide}},
                               NodeKind::ParenExpression);

  EXPECT_TRUE(treeDumpEqual(TreeParen, R"txt(
ParenExpression Detached synthesized
|-'(' LeftHandSide synthesized
`-')' RightHandSide synthesized
  )txt"));
}

TEST_P(SynthesisTest, Tree_OfTree) {
  buildTree("", GetParam());

  auto *Leaf1 = createLeaf(*Arena, tok::numeric_constant, "1");
  auto *Int1 = createTree(*Arena, {{Leaf1, NodeRole::LiteralToken}},
                          NodeKind::IntegerLiteralExpression);

  auto *LeafPlus = createLeaf(*Arena, tok::plus);

  auto *Leaf2 = createLeaf(*Arena, tok::numeric_constant, "2");
  auto *Int2 = createTree(*Arena, {{Leaf2, NodeRole::LiteralToken}},
                          NodeKind::IntegerLiteralExpression);

  auto *TreeBinaryOperator = createTree(*Arena,
                                        {{Int1, NodeRole::LeftHandSide},
                                         {LeafPlus, NodeRole::OperatorToken},
                                         {Int2, NodeRole::RightHandSide}},
                                        NodeKind::BinaryOperatorExpression);

  EXPECT_TRUE(treeDumpEqual(TreeBinaryOperator, R"txt(
BinaryOperatorExpression Detached synthesized
|-IntegerLiteralExpression LeftHandSide synthesized
| `-'1' LiteralToken synthesized
|-'+' OperatorToken synthesized
`-IntegerLiteralExpression RightHandSide synthesized
  `-'2' LiteralToken synthesized
  )txt"));
}

TEST_P(SynthesisTest, Statement_EmptyStatement) {
  buildTree("", GetParam());

  auto *S = createEmptyStatement(*Arena);
  EXPECT_TRUE(treeDumpEqual(S, R"txt(
EmptyStatement Detached synthesized
`-';' synthesized
  )txt"));
}
} // namespace