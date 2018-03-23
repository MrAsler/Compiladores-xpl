#ifndef __XPL_SEMANTICS_XMLWRITER_H__
#define __XPL_SEMANTICS_XMLWRITER_H__

#include <string>
#include <iostream>
#include <cdk/ast/basic_node.h>
#include <cdk/symbol_table.h>
#include "targets/basic_ast_visitor.h"
#include "targets/symbol.h"

namespace xpl {

  /**
   * Print nodes as XML elements to the output stream.
   */
  class xml_writer: public basic_ast_visitor {
    cdk::symbol_table<xpl::symbol> &_symtab;

  public:
    xml_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<xpl::symbol> &symtab) :
        basic_ast_visitor(compiler), _symtab(symtab) {
    }

  public:
    ~xml_writer() {
      os().flush();
    }

  private:
    inline void openTag(const std::string &tag, int lvl) {
      os() << std::string(lvl, ' ') + "<" + tag + ">" << std::endl;
    }
    inline void openTag(const cdk::basic_node *node, int lvl) {
      openTag(node->label(), lvl);
    }
    inline void closeTag(const std::string &tag, int lvl) {
      os() << std::string(lvl, ' ') + "</" + tag + ">" << std::endl;
    }
    inline void closeTag(const cdk::basic_node *node, int lvl) {
      closeTag(node->label(), lvl);
    }
    inline void lineTag(std::string name, int lvl, std::string value) {
      os() << std::string(lvl, ' ') + "<" + name + ">";
      os() << value;
      os() << "</" + name + ">"  << std::endl;
    }

    inline void xml_print(int lvl, std::string value) {
      os() <<  std::string(lvl, ' ') + value << std::endl;
    }

    inline void type_printer(int lvl, basic_type * type) {
      xml_print(lvl, type_string(type));
      if (type->subtype() != nullptr) {
        openTag("type", lvl+2);
          type_printer (lvl+4, type->subtype());
        closeTag("type", lvl+2);
      }
    }

    inline std::string type_string(basic_type * type){
      switch(type->name()) {

        case basic_type::TYPE_INT:     
          return "int";    
          break;
        case basic_type::TYPE_DOUBLE:  
          return "real";      
          break;
        case basic_type::TYPE_STRING:  
          return "string";      
          break;
        case basic_type::TYPE_POINTER: 
          return "pointer";       
          break;
        case basic_type::TYPE_VOID:    
          return "void";        
          break;

        default: 
          return "";
      }
}

  public:
    void do_sequence_node(cdk::sequence_node * const node, int lvl);

  protected:
    template<typename T>
    void process_literal(cdk::literal_node<T> * const node, int lvl) {
      os() << std::string(lvl, ' ') << "<" << node->label() << ">" << node->value() << "</" << node->label() << ">" << std::endl;
    }

  public: // literals
    void do_integer_node(cdk::integer_node * const node, int lvl);
    void do_double_node(cdk::double_node * const node, int lvl);
    void do_string_node(cdk::string_node * const node, int lvl);

  protected:
    void do_unary_expression(cdk::unary_expression_node * const node, int lvl);

  public: // unary expressions
    void do_neg_node(cdk::neg_node * const node, int lvl);
    void do_not_node(cdk::not_node * const node, int lvl);
    void do_identity_node(xpl::identity_node * const node, int lvl);
    void do_memalloc_node(xpl::memalloc_node * const node, int lvl);
    void do_address_node(xpl::address_node * const node, int lvl);

  protected:
    void do_binary_expression(cdk::binary_expression_node * const node, int lvl);

  public: // binary expressions
    void do_add_node(cdk::add_node * const node, int lvl);
    void do_sub_node(cdk::sub_node * const node, int lvl);
    void do_mul_node(cdk::mul_node * const node, int lvl);
    void do_div_node(cdk::div_node * const node, int lvl);
    void do_mod_node(cdk::mod_node * const node, int lvl);
    void do_lt_node(cdk::lt_node * const node, int lvl);
    void do_le_node(cdk::le_node * const node, int lvl);
    void do_ge_node(cdk::ge_node * const node, int lvl);
    void do_gt_node(cdk::gt_node * const node, int lvl);
    void do_ne_node(cdk::ne_node * const node, int lvl);
    void do_eq_node(cdk::eq_node * const node, int lvl);
    void do_and_node(cdk::and_node * const node, int lvl);
    void do_or_node(cdk::or_node * const node, int lvl);

  public: // expressions
    void do_identifier_node(cdk::identifier_node * const node, int lvl);
    void do_rvalue_node(cdk::rvalue_node * const node, int lvl);
    void do_assignment_node(cdk::assignment_node * const node, int lvl);
    void do_funcall_node(xpl::funcall_node * const node, int lvl);
    void do_index_node(xpl::index_node * const node, int lvl);
    void do_read_node(xpl::read_node * const node, int lvl);

  public: // basic nodes
    void do_body_node(xpl::body_node * const node, int lvl);
    void do_block_node(xpl::block_node * const node, int lvl);
    void do_evaluation_node(xpl::evaluation_node * const node, int lvl);
    void do_function_node(xpl::function_node * const node, int lvl);
    void do_next_node(xpl::next_node * const node, int lvl);
    void do_print_node(xpl::print_node * const node, int lvl);
    void do_return_node(xpl::return_node * const node, int lvl);
    void do_stop_node(xpl::stop_node * const node, int lvl);

  public : // basic nodes - declaration
    void do_decl_variable_node(xpl::decl_variable_node * const node, int lvl);
    void do_decl_function_node(xpl::decl_function_node * const node, int lvl);

  public: // basic nodes - condition
    void do_if_node(xpl::if_node * const node, int lvl);
    void do_if_else_node(xpl::if_else_node * const node, int lvl);

  public: // basic nodes - iteration
    void do_sweep_node(xpl::sweep_node * const node, int lvl);
    void do_while_node(xpl::while_node * const node, int lvl);

  };

} // xpl

#endif
