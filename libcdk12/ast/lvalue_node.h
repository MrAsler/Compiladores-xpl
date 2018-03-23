#ifndef __CDK12_LEFTVALUE_H__
#define __CDK12_LEFTVALUE_H__

#include <cdk/ast/expression_node.h>
#include <string>

namespace cdk {

  /**
   * Class for describing syntactic tree leaves for lvalues.
   */
  class lvalue_node: public expression_node {
  public:
    inline lvalue_node(int lineno) :
        expression_node(lineno) {
    }

  };

} // cdk

#endif
