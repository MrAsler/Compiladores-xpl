#ifndef __XPL_MEMALLOCNODE_H__
#define __XPL_MEMALLOCNODE_H__

namespace xpl {

  /**
   * Class for describing memory alocation nodes.
   */
  class memalloc_node: public cdk::unary_expression_node {

  public:
    inline memalloc_node(int lineno, expression_node *arg) :
        cdk::unary_expression_node(lineno, arg) {
    }

    /**
     * @param sp semantic processor visitor
     * @param level syntactic tree level
     */    
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_memalloc_node(this, level);
    }

  };
  
} // xpl

#endif