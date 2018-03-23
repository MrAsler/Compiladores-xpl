#ifndef __XPL_ADDRESSNODE_H__
#define __XPL_ADDRESSNODE_H__

namespace xpl {

  /**
   * Class for describing the adressing (?) operator
   */
  class address_node: public cdk::unary_expression_node {
    
  public:
    inline address_node(int lineno, cdk::expression_node *arg) :
        cdk::unary_expression_node(lineno, arg) {
    }

    /**
     * @param sp semantic processor visitor
     * @param level syntactic tree level
     */
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_address_node(this, level);
    }

  };

} // xpl

#endif