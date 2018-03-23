#ifndef __XPL_FUNCALLNODE_H__
#define __XPL_FUNCALLNODE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace xpl {

  /**
   * Class for describing declaration for function and procedure call nodes.
   */
  class funcall_node: public cdk::expression_node {
    std::string *_name;
    cdk::sequence_node *_argument;

  public:
    inline funcall_node(int lineno, std::string *name, cdk::sequence_node *argument) :
        cdk::expression_node(lineno),
        _name(name),
        _argument(argument) {
    }

  public:
    inline std::string *name() {
      return _name;
    }
    inline cdk::sequence_node *argument() {
      return _argument;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_funcall_node(this, level);
    }

  };

} // xpl

#endif