// $Id: evaluation_node.h,v 1.2 2017/04/16 00:24:07 ist181926 Exp $
#ifndef __XPL_EVALUATIONNODE_H__
#define __XPL_EVALUATIONNODE_H__

#include <cdk/ast/expression_node.h>

namespace xpl {
  /**
   * Class for describing evaluation nodes.
   */
  class evaluation_node: public cdk::basic_node {
    cdk::expression_node *_argument;

  public:
    inline evaluation_node(int lineno, cdk::expression_node *argument) :
        cdk::basic_node(lineno), _argument(argument) {
    }

  public:
    inline cdk::expression_node *argument() {
      return _argument;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_evaluation_node(this, level);
    }

  };

} // xpl

#endif
