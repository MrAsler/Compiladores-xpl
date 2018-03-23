#ifndef __XPL_SWEEPNODE_H__
#define __XPL_SWEEPNODE_H__

#include <cdk/ast/lvalue_node.h>
#include <cdk/ast/expression_node.h>

namespace xpl {
  /**
   * Class for describing sweep+/sweep- nodes.
   */
  class sweep_node: public cdk::basic_node {
    bool _signal;
    cdk::lvalue_node *_lvalue;
    cdk::expression_node *_init;
    cdk::expression_node *_condition;
    cdk::expression_node *_add; // If null, value is 1
    cdk::basic_node *_block;

  public:
    inline sweep_node(
      int lineno,
      bool signal,
      cdk::lvalue_node *lvalue,
      cdk::expression_node *init,
      cdk::expression_node *condition,
      cdk::expression_node *add,
      cdk::basic_node *block) :

        cdk::basic_node(lineno),
        _signal(signal),
        _lvalue(lvalue),
        _init(init),
        _condition(condition),
        _add(add),
        _block(block) {}

  public:
    inline bool signal() {
      return _signal;
    }
    inline cdk::lvalue_node *lvalue() {
      return _lvalue;
    }
    inline cdk::expression_node *init() {
      return _init;
    }
    inline cdk::expression_node *condition() {
      return _condition;
    }
    inline cdk::expression_node *add() {
      return _add;
    }
    inline cdk::basic_node *block() {
      return _block;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_sweep_node(this, level);
    }

  };
} // xpl

#endif
