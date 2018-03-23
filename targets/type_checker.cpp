#include <string>
#include "targets/type_checker.h"
#include "ast/all.h"  // automatically generated

#define ASSERT_UNSPEC \
    { if (node->type() != nullptr && \
          node->type()->name() != basic_type::TYPE_UNSPEC) return; }


//------------ LITERALS -----------------------------------------------------

void xpl::type_checker::do_integer_node(cdk::integer_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(new basic_type(4, basic_type::TYPE_INT));
}

void xpl::type_checker::do_double_node(cdk::double_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(new basic_type(8, basic_type::TYPE_DOUBLE));
}

void xpl::type_checker::do_string_node(cdk::string_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(new basic_type(4, basic_type::TYPE_STRING));
}

//------------ UNARY EXPRESSIONS --------------------------------------------

/* **********************Identity and Simmetry *************************** */
inline void xpl::type_checker::processIdentSimExpression(cdk::unary_expression_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->argument()->accept(this, lvl+2);
  type argtype = node->argument()->type()->name();

  if ( (argtype != basic_type::TYPE_INT) && (argtype != basic_type::TYPE_DOUBLE) ) {
    throw std::string("Argument in identity or sim expression must be int or double.");
  }

  node->type(node->argument()->type());
}
void xpl::type_checker::do_neg_node(cdk::neg_node * const node, int lvl) {
  processIdentSimExpression(node, lvl);
}
void xpl::type_checker::do_identity_node(xpl::identity_node * const node, int lvl) {
  processIdentSimExpression(node, lvl);
}

/* **** Type of unary expression that only checks if its argument is int **** */
inline void xpl::type_checker::processUnaryIntExpression(cdk::unary_expression_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->argument()->accept(this, lvl+2);
  type argtype = node->argument()->type()->name();
  if ( argtype != basic_type::TYPE_INT ) {
    throw std::string("Argument must be integer.");
  }

  node->type(node->argument()->type());
}
void xpl::type_checker::do_not_node(cdk::not_node * const node, int lvl) {
  processUnaryIntExpression(node, lvl);
}
void xpl::type_checker::do_memalloc_node(xpl::memalloc_node * const node, int lvl) {
  ASSERT_UNSPEC;

  processUnaryIntExpression(node, lvl);

  // Precisa de saber o outro lado da atribuicao, 
  // por isso deixa o assignment node definir o tipo deste no
  node->type(new basic_type(0, basic_type::TYPE_UNSPEC));
}

void xpl::type_checker::do_address_node(xpl::address_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->argument()->accept(this, lvl+2);

  // Dynamic cast para ver se o argumento e um lvalue
  try {
    if ( &(dynamic_cast <cdk::lvalue_node&> (*node->argument())) == nullptr ) {
      throw std::string("Address's argument must be a lvalue.");
    }
  } catch (std::bad_cast e) {
    throw std::string("Address's argument must be a lvalue.");
  } 

  // O resultado de um address e sempre 4 bytes
  node->type(new basic_type(4, basic_type::TYPE_INT));
}

//------------ BINARY EXPRESSIONS -------------------------------------------

bool xpl::type_checker::handleSpecialUnspecs(cdk::binary_expression_node * const node, int lvl) {
  // First, verify if both left and right aren't memalloc
  try { 
    if ( &(dynamic_cast <xpl::memalloc_node&> (*node->left())) != nullptr ) {
      throw std::string("Fatal error: Alocating memory in the middle of a binary expression. ");
    }
  } catch (std::bad_cast e) { }
  try {
    if ( &(dynamic_cast <xpl::memalloc_node&> (*node->right())) != nullptr ) {
      throw std::string("Fatal error: Alocating memory in the middle of a binary expression. ");
    }
  } catch (std::bad_cast e) { }


  // Since the only other node that can be unspec is read, we don't need to verify
  // By default, if both are reads, they are infered to int
  type ltype = node->left()->type()->name();
  type rtype = node->right()->type()->name();
  if (ltype == basic_type::TYPE_UNSPEC && rtype == basic_type::TYPE_UNSPEC) {
    node->left()->type(new basic_type(4, basic_type::TYPE_INT));
    node->right()->type(new basic_type(4, basic_type::TYPE_INT));
    node->type(node->left()->type());
    return true;
  } else if (ltype == basic_type::TYPE_UNSPEC) {
    node->left()->type(node->right()->type());
    node->type(node->left()->type());
    return true;
  } else if (rtype == basic_type::TYPE_UNSPEC) {
    node->right()->type(node->left()->type());
    node->type(node->left()->type());  
    return true; 
  }
  return false;
}

inline void xpl::type_checker::compareTypesIntDouble(cdk::binary_expression_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl+2);
  node->right()->accept(this, lvl+2);

  type ltype = node->left()->type()->name();
  type rtype = node->right()->type()->name();

  if ( (ltype == basic_type::TYPE_INT) && (rtype == basic_type::TYPE_INT) ) {
    return;
  }
  if ( (ltype == basic_type::TYPE_INT) && (rtype == basic_type::TYPE_DOUBLE) ) {
    return;
  }
  if ( (ltype == basic_type::TYPE_DOUBLE) && (rtype == basic_type::TYPE_INT) ) {
    return;
  }
  if ( (ltype == basic_type::TYPE_DOUBLE) && (rtype == basic_type::TYPE_DOUBLE) ) {
    return;
  }

  if (handleSpecialUnspecs(node, lvl)) {
    return;
  }

  throw std::string("Types aren't int and/or double.");
}

/* *************************** Comparisons *************************** */
inline void xpl::type_checker::processComparingExpression(cdk::binary_expression_node * const node, int lvl) {
  ASSERT_UNSPEC;

  try {
    compareTypesIntDouble(node, lvl);
  } catch (std::string) {
    throw std::string("Comparison has incorrect types.");
  }

  //The result of a comparison is always 0 or 1
  node->type(new basic_type(4, basic_type::TYPE_INT));
}

void xpl::type_checker::do_lt_node(cdk::lt_node * const node, int lvl) {
  processComparingExpression(node, lvl);
}
void xpl::type_checker::do_le_node(cdk::le_node * const node, int lvl) {
  processComparingExpression(node, lvl);
}
void xpl::type_checker::do_ge_node(cdk::ge_node * const node, int lvl) {
  processComparingExpression(node, lvl);
}
void xpl::type_checker::do_gt_node(cdk::gt_node * const node, int lvl) {
  processComparingExpression(node, lvl);
}

/* *************************** Equality *************************** */
inline void xpl::type_checker::processEqualityExpression(cdk::binary_expression_node * const node, int lvl) {
  ASSERT_UNSPEC;

  try {
    processComparingExpression(node, lvl);
    return;
  } catch (std::string) {}

  type ltype = node->left()->type()->name();
  type rtype = node->right()->type()->name();

  if ( (ltype == basic_type::TYPE_STRING) && (rtype == basic_type::TYPE_STRING) ) {
    return;
  }
  if ( (ltype == basic_type::TYPE_POINTER) && (rtype == basic_type::TYPE_POINTER) ) {
    return;
  }

  throw std::string("Equality has incorrect types.");
}
void xpl::type_checker::do_ne_node(cdk::ne_node * const node, int lvl) {
  processEqualityExpression(node, lvl);
}
void xpl::type_checker::do_eq_node(cdk::eq_node * const node, int lvl) {
  processEqualityExpression(node, lvl);
}


/* *************************** Logic *************************** */
inline void xpl::type_checker::processLogicExpression(cdk::binary_expression_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl+2);
  node->right()->accept(this, lvl+2);

  handleSpecialUnspecs(node, lvl);

  if ( node->left()->type()->name() != basic_type::TYPE_INT ) {
    throw std::string("Left side must be INT.");
  }
  if ( node->right()->type()->name() != basic_type::TYPE_INT ) {
    throw std::string("Right side must be INT.");
  }

  // The result of a logic operation is always an int
  node->type(new basic_type(4, basic_type::TYPE_INT));
}

void xpl::type_checker::do_and_node(cdk::and_node * const node, int lvl) {
  processLogicExpression(node, lvl);
}
void xpl::type_checker::do_or_node(cdk::or_node * const node, int lvl) {
  processLogicExpression(node, lvl);
}


/* *************************** Sum *************************** */
inline void xpl::type_checker::processSumExpression(cdk::binary_expression_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl+2);
  node->right()->accept(this, lvl+2);

  type ltype = node->left()->type()->name();
  type rtype = node->right()->type()->name();

  // C operations
  if ( (ltype == basic_type::TYPE_INT) && (rtype == basic_type::TYPE_INT) ) {
    node->type(node->left()->type());
    return;
  }
  if ( (ltype == basic_type::TYPE_INT) && (rtype == basic_type::TYPE_DOUBLE) ) {
    node->type(node->right()->type());
    return;
  }
  if ( (ltype == basic_type::TYPE_DOUBLE) && (rtype == basic_type::TYPE_INT) ) {
    node->type(node->left()->type());
    return;
  }
  if ( (ltype == basic_type::TYPE_DOUBLE) && (rtype == basic_type::TYPE_DOUBLE) ) {
    node->type(node->left()->type());
    return;
  }

  if (handleSpecialUnspecs(node, lvl)) {
    return;
  }

  throw std::string("Sum has incorrect types - Can't sum " + printType(ltype) + " with " + printType(rtype));
}
void xpl::type_checker::do_add_node(cdk::add_node * const node, int lvl) {
  try {
    processSumExpression(node, lvl);
    return;
  } catch (std::string) { }

  type ltype = node->left()->type()->name();
  type rtype = node->right()->type()->name();

  // (i) deslocamentos - Um deve ser ponteiro e o outro inteiro
  if ( (ltype == basic_type::TYPE_POINTER) && (rtype == basic_type::TYPE_INT) ) {
    node->type(node->left()->type());
    return;
  }
  if ( (ltype == basic_type::TYPE_INT) && (rtype == basic_type::TYPE_POINTER) ) {
    node->type(node->right()->type());
    return;
  }

throw std::string("Addition has incorrect types - " + printType(ltype) + " with " + printType(rtype));

}
void xpl::type_checker::do_sub_node(cdk::sub_node * const node, int lvl) {
  try {
    processSumExpression(node, lvl);
    return;
  } catch (std::string) { }

  type ltype = node->left()->type()->name();
  type rtype = node->right()->type()->name();

    // (ii) diferenças de ponteiros - Ambos ponteiros do mesmo tipo
  if ( (ltype == basic_type::TYPE_POINTER) && (rtype == basic_type::TYPE_POINTER) ) {
    if ( getSubtype(node->left()->type()) == getSubtype(node->right()->type()) ) {
      node->type(node->left()->type());
      return;
    }
  }

  throw std::string("Subtraction has incorrect types - " + printType(ltype) + " with " + printType(rtype));
}

/* *************************** Multiplication *************************** */
inline void xpl::type_checker::processMultExpression(cdk::binary_expression_node * const node, int lvl) {
  ASSERT_UNSPEC;

  try {
    compareTypesIntDouble(node, lvl);
  } catch (std::string) {
    throw std::string("Multiplication has incorrect types.");
  }

  type ltype = node->left()->type()->name();
  type rtype = node->right()->type()->name();

  // Se um dos filhos for double, a operacao vai envolver 8 bytes
  if ( (ltype == basic_type::TYPE_DOUBLE) || (rtype == basic_type::TYPE_DOUBLE) ) {
    node->type(new basic_type(8, basic_type::TYPE_DOUBLE));
  } else {
    node->type(new basic_type(4, basic_type::TYPE_INT));
  }
}
void xpl::type_checker::do_mul_node(cdk::mul_node * const node, int lvl) {
  processMultExpression(node, lvl);
}
void xpl::type_checker::do_div_node(cdk::div_node * const node, int lvl) {
  processMultExpression(node, lvl);
}


// Only needs to check if both are int, so uses an already created function
void xpl::type_checker::do_mod_node(cdk::mod_node * const node, int lvl) {
  processLogicExpression(node, lvl);
}

//------------ EXPRESSIONS --------------------------------------------------

void xpl::type_checker::do_identifier_node(cdk::identifier_node * const node, int lvl) {
  ASSERT_UNSPEC

  const std::string &id = node->name();
  auto symbol = _symtab.find(id);

  if (symbol == nullptr) {
    throw "Symbol vazio : " + id;
  }

  node->type(symbol->type());
}

void xpl::type_checker::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_UNSPEC
  try {
    node->lvalue()->accept(this, lvl+2);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "Undeclared variable '" + id + "'";
  }
}

void xpl::type_checker::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  /**
   * O valor da expressao do lado direito do operador e guardado na posicao indicada 
   * pelo left-value  (operando esquerdo do operador). Podem ser atribuidos valores inteiros a 
   * left-values reais (conversao automatica). Nos outros casos, ambos os tipos tem de concordar.
   * 
   * (1) Verificar se ambos os tipos sao iguais
   * (2) Se nao forem, verificar se podem concordar
   * (3) Caso especial - Lidar com memalloc
   * (4) Caso especial - Lidar com read
   * (4.1) Read so pode ser int ou double
   * (5) Lidar com arrays
   * (5.1) Array de double pode receber um int
   */
  ASSERT_UNSPEC

  node->lvalue()->accept(this, lvl+2);
  node->rvalue()->accept(this, lvl+2);

  node->type(node->lvalue()->type());
  type ltype = node->lvalue()->type()->name();
  type rtype = node->rvalue()->type()->name();

  // (1) Verificar se ambos os tipos sao iguais
  if (ltype == rtype) {
    return;
  }

  // (2) Se nao forem, verificar se podem concordar
  if ( (ltype == basic_type::TYPE_DOUBLE) && (rtype == basic_type::TYPE_INT) ) {
    return;
  }

  // (3) Caso especial - Lidar com memalloc
  // Se esquerda: pointer e direita: memalloc, tem de lhe definir o tipo
  if (ltype == basic_type::TYPE_POINTER && rtype == basic_type::TYPE_UNSPEC) {
    node->rvalue()->type(new basic_type(4, basic_type::TYPE_POINTER));
    node->rvalue()->type()->_subtype = node->lvalue()->type()->subtype();
    return;
  }

  // (4) Caso especial - Lidar com read
  // Se direita: unspec e nao e memalloc, entao e read, tem de lhe definir o tipo
  if (rtype == basic_type::TYPE_UNSPEC) {
    // (4.1) Read so pode ser int ou double
    if (ltype != basic_type::TYPE_INT && ltype != basic_type::TYPE_DOUBLE) {
      throw std::string("Read can only be int or double, can't be " + printType(ltype));
    }
    node->rvalue()->type(node->lvalue()->type());
    return;
  }

  // (5) Lidar com arrays
  // Se esquerda for ponteiro, comparar subtipo com tipo da esquerda
  if ( ltype == basic_type::TYPE_POINTER ) {
    ltype = getSubtype(node->lvalue()->type());
    rtype = getSubtype(node->rvalue()->type());
    if (ltype == rtype) {
      return;
    // (5.1) Array de double pode receber um int
    } else if (ltype == basic_type::TYPE_DOUBLE && rtype == basic_type::TYPE_INT) {
      return;
    } else {
      throw std::string("Expected pointer of subtype " + printType(ltype) 
        + ", received " + printType(rtype));
    }
  }  

  throw std::string("Expected " + printType(ltype) + ", received " + printType(rtype));
}

void xpl::type_checker::do_funcall_node(xpl::funcall_node * const node, int lvl) {
  /* 
   * (1) Verificar se existe o simbolo
   * (2) Verificar se o simbolo e funcao
   * (3) Verificar se a lista de argumentos tem o mesmo tamanho
   * (4) Verificar se os tipos dos argumentos sao iguais
   * (4.1) Caso especial - read
   * (4.1.1) Read so pode ser int ou double
   * (4.2) Verificar se dois ponteiros do mesmo subtipo
   * (4.2.1) Caso especial : Ponteiro real pode receber valor de ponteiro inteiro
   */
  ASSERT_UNSPEC

  const std::string &id = *(node->name());
  auto symbol = _symtab.find(id);

  // (1) Verificar se existe o simbolo
  if (symbol == nullptr) {
    throw "Function " + id + " does not exist.";
  }

  // (2) Verificar se o simbolo e funcao
  if (symbol != nullptr && symbol->fn() == false) {
    throw id + " is not a function.";
  }

  // PREP (3) e (4) - Obter listas de fn args e call args
  int func_args;
  int call_args;
  if (symbol->getArgs().empty()) {
    func_args = 0;
  } else {
    func_args = symbol->getArgs().size();
  }

  // Adicionar tipos dos argumentos da funcall a lista de basic types
  // Usa-se dynamic cast porque pela gramatica sabe-se que sao expressions.
  std::vector<basic_type> call_types;
  if (node->argument() != nullptr) {
    for (size_t i = 0; i < node->argument()->size(); i++) {
      if (node->argument()->node(i) != nullptr) { 
        try {
          node->argument()->node(i)->accept(this, 0);
          auto *expr = dynamic_cast <cdk::expression_node*> (node->argument()->node(i));
          call_types.push_back( *(expr->type()) );
        } catch (std::bad_cast e) {
        // Sequence node is unreliable in its size, so this is ok.
        // throw std::string("Argument must be a declaration of variable.");
        }
      } 
    }
  }

  if (call_types.empty()) {
    call_args = 0;
  } else {
    call_args = call_types.size();
  }

  // (3) Verificar se a lista de argumentos tem o mesmo tamanho
  if (func_args != call_args) {
    throw "Calling function " + id + " with wrong amount of arguments (expected " +
    std::to_string(func_args) + ", calling with " + std::to_string(call_args) +")";
  }

  // (4) Verificar se os argumentos em ambos as listas sao do mesmo tipo
  type cltype;
  type fntype;
  for (size_t  i = 0; i < symbol->getArgs().size(); i++) {
    cltype = call_types[i].name();
    fntype = symbol->getArgs()[i].name();
    if (fntype == basic_type::TYPE_DOUBLE && cltype == basic_type::TYPE_INT) { // Convert callarg int to double
      ((cdk::expression_node *) node->argument()->node(i))->type(new basic_type(8, basic_type::TYPE_DOUBLE));
      continue;
    }

    // (4.1) Caso especial - read
    if (cltype == basic_type::TYPE_UNSPEC) {
      if (fntype == basic_type::TYPE_INT) {
        ((cdk::expression_node *) node->argument()->node(i))->type(new basic_type(4, basic_type::TYPE_INT));
      } else if (fntype == basic_type::TYPE_DOUBLE) {
        ((cdk::expression_node *) node->argument()->node(i))->type(new basic_type(8, basic_type::TYPE_DOUBLE));
      } else { // (4.1.1) Read so pode ser int ou double
        throw std::string("Read can only be int or double, but function expects " + printType(fntype));        
      } 
      continue;
    }

    // (4.2) Verificar se dois ponteiros do mesmo subtipo
    if (fntype == basic_type::TYPE_POINTER) { // Compare subtypes
      type lsubtype = getSubtype(&symbol->getArgs()[i]);
      type rsubtype = getSubtype(&call_types[i]);

      if (lsubtype == rsubtype) {
        continue;
      } else if (lsubtype == basic_type::TYPE_DOUBLE && rsubtype == basic_type::TYPE_INT) {
        // (4.2.1) Caso especial : Ponteiro real pode receber valor de ponteiro inteiro
        continue;
      }
    }

    if (fntype != cltype) { 
      throw "Calling function " + id + " with arg number " +  std::to_string(i+1) + " of type " 
      + printType(cltype) + ", expected " + printType(fntype);
    }
  }

  // No fica com o tipo da funcao que chama
  node->type(symbol->type());
}

void xpl::type_checker::do_index_node(xpl::index_node * const node, int lvl) {
  ASSERT_UNSPEC

  /* 
   * (1) Verificar se o indexed e um pointeiro
   * (2) Verificar se o shift e um inteiro
   */

  node->expression()->accept(this, lvl+2);
  node->shift()->accept(this, lvl+2);

  type indexed = node->expression()->type()->name();
  type shift = node->shift()->type()->name();

  // (1) Verificar se o indexed e um pointeiro
  if ( !(indexed == basic_type::TYPE_POINTER) ) {
    throw std::string("The indexed value must be a pointer, is " + printType(indexed));
  }
  // (2) Verificar se o shift e um inteiro
  if ( !(shift == basic_type::TYPE_INT) ) {
    throw std::string("The index shift must be an integer, is " + printType(shift));
  }

  // Tipo do index e igual ao subtipo do ponteiro
  node->type(node->expression()->type()->subtype());
}

void xpl::type_checker::do_read_node(xpl::read_node * const node, int lvl) {
  ASSERT_UNSPEC

  /* Precisa de saber o outro lado da atribuicao 
   * por isso deixa os seguintes nos deciderem o tipo: 
   * assignment,
   * decl var,
   * funcall,
   * print
   */

  node->type(new basic_type(0, basic_type::TYPE_UNSPEC));
}

/* *********************************************************************** */
//------------ BASIC NODES --------------------------------------------------
/* *********************************************************************** */

void xpl::type_checker::do_evaluation_node(xpl::evaluation_node * const node, int lvl) {
  node->argument()->accept(this, lvl+2);
}

void xpl::type_checker::do_function_node(xpl::function_node * const node, int lvl) {
  /* 
   * (1) Verificar se existe variavel com o mesmo nome
   * (2) Verificar se a funcao esta declarada e definida
   * (3) Verificar se o tipo da funcao e igual ao literal
   * (4) Verificar se os argumentos estao correctos (funcall faz esta verf.)
   * (4) Se funcao ja foi declarada, verificar se os argumentos sao do mesmo tipo (prevent overloading) 
   * (4.1) Se numero de argumentos e diferente
   * (4.2) Se argumentos sao de tipos diferentes
   */
  const std::string &id = *(node->name());
  auto symbol = _symtab.find(id);

  // (1) Verificar se existe variavel com o mesmo nome
  if (symbol != nullptr && symbol->fn() == false) {
    throw "Redefinition of " + id + ". Was a variable, now is a function.";
  }

  // (2) Verificar se a funcao esta declarada mas nao definida
  if (symbol != nullptr && symbol->fndef() == true) {
    throw "Function " + id + " is already defined.";
  }

  // (3) Verificar se o tipo da funcao e igual ao literal
  // A funcao pode ter literal indefinido
  // Pode ser real e ter literal inteiro (converte int para double)
  if (node->literal() != nullptr) {
    node->literal()->accept(this, lvl+2);
    type fntype = node->type()->name();
    type result = node->literal()->type()->name();
    if ( fntype == basic_type::TYPE_DOUBLE && result == basic_type::TYPE_INT) {
      node->literal()->type(new basic_type(8, basic_type::TYPE_DOUBLE)); 
      return;
    }
    if ( fntype != result ) {
      throw "Function " + id + "'s type is different from its return type.";
    }
  }

  // * (4) Se funcao ja foi declarada, verificar se os 
  // argumentos sao do mesmo tipo (prevent overloading) 
  if (symbol != nullptr) {
	// PREP (4.1) e (4.2) - Obter listas de fn args e call args
	  int func_args;
	  int def_args;
	  if (symbol->getArgs().empty()) {
	    func_args = 0;
	  } else {
	    func_args = symbol->getArgs().size();
	  }

	  // Adicionar tipos dos argumentos da def func a lista de basic types
	  // Usa-se dynamic cast porque pela gramatica sabe-se que sao expressions.
	  std::vector<basic_type> def_types;
	  if (node->argument() != nullptr) {
	    for (size_t i = 0; i < node->argument()->size(); i++) {
	      if (node->argument()->node(i) != nullptr) { 
	        try {
	          node->argument()->node(i)->accept(this, 0);
	          auto *var = dynamic_cast <xpl::decl_variable_node*> (node->argument()->node(i));
	          def_types.push_back( *(var->type()) );
	        } catch (std::bad_cast e) {
	        // Sequence node is unreliable in its size, so this is ok.
	        // throw std::string("Argument must be a declaration of variable.");
	        }
	      } 
	    }
	  }
	  if (def_types.empty()) {
	    def_args = 0;
	  } else {
	    def_args = def_types.size();
	  }

	  // (3) Verificar se a lista de argumentos tem o mesmo tamanho
	  if (func_args != def_args) {
	    throw "Declared function " + id + " with " + std::to_string(func_args) + " arguments, defining with " + std::to_string(def_args);
	  }

	  // (4) Verificar se os argumentos em ambos as listas sao do mesmo tipo
	  type cltype;
	  type fntype;
	  for (size_t  i = 0; i < symbol->getArgs().size(); i++) {
	    cltype = def_types[i].name();
	    fntype = symbol->getArgs()[i].name();
	    if (fntype == basic_type::TYPE_DOUBLE && cltype == basic_type::TYPE_INT) { // Convert defarg int to double
	      ((cdk::expression_node *) node->argument()->node(i))->type(new basic_type(8, basic_type::TYPE_DOUBLE));
	      continue;
	    }

	    // (4.2) Verificar se dois ponteiros do mesmo subtipo
	    if (fntype == basic_type::TYPE_POINTER) { // Compare subtypes
	      type lsubtype = getSubtype(&symbol->getArgs()[i]);
	      type rsubtype = getSubtype(&def_types[i]);

	      if (lsubtype == rsubtype) {
	        continue;
	      }     
	    }

	    if (fntype != cltype) { 
	      throw "Declared function " + id + " with arg number " +  std::to_string(i+1) + " of type " 
	      + printType(cltype) + ", now has type " + printType(fntype);
	    }
	  }
	}

}

void xpl::type_checker::do_print_node(xpl::print_node * const node, int lvl) {
  node->argument()->accept(this, lvl+2);

  // If argument is a read, make its type int.
  if (node->argument()->type()->name() == basic_type::TYPE_UNSPEC) {
    node->argument()->type(new basic_type(4, basic_type::TYPE_INT));
  }
}

//------------ BASIC NODES - DECLARATION ------------------------------------

void xpl::type_checker::do_decl_variable_node(xpl::decl_variable_node * const node, int lvl) {
  /* 
   * (1) Verificar se existe simbolo com o mesmo nome
   * (2) Verificar que a inicializacao e do mesmo tipo
   * (3) Caso especial - Decl com memalloc
   * (4) Caso especial - Decl com read
   * (4.1) Read so pode ser int ou double
   * Verificacao de var global iniciada com literal esta no postfix
   */
  const std::string &id = *(node->name());
  
  // (1) Verificar se existe simbolo com o mesmo nome
  if (_symtab.find_local(id) != nullptr) {
    throw id + " redeclared";
  }

  // (2) Verificar que a inicializacao e do mesmo tipo
  if (node->init() != nullptr) {
    node->init()->accept(this, lvl+2);
    type vartype = node->type()->name();
    type initype = node->init()->type()->name();

    // Se a variavel e do tipo real e recebe um inteiro, 
    // o postfix_visitor tem um metodo chamado decl_initiator 
    // que lida com isso dependendo se esta numa funcao ou nao
    if (vartype == basic_type::TYPE_DOUBLE && initype == basic_type::TYPE_INT) {
      return;
    }

    // (3) Caso especial - Decl com memalloc
    if (vartype == basic_type::TYPE_POINTER && initype == basic_type::TYPE_UNSPEC) {
      node->init()->type(new basic_type(4, basic_type::TYPE_POINTER));
      node->init()->type()->_subtype = node->type()->subtype();
      return;
    }

    // (4) Caso especial - Lidar com read
    // Se direita: unspec e nao e memalloc, entao e read, tem de lhe definir o tipo
    if (initype == basic_type::TYPE_UNSPEC) {
      // (4.1) Read so pode ser int ou double
      if (vartype != basic_type::TYPE_INT && vartype != basic_type::TYPE_DOUBLE) {
        throw std::string("Read can only be int or double, can't be " + printType(vartype));
      }
      node->init()->type(node->type());
      return;
    }

    if (vartype != initype) {
      throw "Variable " + id + " is of type " + 
        printType(vartype) + ", initializing with type " + printType(initype);
    }
  }
}

void xpl::type_checker::do_decl_function_node(xpl::decl_function_node * const node, int lvl) {
  /* 
   * (1) Verificar se existe simbolo com o mesmo nome
   */
  const std::string &id = *(node->name());

  // (1) Verificar se existe simbolo com o mesmo nome
  if (_symtab.find(id) != nullptr) {
    throw id + " redeclared";
  }
}

//------------ BASIC NODES - CONDITION --------------------------------------

void xpl::type_checker::do_if_node(xpl::if_node * const node, int lvl) {
  /* 
   * (1) Verificar condicao
   */

  node->condition()->accept(this, lvl+2);
}

void xpl::type_checker::do_if_else_node(xpl::if_else_node * const node, int lvl) {
  /* 
   * (1) Verificar condicao
   */

  node->condition()->accept(this, lvl+2);
}

//------------ BASIC NODES - ITERATION --------------------------------------

void xpl::type_checker::do_sweep_node(xpl::sweep_node * const node, int lvl) {
  /* 
   * (1) Verificar lvalue esta iniciado
   * (2) Verificar init mesmo tipo que lvalue
   * (3) Verificar condicao
   * (4) Verificar somador mesmo tipo que lvalue
   */

  // (1) Verificar lvalue esta iniciado
  node->lvalue()->accept(this, lvl+2);

  // (2) Verificar init mesmo tipo que lvalue
  node->init()->accept(this, lvl+2);
  type lefttype = node->lvalue()->type()->name();
  type initype = node->init()->type()->name();
  bool flag = true;

  if (lefttype == basic_type::TYPE_DOUBLE && initype == basic_type::TYPE_INT) {
    flag = false;
  } else if (lefttype == basic_type::TYPE_INT && initype == basic_type::TYPE_INT) {
    flag = false;
  } else if (lefttype == basic_type::TYPE_DOUBLE && initype == basic_type::TYPE_DOUBLE) {
    flag = false;
  }

  if (flag) {
    throw std::string("Lvalue is of type " + printType(lefttype) + 
    ", initializing with type " + printType(initype));
  }

  // (3) Verificar condicao
  node->condition()->accept(this, lvl+2);

  // (4) Verificar somador mesmo tipo que lvalue
  node->add()->accept(this, lvl+2);
  type addtype = node->add()->type()->name();

  if (lefttype == basic_type::TYPE_DOUBLE && addtype == basic_type::TYPE_INT) {
    node->add()->type(node->lvalue()->type());
    return;
  } else if (lefttype == basic_type::TYPE_INT && addtype == basic_type::TYPE_INT) {
    return;
  } else if (lefttype == basic_type::TYPE_DOUBLE && addtype == basic_type::TYPE_DOUBLE) {
    return;
  }

  throw std::string("Lvalue is of type " + printType(lefttype) + 
    ", adding with type " + printType(addtype));
}

void xpl::type_checker::do_while_node(xpl::while_node * const node, int lvl) {
    /* 
   * (1) Verificar condicao
   */

  node->condition()->accept(this, lvl+2);
}
