/*
  COMMAND-ARGUMENT-HOLDER.hpp  -  specifically typed container for storage of command arguments
 
  Copyright (C)         Lumiera.org
    2009,               Hermann Vosseler <Ichthyostega@web.de>
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
*/


/** @file command-argument-holder.hpp
 ** A simple container record holding the actual command arguments. 
 ** While all command objects themselves have a common type (type erasure),
 ** the actual argument tuple and the state memento for undo can't. Especially,
 ** the size of arguments and memento will depend on their respective types.
 ** Thus, to manage somehow the storage of this data, we create a common holder,
 ** to be managed by a custom allocator.
 ** 
 ** @todo doing just plain heap allocation for now :-P
 **  
 ** @see Command
 ** @see UndoMutation
 ** @see MementoTie
 ** @see command-argument-test.cpp
 **
 */



#ifndef CONTROL_COMMAND_ARGUMENT_HOLDER_H
#define CONTROL_COMMAND_ARGUMENT_HOLDER_H

//#include "pre.hpp"
//#include "lib/error.hpp"
#include "proc/control/command-closure.hpp"
#include "proc/control/memento-tie.hpp"

//#include <tr1/memory>
//#include <boost/scoped_ptr.hpp>
//#include <tr1/functional>
#include <iostream>
#include <string>



namespace control {
  
//  using lumiera::Symbol;
//  using std::tr1::shared_ptr;
//  using boost::scoped_ptr;
//  using std::tr1::function;
//  using std::ostream;
  using std::string;
  
  namespace { // empty state marker objects for ArgumentHolder
    
    template<typename SIG>
    struct MissingArguments
      : Closure<SIG>
      {
        typedef typename Closure<SIG>::ArgTuple ArgTuple;
        
        MissingArguments ()
          : Closure<SIG> (ArgTuple ())
          { }
        
      private:
        virtual bool isValid ()  const { return false; }
      };
    
    
    template<typename SIG, typename MEM>
    struct UntiedMemento
      : MementoTie<SIG,MEM>
      {
        typedef typename CommandSignature<SIG,MEM>::CaptureSig SIG_cap;
        typedef typename CommandSignature<SIG,MEM>::UndoOp_Sig SIG_undo;
    
        UntiedMemento()
          : MementoTie<SIG,MEM> (function<SIG_undo>(), function<SIG_cap>() )
          { }
      };
  
  } // (END) impl details / empty state marker objects
  
  
  
  
  /**
   * Specifically typed CmdClosure, which serves for 
   * actually allocating storage to hold the command arguments
   * and the undo state (memento) for Proc-Layer commands.
   * Both the contained components within ArgumentHolder 
   * can be in \em empty state; there is no distinct
   * lifecycle limitations. ArgumentHolder is part
   * of Proc-Layer command's implementation
   * and should not be used standalone. 
   */
  template<typename SIG, typename MEM>
  class ArgumentHolder
    : public CmdClosure
    {
      Closure<SIG> arguments_;
      MementoTie<SIG,MEM> memento_;
      
      typedef typename Closure<SIG>::ArgTuple ArgTuple;
      
      
      /* === proxied CmdClosure interface === */
      
      virtual bool isValid ()  const
        {
          return bool(arguments_);
        }
      
      
      virtual CmdFunctor bindArguments (CmdFunctor& func)
        {
          if (!arguments_)
            throw lumiera::error::State ("Lifecycle error: can't bind functor, "
                                         "command arguments not yet provided",
                                         LUMIERA_ERROR_UNBOUND_ARGUMENTS);
          
          return arguments_.bindArguments(func);
        }
      
      
      virtual operator string()  const
        {
          return "Command-State{ arguments="
               + arguments_? arguments_ : "unbound"
               + memento_  ? ", <memento> }" : "<no undo> }"
               ;
        }
      
      
    public:
      /** per default, all data within ArgumentHolder
       *  is set up in \em empty state. Later on, the
       *  command arguments are to be provided by #bind ,
       *  whereas the undo functions will be wired by #tie
       */
      ArgumentHolder ()
        : arguments_(MissingArguments<SIG>() )
        , memento_(UntiedMemento<SIG,MEM>() )
        { }
      
      /** has undo state capturing been invoked? */
      bool canUndo () { return bool(memento_); }
      bool empty ()   { return !arguments_; }
      
      
      /** store a new argument tuple within this ArgumentHolder,
       *  discarding and previously stored arguments */
      void
      bind (ArgTuple argTup)
        {
          this->arguments_ = Closure<SIG> (argTup);
        }
      
      
      typedef typename CommandSignature<SIG,MEM>::CaptureSig SIG_cap;
      typedef typename CommandSignature<SIG,MEM>::UndoOp_Sig SIG_undo;
      
      /** create a new memento storage wiring, discarding existing memento state.
       *  @note any bound undo/capture functions based on the previously held MementoTie
       *        are silently invalidated; using them will likely cause memory corruption! */
      MementoTie<SIG,MEM>&
      tie (function<SIG_undo> const& undoFunc,
           function<SIG_cap> const& captureFunc)
        {
          return this->memento_ = MementoTie<SIG,MEM> (undoFunc,captureFunc);
        }
      
    };
  
    
  
  
} // namespace control
#endif
