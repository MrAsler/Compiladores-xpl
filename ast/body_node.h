#ifndef __XPL_BODYNODE_H__
#define __XPL_BODYNODE_H__

#include <cdk/ast/sequence_node.h>
#include <ast/block_node.h>

namespace xpl {
  /**
   * Class for describing body nodes.
   */
  class body_node: public xpl::block_node {

  public:
    inline body_node(int lineno, cdk::sequence_node *declarations, cdk::sequence_node *instructions) :
        xpl::block_node(lineno, declarations, instructions) { 
    }

  public:
    
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_body_node(this, level);
    }

  };
} // xpl

#endif
