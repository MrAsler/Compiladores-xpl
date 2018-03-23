#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include "targets/sizeof_calculator.h"
#include "ast/all.h"  // all.h is automatically generated

//---------------------------------------------------------------------------

void xpl::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    if (node->node(i) != nullptr) {
      node->node(i)->accept(this, lvl + 2);
    }
  }
  if (lvl == 0) {// If this is the main sequence, verify what to it needs to import
    std::set<std::string> importlist = getImports();
    for (auto elem: importlist) {
      _pf.EXTERN(elem);
    }

    // these are just a few library function imports 
    _pf.EXTERN("readi");   // Read integers
    _pf.EXTERN("readd");   // Read doubles
    _pf.EXTERN("printi");  // Print integers
    _pf.EXTERN("printd");  // Print doubles
    _pf.EXTERN("prints");  // Print strings
    _pf.EXTERN("println"); // Print newlines
    _pf.EXTERN("argc"); // get number of args
    _pf.EXTERN("argv"); // get n arg as a string
    _pf.EXTERN("envp"); // get n env arg as a string
  }
}

//------------ LITERALS -----------------------------------------------------

void xpl::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {

  // In case an integer node needs to be converted to double
  if(node->type()->name() == basic_type::TYPE_DOUBLE) {
    auto dbnode = new cdk::double_node(node->lineno(), node->value()); 
    do_double_node( dbnode, lvl);
    delete dbnode;
    return;
  }
  
  if (infn()) { // Local
    _pf.INT(node->value());   // Push integer to top of stack
  } else {      // Global
    _pf.CONST(node->value()); // Put integer on memory
  }
}

void xpl::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {

  if (infn()) { // Local
    _pf.DATA();
    _pf.ALIGN();
    _pf.LABEL(mklbl(++_lbl));
    _pf.DOUBLE(node->value());
    _pf.TEXT();
    _pf.ADDR(mklbl(_lbl));
    _pf.DLOAD();            
  } else {      // Global
    _pf.DOUBLE(node->value()); // Put double on memory
  }
}

void xpl::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  int lbl = ++_lbl;

  /* generate the string */
  _pf.RODATA(); // strings are DATA readonly
  _pf.ALIGN(); // make sure we are aligned
  _pf.LABEL(mklbl(lbl)); // give the string a name
  _pf.STR(node->value()); // output string characters

  if (infn()) { // LOCAL
    _pf.TEXT(); // return to the TEXT segment
    _pf.ADDR(mklbl(lbl)); // the string to be printed
  } else {      // GLOBAL
    _pf.ALIGN();
    _pf.LABEL(_adrvar); 
    _pf.ID(mklbl(lbl));
  }
}

//------------ UNARY EXPRESSIONS --------------------------------------------

void xpl::postfix_writer::do_neg_node(cdk::neg_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl+2); // determine the value

  // 2-complement
  if (node->argument()->type()->size() == 4) {
    _pf.NEG(); 
  } else if (node->argument()->type()->size() == 8) {
    _pf.DNEG();
  }
}

void xpl::postfix_writer::do_identity_node(xpl::identity_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  int size = node->type()->size();

  node->argument()->accept(this, lvl+2); // Push value to stack $ VAL
  size == 8 ? _pf.DDUP() : _pf.DUP();    // Duplicate value     $ VAL VAL
  _pf.INT(0);                            // Push 0 to stack     $ 0 VAL VAL
  if (size == 8) { 
    _pf.I2D(); 
    doublecmp(); 
  }         
  _pf.LE();                            // 0 > value ?         $ 0|1 VAL
  _pf.JZ(mklbl(lbl));                  // If false, ignore    $ VAL
  size == 8 ? _pf.DNEG() : _pf.NEG();  // Else, neg           $ -VAL
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

void xpl::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl+2); // Push value to stack

  _pf.NOT(); // 1-complement
}

void xpl::postfix_writer::do_memalloc_node(xpl::memalloc_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  // Put size of subtype on stack
  _pf.INT(node->type()->subtype()->size());

  // Get number of elements
  node->argument()->accept(this, lvl+2);

  // Multiply size of subtype x number of elements
  _pf.MUL();

  // Allocate total size
  _pf.ALLOC();

  // Pushes the value of the stack pointer 
  _pf.SP();

}

void xpl::postfix_writer::do_address_node(xpl::address_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->argument()->accept(this, lvl+2); // Push address to stack
}

//------------ BINARY EXPRESSIONS -------------------------------------------

void xpl::postfix_writer::int2double(cdk::expression_node * const left, cdk::expression_node * const right, int lvl) {
  type ltype = left->type()->name();
  type rtype = right->type()->name();

  left->accept(this, lvl+2);
  if ( ltype == basic_type::TYPE_INT && rtype == basic_type::TYPE_DOUBLE ) {
    _pf.I2D();
  }

  right->accept(this, lvl+2);
  if ( ltype == basic_type::TYPE_DOUBLE && rtype == basic_type::TYPE_INT ) {
    _pf.I2D();
  }
}

void xpl::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if(node->type()->name() == basic_type::TYPE_POINTER) {
    int size = 0;

    if ( (node->left()->type()->name() == basic_type::TYPE_INT)) {
      node->right()->accept(this, lvl+2); // load the pointer
      size = node->right()->type()->subtype()->size();
      node->left()->accept(this, lvl+2);  // load the shift
    } else {
      node->left()->accept(this, lvl+2);  // load the pointer
      size = node->left()->type()->subtype()->size();
      node->right()->accept(this, lvl+2); // load the shift
    } 
    
    _pf.INT(size); // Block size (4 or 8)
    _pf.MUL();     // Multiply shift by block size
    _pf.ADD();     // Add result to pointer location
    return;
  }

  int2double(node->left(), node->right(), lvl);

  if (node->type()->size() == 4) {
    _pf.ADD();
  } else if (node->type()->size() == 8) {
    _pf.DADD();
  }
}

void xpl::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if(node->type()->name() == basic_type::TYPE_POINTER) {
    int blocksize = node->left()->type()->subtype()->size();

    // Mete os dois enderecos na stack
    node->left()->accept(this, lvl+2);
    node->right()->accept(this, lvl+2);
    // Subtrai um pelo outro
    _pf.SUB();
    // Mete o tamanho do block na stack
    _pf.INT(blocksize);
    // Divide o espaco entre enderecos pelo tamanho do block   
    _pf.DIV();
    // E assim obtem o num de objectos entre eles.
    // Declara isto como int para poder imprimir
    node->type(new basic_type(4, basic_type::TYPE_INT));
    return;
  }
  int2double(node->left(), node->right(), lvl);

  if (node->type()->size() == 4) {
    _pf.SUB();
  } else if (node->type()->size() == 8) {
    _pf.DSUB();
  }
}
void xpl::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int2double(node->left(), node->right(), lvl);

  if (node->type()->size() == 4) {
    _pf.MUL();
  } else if (node->type()->size() == 8) {
    _pf.DMUL();
  }
}
void xpl::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int2double(node->left(), node->right(), lvl);

  if (node->type()->size() == 4) {
    _pf.DIV();
  } else if (node->type()->size() == 8) {
    _pf.DDIV();
  }
}
void xpl::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl+2);
  node->right()->accept(this, lvl+2);

  _pf.MOD();
}

void xpl::postfix_writer::doublecmp() {
  _pf.DCMP();
  _pf.INT(0);
}

int xpl::postfix_writer::getSizeOfBinaryExpr(cdk::binary_expression_node * const node, int lvl) {
  int leftsize = node->left()->type()->size();
  int rightsize = node->right()->type()->size();
  return (leftsize > rightsize ? leftsize : rightsize);
}

void xpl::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int2double(node->left(), node->right(), lvl);
  if (getSizeOfBinaryExpr(node, lvl) == 8) { doublecmp(); }

  _pf.LT();
}
void xpl::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int2double(node->left(), node->right(), lvl);
  if (getSizeOfBinaryExpr(node, lvl) == 8) { doublecmp(); }

  _pf.LE();
}
void xpl::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int2double(node->left(), node->right(), lvl);
  if (getSizeOfBinaryExpr(node, lvl) == 8) { doublecmp(); }

  _pf.GE();
}
void xpl::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int2double(node->left(), node->right(), lvl);
  if (getSizeOfBinaryExpr(node, lvl) == 8) { doublecmp(); }

  _pf.GT();
}
void xpl::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int2double(node->left(), node->right(), lvl);
  if (getSizeOfBinaryExpr(node, lvl) == 8) { doublecmp(); }

  _pf.NE();
}
void xpl::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int2double(node->left(), node->right(), lvl);
  if (getSizeOfBinaryExpr(node, lvl) == 8) { doublecmp(); }

  _pf.EQ();
}


void xpl::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  int fail = ++_lbl;
  int end = ++_lbl;

  // Visit left condition. Puts a value on the stack
  node->left()->accept(this, lvl+2);
  // If its 0, fail and put 0 on the stack
  _pf.JZ(mklbl(fail)); 

  // Visit right condition. Puts a value on the stack
  node->right()->accept(this, lvl+2);
  // If its 0, fail and put 0 on the stack
  _pf.JZ(mklbl(fail)); 
  // Else, put 1 on the stack and jmp to end
  _pf.INT(1);
  _pf.JMP(mklbl(end));

  _pf.ALIGN();
  _pf.LABEL(mklbl(fail));
  _pf.INT(0);

  _pf.ALIGN();
  _pf.LABEL(mklbl(end));
}
void xpl::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  int pass = ++_lbl;
  int end = ++_lbl;

  // Visit left condition. Puts a value on the stack
  node->left()->accept(this, lvl+2);
  // If its 0, continue to next condition, else jump to put 1 on stack
  _pf.JNZ(mklbl(pass)); 

  // Visit right condition. Puts a value on the stack
  node->right()->accept(this, lvl+2);
  // If its 0, put 0 on stack, else jump to put 1 on stack
  _pf.JNZ(mklbl(pass)); 
  // Both conditions failed, put 0 on stack and jump to end
  _pf.INT(0);
  _pf.JMP(mklbl(end));

  _pf.ALIGN();
  _pf.LABEL(mklbl(pass));
  _pf.INT(1);

  _pf.ALIGN();
  _pf.LABEL(mklbl(end));
}

//------------ EXPRESSIONS --------------------------------------------------

void xpl::postfix_writer::do_identifier_node(cdk::identifier_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  std::string id = node->name();
  auto symbol = _symtab.find(id);

  if (symbol->toImport()) {
    addId(&imports, id);
  }

  if (symbol->local()) { // Local
    _pf.LOCAL(symbol->value());
  } else {              // GLOBAL
    _pf.ADDR(id);
  }
}

void xpl::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int leftsize = node->lvalue()->type()->size();

  // Push address of lvalue on stack
  node->lvalue()->accept(this, lvl+2);

  // Load lvalue address and put value on stack
  if (leftsize == 4) {
    _pf.LOAD();
  } else if (leftsize == 8) {
    _pf.DLOAD();
  }
}

void xpl::postfix_writer::assign(cdk::lvalue_node * const lvalue, cdk::expression_node * const rvalue, int lvl) {
  type rtype = rvalue->type()->name();
  int rsize = rvalue->type()->size();

  rvalue->accept(this, lvl+2); // Put rvalue on stack

  // If is real receiving an integer, must convert integer to real
  if (lvalue->type()->name() == basic_type::TYPE_DOUBLE && rtype == basic_type::TYPE_INT) {
    _pf.I2D();
    rvalue->type(lvalue->type());
    rtype = rvalue->type()->name();
    rsize = rvalue->type()->size();
  }

  if (rsize == 4) {
    _pf.DUP();
  } else if (rsize == 8) {
    _pf.DDUP();
  }

  lvalue->accept(this, lvl+2);
  if (rsize == 4) {
    _pf.STORE();
  } else if (rsize == 8) {
    _pf.DSTORE();
  }
}

void xpl::postfix_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  assign(node->lvalue(), node->rvalue(), lvl);
}

void xpl::postfix_writer::do_funcall_node(xpl::funcall_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  std::string &id = *(node->name());
  auto symbol = _symtab.find(id);
  int callsize = node->type()->size();
  int argsize = 0;

  if (symbol->toImport()) {
    addId(&imports, id);
  }

  // Put arguments on stack (reverse order)
  if (node->argument()) {
    for (int i = node->argument()->size() - 1; i >= 0; i--) {
      node->argument()->node(i)->accept(this, lvl+2);
      argsize += ((cdk::expression_node *) node->argument()->node(i))->type()->size();
    }
  }

  _pf.CALL(id);

  // Remove arguments from stack
  _pf.TRASH(argsize);

  if (callsize == 4) {
    _pf.PUSH();
  } else if (callsize == 8) {
    _pf.DPUSH();
  }
}

void xpl::postfix_writer::do_index_node(xpl::index_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  // Get pointer value
  node->expression()->accept(this, lvl+2);
  // Add amount of shift to stack
  node->shift()->accept(this, lvl+2);
  // Add size to stack
  _pf.INT(node->expression()->type()->subtype()->size());
  // Multiply shift by size
  _pf.MUL();
  // Sum pointer value with total shift
  _pf.ADD();
}

void xpl::postfix_writer::do_read_node(xpl::read_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS  

  if (node->type()->name() == basic_type::TYPE_INT) {
    _pf.CALL("readi");
    _pf.PUSH();
  } else if (node->type()->name() == basic_type::TYPE_DOUBLE) {
    _pf.CALL("readd");
    _pf.DPUSH();
  }
}

/* *********************************************************************** */
//------------ BASIC NODES --------------------------------------------------
/* *********************************************************************** */

void xpl::postfix_writer::do_body_node(xpl::body_node * const node, int lvl) {
    if(node->declarations()) { node->declarations()->accept(this, lvl+2); }
    if(node->instructions()) { node->instructions()->accept(this, lvl+2); }
}
void xpl::postfix_writer::do_block_node(xpl::block_node * const node, int lvl) {
    _symtab.push();

    if(node->declarations()) { node->declarations()->accept(this, lvl+2); }
    if(node->instructions()) { node->instructions()->accept(this, lvl+2); }

    _symtab.pop();
}

void xpl::postfix_writer::do_evaluation_node(xpl::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int evalsize = node->argument()->type()->size();

  // Do the expression
  node->argument()->accept(this, lvl+2);

  // Delete the expression result
  _pf.TRASH(evalsize); 
}

void xpl::postfix_writer::do_function_node(xpl::function_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  /********** Initialization **********/
  infn(true);                         // In function (Variables declared as local)
  std::string &id = *(node->name());  // Function name
  auto symbol = _symtab.find(id);     // Get function from symtab is exists
  int retsize = node->type()->size(); // Default space for return result
  _offset = 0;                        // Reset offset value
  _rtrnlbl = ++_lbl;                  // Label for return to jump to end of function
  addId(&defined, id);                // Add function to list of defineds
  /************************************/

  if ( symbol == nullptr) {   // If symbol is not created, create
    symbol = std::make_shared<xpl::symbol>
      (node->toImport(), true, true, true, node->type(), id, 0);
    _symtab.insert(id, symbol);
  }

  if (id == "_main") { // In case there's a function with reserved name, change it
    id = "._main";
  }

  if (id == "xpl") {   //(RTS mandates the main function have name be "_main")
    id = "_main";
  }

  _pf.TEXT();
  _pf.ALIGN();
  if (node->toExport()) { _pf.GLOBAL(id, _pf.FUNC()); }
  _pf.LABEL(id);

  /********** Alocate size of all variables inside function **********/
  xpl::sizeof_calculator *visitor = new xpl::sizeof_calculator(_compiler);
  node->accept(visitor, 0);
  int size = visitor->size();
  delete visitor;
  _pf.ENTER(size);
  /***************************************************************/

  _symtab.push();

  /**************** Alocate space for arguments ******************/
  if (node->argument() != nullptr) {
    _argdcl = true; // Flag for decl_var so it knows update offset after aloc
    _offset = 8;    // Stack zone for arguments
    node->argument()->accept(this, lvl+2); 

    // If the function wasn't defined, add its arguments to list
    // Dynamic cast used because through grammar we know args are decl_vars
    if (symbol->getArgs().empty()) {
      for (size_t i = 0; i < node->argument()->size(); i++) {
        if (node->argument()->node(i) != nullptr) {   
          try {
            auto *decl = dynamic_cast <xpl::decl_variable_node*> (node->argument()->node(i)); 
            symbol->addArg( *(decl->type()) );
          } catch (std::bad_cast e) {
            // Sequence node is unreliable in its size, so this is ok.
          }
        } 
      }
    }
    _argdcl = false;
    _offset = 0;    // Stack zone for variables
  }
  /***************************************************************/

  /********** Alocate space for literal / return value **********/
  _offset -= retsize;
  if (node->literal()) {                      // If fn has a literal defined
    node->literal()->accept(this, lvl+2);     // Put it on the stack fp - retsize
    if (retsize == 4) {                       
      _pf.LOCA(_offset);
    } else if (retsize == 8) {
      _pf.LOCAL(_offset);
      _pf.DSTORE();
    }
  }
  symbol->value(_offset);
  /***************************************************************/

  node->body()->accept(this, lvl+2);

  _symtab.pop();

  _pf.ALIGN();
  _pf.LABEL(mklbl(_rtrnlbl));

  /************* If there's a return value, pop it ***************/
  if (retsize == 4) {        // Int, string or pointer 
    _pf.LOCV(-4);
    _pf.POP();
  } else if (retsize == 8) { // Double
    _pf.LOCAL(-8);
    _pf.DLOAD();
    _pf.DPOP();
  }
  /***************************************************************/

  _pf.LEAVE();
  _pf.RET();

  infn(false);
}

void xpl::postfix_writer::do_next_node(xpl::next_node * const node, int lvl) {
  if (_nextList.empty()) {
    std::cerr << node->lineno() <<": Next outside loop." << std::endl;
    exit(2);
  }

  _pf.JMP(mklbl(_nextList.back()));
}

void xpl::postfix_writer::do_print_node(xpl::print_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS; 

  node->argument()->accept(this, lvl+2);
  type argtype = node->argument()->type()->name();

  if (argtype == basic_type::TYPE_INT) {
    _pf.CALL("printi");
    _pf.TRASH(4); // delete the printed value
  } else if (argtype == basic_type::TYPE_DOUBLE) {
    _pf.CALL("printd");
    _pf.TRASH(8); // delete the printed value's address
  } else if (argtype == basic_type::TYPE_STRING) {
    _pf.CALL("prints");
    _pf.TRASH(4); // delete the printed value's address
  } else {
    std::cerr << "Print error: Can't print " << printType(argtype) << std::endl;
    exit(2);
  }

  if (node->newline()) {
    _pf.CALL("println"); // print a newline
  }
}

void xpl::postfix_writer::do_return_node(xpl::return_node * const node, int lvl) {
  _pf.JMP(mklbl(_rtrnlbl));
}

void xpl::postfix_writer::do_stop_node(xpl::stop_node * const node, int lvl) {
  if (_stopList.empty()) {
    std::cerr << node->lineno() <<": Stop outside loop." << std::endl;
    exit(2);
  } 
  _pf.JMP(mklbl(_stopList.back()));
}

//------------ BASIC NODES - DECLARATION ------------------------------------

void xpl::postfix_writer::decl_init_check(xpl::decl_variable_node * const node, int lvl) {
  /* A global variable's init must be a literal */
  // Sadly I was unable to find how to pass 
  // a generic type to dynamic cast a literal_node (like literal_node<_>),
  // so this has to be checked in an hardcoded manner

  try {
    if ( &(dynamic_cast <cdk::integer_node&> (*node->init())) != nullptr ) { return; } 
  } catch (std::bad_cast e) { }
  try { 
    if ( &(dynamic_cast <cdk::double_node&> (*node->init())) != nullptr ) { return; }
  } catch (std::bad_cast e) { }
  try { 
    if ( &(dynamic_cast <cdk::string_node&> (*node->init())) != nullptr ) { return; }
  } catch (std::bad_cast e) { }
    
  std::cerr << node->lineno() <<": A global variable can only be initialized with a literal" << std::endl;
  exit(2);
}

void xpl::postfix_writer::decl_initiator(xpl::decl_variable_node * const node, int lvl) {
  if (infn()) {
    node->init()->accept(this, lvl+2);  
    if (node->type()->size() == 8 && node->init()->type()->name() == basic_type::TYPE_INT) {
        _pf.I2D();
    }
  } else {
    if (node->type()->size() == 8) {
      node->init()->type(new basic_type(8, basic_type::TYPE_DOUBLE)); 
    }
    node->init()->accept(this, lvl+2);
  }
}

void xpl::postfix_writer::do_decl_variable_node(xpl::decl_variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  std::string &id = *(node->name());
  int varsize = node->type()->size();
  bool init = (node->init() != nullptr);

  // Symbol value saves the address of the variable value
  auto symbol = 
    std::make_shared<xpl::symbol>(node->toImport(), infn(), false, false, node->type(), id, 0);
  _symtab.insert(id, symbol);


  if (node->toImport()) {
    return;
  } else {
    addId(&defined, id);
  }
  
  if (infn()) {   // LOCAL

    if (!_argdcl) { _offset -= varsize; } // If local, update offset first
    symbol->value(_offset);               // Save offset in variable's symbol value
    if (init) {                           // Only needs to save info if is inited
      decl_initiator(node, lvl);          // Put expression value on stack
      _pf.LOCAL(_offset);                 // Write value on offset location
      varsize == 8 ? _pf.DSTORE() : _pf.STORE();
    }
    if (_argdcl) { _offset += varsize; } // If argument, update offset after

  } else {      // GLOBAL

    if (init) { decl_init_check(node, lvl+2); }
    _adrvar = id;       // Saving in global variable var id in case of string
    init ? _pf.DATA() : _pf.BSS();
    _pf.ALIGN();
    if (node->toExport()) { _pf.GLOBAL(id, _pf.OBJ()); }  
    if (node->type()->name() != basic_type::TYPE_STRING ) { _pf.LABEL(id); }
    init ? decl_initiator(node, lvl) : _pf.BYTE(varsize); 
  }
} 

void xpl::postfix_writer::do_decl_function_node(xpl::decl_function_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  std::string &id = *(node->name());
  auto symbol = std::make_shared<xpl::symbol>
    (node->toImport(), true, true, false, node->type(), id, 0);

  // Add function arguments to symbol
  for (size_t i = 0; i < node->argument()->size(); i++) {
    if (node->argument()->node(i) != nullptr) {   
      try {
        auto *decl = dynamic_cast <xpl::decl_variable_node*> (node->argument()->node(i)); 
        symbol->addArg( *(decl->type()) );
      } catch (std::bad_cast e) {
      // Sequence node is unreliable in its size, so this is ok.
      }
    } 
  }

  if (!_symtab.insert(id, symbol)) {
    throw std::string("Error inserting new function " + id + " symbol.");
  }
}

//------------ BASIC NODES - CONDITION --------------------------------------

void xpl::postfix_writer::do_if_node(xpl::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int lbl1;

  node->condition()->accept(this, lvl+2);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl+2);
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1));
}

void xpl::postfix_writer::do_if_else_node(xpl::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int lbl1, lbl2;

  node->condition()->accept(this, lvl+2);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl+2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl+2);
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1 = lbl2));
}

//------------ BASIC NODES - ITERATION --------------------------------------

void xpl::postfix_writer::do_sweep_node(xpl::sweep_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;

    int condition = ++_lbl;
    int continu = ++_lbl;
    int end = ++_lbl;
    int lvalsize = node->lvalue()->type()->size();
    auto rvalue_node = new cdk::rvalue_node(node->lineno(), node->lvalue());

    _nextList.push_back(continu);
    _stopList.push_back(end);


    // ********** ASSIGNMENT **************
    assign(node->lvalue(), node->init(), lvl);
    _pf.TRASH(lvalsize);
    // ************************************

    _pf.ALIGN();
    _pf.LABEL(mklbl(condition));

    // ********** CONDITION NODE *********
    cdk::binary_expression_node* condnode;

    if (node->signal()) {
      condnode = new cdk::le_node(lvl, rvalue_node, node->condition());
    } else {
      condnode = new cdk::ge_node(lvl, rvalue_node, node->condition());
    }
    condnode->accept(this, lvl+2);
    delete condnode;
    _pf.JZ(mklbl(end));
    // ********* CONDITION NODE OVER *********

    node->block()->accept(this, lvl + 2);

    _pf.ALIGN();
    _pf.LABEL(mklbl(continu));


    // ******* ADD & STORE **************
    cdk::binary_expression_node* sumnode;

    if (node->signal()) {
      sumnode = new cdk::add_node(lvl, rvalue_node, node->add());
    } else {
      sumnode = new cdk::sub_node(lvl, rvalue_node, node->add());
    }
    sumnode->accept(this, lvl+2);
    delete sumnode;

    node->lvalue()->accept(this, lvl);  
    if (lvalsize == 4) {
      _pf.STORE();
    } else if (lvalsize == 8) {
      _pf.DSTORE();
    }
    // ******* ADD & STORE OVER *********

    _pf.JMP(mklbl(condition));
    _pf.ALIGN();
    _pf.LABEL(mklbl(end));

    _nextList.pop_back();
    _stopList.pop_back();
}

void xpl::postfix_writer::do_while_node(xpl::while_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int condition = ++_lbl;
  int end = ++_lbl;
  _nextList.push_back(condition);
  _stopList.push_back(end);

  _pf.ALIGN();
  _pf.LABEL(mklbl(condition));
  node->condition()->accept(this, lvl+2);
  _pf.JZ(mklbl(end));
  node->block()->accept(this, lvl+2);
  _pf.JMP(mklbl(condition));
  _pf.ALIGN();
  _pf.LABEL(mklbl(end));

  _nextList.pop_back();
  _stopList.pop_back();
}

