/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file testReturnValue.cpp
 * @brief Unit test for ReturnValue class & parser
 * @author Frank Dellaert
 * @date Nov 30, 2014
 **/

#include <wrap/ReturnValue.h>
#include <CppUnitLite/TestHarness.h>
#include <iostream>

using namespace std;
using namespace wrap;

//******************************************************************************
TEST( ReturnType, Constructor1 ) {
  ReturnType actual("Point2");
  EXPECT(actual.namespaces().empty());
  EXPECT(actual.name()=="Point2");
  EXPECT(actual.category==Qualified::CLASS);
  EXPECT(!actual.isPtr);
}

//******************************************************************************
TEST( ReturnType, Constructor2 ) {
  ReturnType actual("Point3", Qualified::CLASS, true);
  EXPECT(actual.namespaces().empty());
  EXPECT(actual.name()=="Point3");
  EXPECT(actual.category==Qualified::CLASS);
  EXPECT(actual.isPtr);
}

//******************************************************************************
// http://boost-spirit.com/distrib/spirit_1_8_2/libs/spirit/doc/grammar.html
struct ReturnTypeGrammar: public classic::grammar<ReturnTypeGrammar> {

  wrap::ReturnType& result_; ///< successful parse will be placed in here

  TypeGrammar type_g;

  /// Construct ReturnType grammar and specify where result is placed
  ReturnTypeGrammar(wrap::ReturnType& result) :
      result_(result), type_g(result_) {
  }

  /// Definition of type grammar
  template<typename ScannerT>
  struct definition {

    classic::rule<ScannerT> type_p;

    definition(ReturnTypeGrammar const& self) {
      using namespace classic;
      type_p = self.type_g >> !ch_p('*')[assign_a(self.result_.isPtr, T)];
    }

    classic::rule<ScannerT> const& start() const {
      return type_p;
    }

  };
};
// ReturnTypeGrammar

//******************************************************************************
TEST( ReturnType, grammar ) {

  using classic::space_p;

  // Create type grammar that will place result in actual
  ReturnType actual;
  ReturnTypeGrammar g(actual);

  EXPECT(parse("Point3", g, space_p).full);
  EXPECT( actual==ReturnType("Point3"));
  actual.clear();

  EXPECT(parse("Test*", g, space_p).full);
  EXPECT( actual==ReturnType("Test",Qualified::CLASS,true));
  actual.clear();

  EXPECT(parse("VectorNotEigen", g, space_p).full);
  EXPECT(actual==ReturnType("VectorNotEigen"));
  actual.clear();

  EXPECT(parse("double", g, space_p).full);
  EXPECT(actual==ReturnType("double",Qualified::BASIS));
  actual.clear();
}

//******************************************************************************
TEST( ReturnValue, Constructor ) {
  ReturnValue actual(ReturnType("Point2"), ReturnType("Point3"));
  EXPECT(actual.type1==Qualified("Point2"));
  EXPECT(actual.type2==Qualified("Point3"));
  EXPECT(actual.isPair);
}

//******************************************************************************
// http://boost-spirit.com/distrib/spirit_1_8_2/libs/spirit/doc/grammar.html
struct ReturnValueGrammar: public classic::grammar<ReturnValueGrammar> {

  wrap::ReturnValue& result_; ///< successful parse will be placed in here

  ReturnTypeGrammar returnType1_g, returnType2_g;

  /// Construct type grammar and specify where result is placed
  ReturnValueGrammar(wrap::ReturnValue& result) :
      result_(result), returnType1_g(result.type1), returnType2_g(result.type2) {
  }

  /// Definition of type grammar
  template<typename ScannerT>
  struct definition {

    classic::rule<ScannerT> pair_p, returnValue_p;

    definition(ReturnValueGrammar const& self) {

      using namespace classic;

      pair_p = (str_p("pair") >> '<' >> self.returnType1_g >> ','
          >> self.returnType2_g >> '>')[assign_a(self.result_.isPair, T)];

      returnValue_p = pair_p | self.returnType1_g;
    }

    classic::rule<ScannerT> const& start() const {
      return returnValue_p;
    }

  };
};
// ReturnValueGrammar

//******************************************************************************
TEST( ReturnValue, grammar ) {

  using classic::space_p;

  // Create type grammar that will place result in actual
  ReturnValue actual;
  ReturnValueGrammar g(actual);

  EXPECT(parse("pair<Point2,Point3>", g, space_p).full);
  EXPECT( actual==ReturnValue(ReturnType("Point2"),ReturnType("Point3")));
  actual.clear();

  EXPECT(parse("pair<Test*,Test>", g, space_p).full);
  EXPECT( actual==ReturnValue( //
      ReturnType("Test",Qualified::CLASS,true),ReturnType("Test")));
  actual.clear();

  EXPECT(parse("pair<Test,Test*>", g, space_p).full);
  EXPECT( actual==ReturnValue(ReturnType("Test"), //
      ReturnType("Test",Qualified::CLASS,true)));
  actual.clear();

  EXPECT(parse("VectorNotEigen", g, space_p).full);
  EXPECT(actual==ReturnValue(ReturnType("VectorNotEigen")));
  actual.clear();

  EXPECT(parse("double", g, space_p).full);
  EXPECT(actual==ReturnValue(ReturnType("double",Qualified::BASIS)));
  actual.clear();
}

//******************************************************************************
int main() {
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
//******************************************************************************
