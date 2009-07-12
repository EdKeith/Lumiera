/*
  ARGUMENT-TUPLE-ACCEPT.hpp  -  helper template providing a bind(...) member function
 
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


/** @file argument-tuple-accept.hpp
 ** This template allows to mix in a \c bind(...) function.
 ** Thereby, the correct number and types of arguments is derived
 ** according to the function signature given as template parameter.
 ** This helper template is used for the ArgumentHolder and generally
 ** for binding arguments for Proc-Layer commands.
 ** 
 ** @see CommandDef
 ** @see ArgumentHolder
 ** @see argument-tuple-accept-test.cpp
 **
 */



#ifndef CONTROL_ARGUMENT_TUPLE_ACCEPT_H
#define CONTROL_ARGUMENT_TUPLE_ACCEPT_H

//#include "pre.hpp"
//#include "lib/error.hpp"
#include "lib/meta/typelist.hpp"
#include "lib/meta/typelist-util.hpp"
#include "lib/meta/function.hpp"
#include "lib/meta/tuple.hpp"

//#include <tr1/memory>
//#include <boost/scoped_ptr.hpp>
//#include <tr1/functional>
//#include <iostream>
//#include <string>



namespace control {
  
//  using lumiera::Symbol;
//  using std::tr1::shared_ptr;
//  using boost::scoped_ptr;
//  using std::tr1::function;
//  using std::ostream;
//  using std::string;
  
  namespace bind_arg {
    
    using namespace lumiera::typelist;
    
    template< class TAR, class BA
            , typename TYPES
            >
    struct AcceptArgs ;
    
    
    /* specialisations for 0...9 Arguments.... */
    
    template< class TAR, class BA
            >                                                                        //____________________________________
    struct AcceptArgs<TAR,BA, Types<> >                                             ///< Accept dummy binding (0 Arguments)
      : BA
      {
        void
        bind ()
          {
            static_cast<TAR*> (this) -> bind (tuple::makeNullTuple() );
          }
      };
    
    
    template< class TAR, class BA
            , typename T1
            >                                                                        //_______________________________
    struct AcceptArgs<TAR,BA, Types<T1> >                                           ///< Accept binding for 1 Argument
      : BA
      {
        void
        bind (T1 a1)
          {
            static_cast<TAR*> (this) -> bind (tuple::make (a1));
          }
      };
    
    
    template< class TAR, class BA
            , typename T1
            , typename T2
            >                                                                        //________________________________
    struct AcceptArgs<TAR,BA, Types<T1,T2> >                                        ///< Accept binding for 2 Arguments
      : BA
      {
        void
        bind (T1 a1, T2 a2)
          {
            static_cast<TAR*> (this) -> bind (tuple::make (a1,a2));
          }
      };
    
    
    template< class TAR, class BA
            , typename T1
            , typename T2
            , typename T3
            >                                                                        //________________________________
    struct AcceptArgs<TAR,BA, Types<T1,T2,T3> >                                     ///< Accept binding for 3 Arguments
      : BA
      {
        void
        bind (T1 a1, T2 a2, T3 a3)
          {
            static_cast<TAR*> (this) -> bind (tuple::make (a1,a2,a3));
          }
      };
    
    
    template< class TAR, class BA
            , typename T1
            , typename T2
            , typename T3
            , typename T4
            >                                                                        //________________________________
    struct AcceptArgs<TAR,BA, Types<T1,T2,T3,T4> >                                  ///< Accept binding for 4 Arguments
      : BA
      {
        void
        bind (T1 a1, T2 a2, T3 a3, T4 a4)
          {
            static_cast<TAR*> (this) -> bind (tuple::make (a1,a2,a3,a4));
          }
      };
    
    
    
    template<typename SIG>
    struct _Type
      {
        typedef typename FunctionSignature< function<SIG> >::Args Args;
        enum { ARG_CNT = count<typename Args::List>::value };
        typedef Tuple<Args> ArgTuple;
      };
  
  } // (END) impl details
  
  
  
  
  /** Helper: mix in a \c bind(...) function
   *  @param SIG function signature to mimic (regarding the arguments; return type will be void)
   *  @param TAR the target class providing a function \c bind(Tuple<Types<T1...> >)
   *  @param BASE the base class for inheritance chaining
   */
  template<typename SIG, class TAR, class BASE>
  class ArgumentTupleAccept
    : bind_arg::AcceptArgs<TAR,BASE, typename _Type<SIG>::Args>
    {
    };
  
    
  
  
} // namespace control
#endif
