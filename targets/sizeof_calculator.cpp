#include <string>
#include "targets/sizeof_calculator.h"
#include "ast/all.h"  // automatically generated

//---------------------------------------------------------------------------

void xpl::sizeof_calculator::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    if (node->node(i) != nullptr) {
      node->node(i)->accept(this, lvl + 2);
    }
  }
}

void xpl::sizeof_calculator::do_integer_node(cdk::integer_node * const node, int lvl) {
  _size += 4;
}

void xpl::sizeof_calculator::do_double_node(cdk::double_node * const node, int lvl) {
  _size += 8;
}

void xpl::sizeof_calculator::do_string_node(cdk::string_node * const node, int lvl) {
  _size += 4;
}

void xpl::sizeof_calculator::do_decl_variable_node(xpl::decl_variable_node * const node, int lvl) {
	_size += (node->type()->name() == basic_type::TYPE_DOUBLE) ? 8 : 4;
}

void xpl::sizeof_calculator::do_function_node(xpl::function_node * const node, int lvl) {
  _size += node->type()->size();
	node->body()->accept(this, lvl);
}

void xpl::sizeof_calculator::do_body_node(xpl::body_node * const node, int lvl) {
	node->declarations()->accept(this, lvl);
  node->instructions()->accept(this, lvl);
}

void xpl::sizeof_calculator::do_block_node(xpl::block_node * const node, int lvl) {
	node->declarations()->accept(this, lvl);
  node->instructions()->accept(this, lvl);
}

void xpl::sizeof_calculator::do_if_node(xpl::if_node * const node, int lvl) {
  node->block()->accept(this, lvl);
}

void xpl::sizeof_calculator::do_if_else_node(xpl::if_else_node * const node, int lvl) {
  node->thenblock()->accept(this, lvl);
  node->elseblock()->accept(this, lvl);
}

void xpl::sizeof_calculator::do_while_node(xpl::while_node * const node, int lvl) {
  node->block()->accept(this, lvl);
}

void xpl::sizeof_calculator::do_sweep_node(xpl::sweep_node * const node, int lvl) {
  node->block()->accept(this, lvl);
}
