#ifndef __XPL_READNODE_H__
#define __XPL_READNODE_H__

#include <cdk/ast/expression_node.h>

namespace xpl {
  /**
   * Leitura
      A operacao de leitura de um valor inteiro ou real pode ser efectuado pela expressao @, 
      que devolve o valor lido, de acordo com o tipo esperado (inteiro ou real). 
      Caso se use como argumento dos operadores de impressao (! ou !!), deve ser lido um inteiro.
      Exemplos: a = @ (leitura para a), f(@) (leitura para argumento de funcao), @!! (leitura e impressao). 
   */
  class read_node: public cdk::expression_node {

  public:
    inline read_node(int lineno) :
        cdk::expression_node(lineno) {
    }

  public:
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_read_node(this, level);
    }

  };

} // xpl

#endif