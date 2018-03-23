#ifndef __XPL_SEMANTICS_POSTFIX_WRITER_H__
#define __XPL_SEMANTICS_POSTFIX_WRITER_H__

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <cdk/symbol_table.h>
#include <cdk/emitters/basic_postfix_emitter.h>
#include "targets/basic_ast_visitor.h"
#include "targets/symbol.h"

namespace xpl {

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer: public basic_ast_visitor {
    cdk::symbol_table<xpl::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;
    int _lbl;
    int _offset = 0;      // Used for declaring local variables
    int _rtrnlbl = 0;     // Used for the return instruction so it can jump the end of the func

    std::vector<int> _nextList;     // Next and stop lists are used by whiles and sweeps,
    std::vector<int> _stopList;     // need to keep track of the current label to next/stop

    std::set<std::string> defined;  // Defined and import lists are used to know what to import
    std::set<std::string> imports;  // at the end of the program
    
    std::string _adrvar;        // Used just because of strings to know the creator's id (global)
    bool _infn = false;         // Used by alot of nodes, to know if is inside a function or not
    bool _argdcl = false;       // Used by function and decl var, to deal with offset
    typedef unsigned long int type; // For cpp

  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<xpl::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) :
        basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0) {
    }

  public:
    ~postfix_writer() {
      os().flush();
    }
    bool infn() {
      return _infn;
    }
    void infn(bool b) {
      _infn = b;
    }

    inline std::string printType (type arg) {
      switch(arg) {
        case basic_type::TYPE_INT:     
          return std::string("int");    
        case basic_type::TYPE_DOUBLE:  
          return std::string("real");      
        case basic_type::TYPE_STRING:  
          return std::string("string");
        case basic_type::TYPE_POINTER:  
          return std::string("pointer");
        case basic_type::TYPE_VOID:  
          return std::string("void");   
        case basic_type::TYPE_UNSPEC:  
          return std::string("unspec");   
        default:  
          return std::string("unknown");         
      }
    }


  private:
    /** Method used to generate sequential labels. */
    inline std::string mklbl(int lbl) {
      std::ostringstream oss;
      if (lbl < 0)
        oss << ".L" << -lbl;
      else
        oss << "_L" << lbl;
      return oss.str();
    }

    // Adds an id either to the list of defined identifiers or
    // to the list of identifiers to import
    inline void addId(std::set<std::string> *vec, std::string id) {
      vec->insert(id);
    }

    // Compares the two vectors that contain the declared pairs and
    // imported pairs and return a vector the names of what to import.
    // Takes O(N*M) but efficiency isn't important here
    inline std::set<std::string> getImports() {
      std::set<std::string> result;
      bool flag = true;

      for (auto imprt : imports) {
        flag = true;
        for (auto def : defined) {
          if (imprt == def) {
            flag = false;
            break;
          }
        }
        if (flag) {
          result.insert(imprt);
        }
      }
      return result; 
    }  

  public: // Helper methods
    // Assigns an expression to a lvalue. Used by assign node and sweep.
    void assign(cdk::lvalue_node * const lvalue, cdk::expression_node * const rvalue, int lvl);

    // int2double: Puts both expression values on stack, converting if needed
    void int2double(cdk::expression_node * const left, cdk::expression_node * const right, int lvl);

    // Returns the size of the biggest child node, which allows the binary expression
    // that is calling this to decide where to use an operation for 4 bytes or for 8 bytes
    int getSizeOfBinaryExpr(cdk::binary_expression_node * const node, int lvl);
    // double compare:  Having two doubles on stack, compares them. MUST be used 
    // before another comparision, as this leaves the dcmp value + int(0) on the stack.
    void doublecmp();

    // Verifies if the global declaration is being started with an expression
    // Needs to be done inside postfix as typechecker doesn't know if var is global
    void decl_init_check(xpl::decl_variable_node * const node, int lvl);

    // Another verification that relies on being inside a function or not, so it's made 
    // on this postfix visitor. Handles reals starting with int expressions.
    void decl_initiator(xpl::decl_variable_node * const node, int lvl);


  public:
    void do_sequence_node(cdk::sequence_node * const node, int lvl);

  public: // literals
    void do_integer_node(cdk::integer_node * const node, int lvl);
    void do_double_node(cdk::double_node * const node, int lvl);
    void do_string_node(cdk::string_node * const node, int lvl);

  public: // unary expressions
    void do_neg_node(cdk::neg_node * const node, int lvl);
    void do_not_node(cdk::not_node * const node, int lvl);
    void do_identity_node(xpl::identity_node * const node, int lvl);
    void do_memalloc_node(xpl::memalloc_node * const node, int lvl);
    void do_address_node(xpl::address_node * const node, int lvl);

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
