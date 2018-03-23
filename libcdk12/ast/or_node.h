#ifndef __CDK12_AST_OR_NODE_H__
#define __CDK12_AST_OR_NODE_H__

#include <cdk/ast/binary_expression_node.h>

namespace cdk {

  /**
   * Class for describing the logical not operator
   */
  class or_node: public binary_expression_node {

  public:
    /**
     * @param lineno source code line number for this node
     * @param left first operand
     * @param right second operand
     */
    inline or_node(int lineno, expression_node *left, expression_node *right) :
        binary_expression_node(lineno, left, right) {
    }

    /**
     * @param sp semantic processor visitor
     * @param level syntactic tree level
     */
    virtual void accept(basic_ast_visitor *sp, int level) {
      sp->do_or_node(this, level);
    }

  };

} // xpl

#endif
