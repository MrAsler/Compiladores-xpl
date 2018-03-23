#ifndef __CDK12_AST_ASSIGNMENT_NODE_H__
#define __CDK12_AST_ASSIGNMENT_NODE_H__

#include <cdk/ast/lvalue_node.h>

namespace cdk {

  /**
   * Class for describing assignment nodes.
   */
  class assignment_node: public expression_node {
    lvalue_node *_lvalue;
    expression_node *_rvalue;

  public:
    inline assignment_node(int lineno, lvalue_node *lvalue, expression_node *rvalue) :
        expression_node(lineno), _lvalue(lvalue), _rvalue(rvalue) {
    }

  public:
    inline lvalue_node *lvalue() {
      return _lvalue;
    }
    inline expression_node *rvalue() {
      return _rvalue;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_assignment_node(this, level);
    }

  };

} // cdk

#endif
