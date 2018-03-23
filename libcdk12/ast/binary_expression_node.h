#ifndef __CDK12_AST_BINARYEXPRESSION_NODE_H__
#define __CDK12_AST_BINARYEXPRESSION_NODE_H__

#include <cdk/ast/expression_node.h>

namespace cdk {

  /**
   * Class for describing binary operators.
   */
  class binary_expression_node: public expression_node {
    expression_node *_left, *_right;

  public:
    /**
     * @param lineno source code line number for this node
     * @param left first operand
     * @param right second operand
     */
    inline binary_expression_node(int lineno, expression_node *left, expression_node *right) :
        expression_node(lineno), _left(left), _right(right) {
    }

    inline expression_node *left() {
      return _left;
    }
    inline expression_node *right() {
      return _right;
    }

  };

} // cdk

#endif
