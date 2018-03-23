#ifndef __XPL_DECL_FUNCTIONNODE_H__
#define __XPL_DECL_FUNCTIONNODE_H__

#include <cdk/basic_type.h>
#include <cdk/ast/sequence_node.h>

namespace xpl {
  /**
   * Class for describing declaration for function and proc nodes.
   */
  class decl_function_node: public cdk::basic_node {
    bool _toimport;
    bool _toexport;
    basic_type *_type;
    std::string *_name;
    cdk::sequence_node *_argument;

  public:
    inline decl_function_node(
      int lineno,
      bool toimport,
      bool toexport,
      basic_type *type,
      std::string *name,
      cdk::sequence_node *argument) :

        cdk::basic_node(lineno),
        _toimport(toimport),
        _toexport(toexport),
        _type(type),
        _name(name),
        _argument(argument) {
        }

  public:
    inline bool toImport() {
      return _toimport;
    }
    inline bool toExport() {
      return _toexport;
    }
    inline basic_type *type() {
      return _type;
    }
    inline std::string *name() {
      return _name;
    }
    inline cdk::sequence_node *argument() {
      return _argument;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_decl_function_node(this, level);
    }

  };
} // xpl

#endif
