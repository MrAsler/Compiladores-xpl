#ifndef __XPL_FUNCTIONNODE_H__
#define __XPL_FUNCTIONNODE_H__

#include <cdk/basic_type.h>
#include <cdk/ast/sequence_node.h>
#include <ast/decl_function_node.h>
#include <ast/body_node.h>

namespace xpl {
  /**
   * Class for describing function nodes.
   */
  class function_node: public cdk::basic_node {
    bool _toimport;
    bool _toexport;
    basic_type *_type;
    std::string *_name;
    cdk::sequence_node *_argument;
    cdk::expression_node *_literal;
    cdk::basic_node *_body;

  public:
    inline function_node(
      int lineno,
      bool toimport,
      bool toexport,
      basic_type *type,
      std::string *name,
      cdk::sequence_node *argument,
      cdk::expression_node *literal,
      cdk::basic_node *body) :

        cdk::basic_node(lineno),
        _toimport(toimport),
        _toexport(toexport),
        _type(type),
        _name(name),
        _argument(argument),
        _literal(literal),
        _body(body) {}

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
    inline cdk::expression_node *literal() {
      return _literal;
    }
    inline cdk::basic_node *body() {
      return _body;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_node(this, level);
    }

  };
} // xpl

#endif
