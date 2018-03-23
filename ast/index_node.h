#ifndef __XPL_INDEXNODE_H__
#define __XPL_INDEXNODE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>

namespace xpl {

  /**
   * A indexacao devolve o valor de uma posicao de memoria indicada por um ponteiro. 
   * Consiste de uma expressão ponteiro seguida do indice entre parenteses rectos. 
   * O resultado de uma indexacao e um left-value.
   * Exemplo (acesso a posicao indicada por p): p[0] 
   */
  class index_node: public cdk::lvalue_node {
    cdk::expression_node *_expression;
    cdk::expression_node *_shift;

  public:
    inline index_node(int lineno, cdk::expression_node *expression, cdk::expression_node *shift) :
        cdk::lvalue_node(lineno),
        _expression(expression),
        _shift(shift) {
    }

  public:
    inline cdk::expression_node *expression() {
      return _expression;
    }
    inline cdk::expression_node *shift() {
      return _shift;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_index_node(this, level);
    }

  };

} // xpl

#endif