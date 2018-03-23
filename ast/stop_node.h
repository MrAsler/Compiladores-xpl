#ifndef __XPL_STOPNODE_H__
#define __XPL_STOPNODE_H__

namespace xpl {
  /**
   * Class for describing stop nodes.
   */
  class stop_node: public cdk::basic_node {

  public:
    inline stop_node(int lineno) :
        cdk::basic_node(lineno) {
    }

  public:

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_stop_node(this, level);
    }

  };
} // xpl

#endif
