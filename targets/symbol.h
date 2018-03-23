#ifndef __XPL_SEMANTICS_SYMBOL_H__
#define __XPL_SEMANTICS_SYMBOL_H__

#include <string>
#include <vector>
#include <cdk/basic_type.h>

namespace xpl {

    class symbol {
      bool _toimport;     // Se e para importar
      bool _local;        // Se e var global ou local (usado em postfix identifier node)
      bool _fn;           // Se e funcao
      bool _fndef;        // Se e funcao, se ja ta definida
      basic_type *_type;  // Tipo do identifier
      std::string _name;  // Nome do identifier
      long _value;        // Valor do offset
      std::vector<basic_type> _arglist; // Argumentos da funcao (caso seja)

    public:
      inline symbol(
      	bool toimport,
        bool local,  
      	bool fn,      
      	bool fndef, 
      	basic_type *type, 
      	const std::string &name, 
      	long value) :

          _toimport(toimport),
          _local(local),
      	  _fn(fn),
       	  _fndef(fndef),
          _type(type), 
          _name(name), 
          _value(value) { 
      }

      virtual ~symbol() {
        delete _type;
      }
      inline bool toImport() {
        return _toimport;
      }
      inline bool local() {
        return _local;
      }
      inline bool fn() {
        return _fn;
      }
      inline bool fndef() {
        return _fndef;
      }
      inline void fndef(bool b) {
        _fndef = b;
      }

      inline basic_type *type() const {
        return _type;
      }
      inline const std::string &name() const {
        return _name;
      }
      inline long value() const {
        return _value;
      }
      inline long value(long v) {
        return _value = v;
      }
      inline void addArg(basic_type type) {
        _arglist.push_back(type);
      }
      inline std::vector<basic_type> getArgs() {
        return _arglist;
      }
    };

} // xpl

#endif
