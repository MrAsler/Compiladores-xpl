#ifndef __XPL_DECL_VARIABLENNODE_H__
#define __XPL_DECL_VARIABLENNODE_H__

#include <cdk/basic_type.h>
#include <cdk/ast/expression_node.h>

namespace xpl {
  /**
   * Class for describing function nodes.
   */
  class decl_variable_node: public cdk::basic_node {
    bool _toimport;
    bool _toexport;
    basic_type *_type;
    std::string *_name;
    cdk::expression_node *_opt_init;

  public:
    inline decl_variable_node(
      int lineno,
      bool toimport,
      bool toexport,
      basic_type *type,
      std::string *name,
      cdk::expression_node *opt_init) :

        cdk::basic_node(lineno),
        _toimport(toimport),
        _toexport(toexport),
        _type(type),
        _name(name),
        _opt_init(opt_init) { 
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
    inline cdk::expression_node *init() {
      return _opt_init;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_decl_variable_node(this, level);
    }

  };

} // xpl

#endif
