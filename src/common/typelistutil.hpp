/*
  TYPELISTUTIL.hpp  -  metaprogramming utilities for lists-of-types
 
  Copyright (C)         Lumiera.org
    2008,               Hermann Vosseler <Ichthyostega@web.de>
 
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

====================================================================
This code is heavily inspired by  
 The Loki Library (loki-lib/trunk/include/loki/Sequence.h)
    Copyright (c) 2001 by Andrei Alexandrescu
    Copyright (c) 2005 by Peter Kümmel
    This Loki code accompanies the book:
    Alexandrescu, Andrei. "Modern C++ Design: Generic Programming
        and Design Patterns Applied". 
        Copyright (c) 2001. Addison-Wesley. ISBN 0201704315
        
  Loki Copyright Notice: 
  Permission to use, copy, modify, distribute and sell this software for any
  purpose is hereby granted without fee, provided that the above copyright
  notice appear in all copies and that both that copyright notice and this
  permission notice appear in supporting documentation.
  The author makes no representations about the suitability of this software
  for any purpose. It is provided "as is" without express or implied warranty.
*/


/** @file typelistutil.hpp
 ** Helpers for working with lumiera::typelist::Types (i.e. lists-of-types). 
 ** The main purpose is to build interfaces and polymorphic implementations
 ** (using virtual functions) based on templated Types or Collections of types,
 ** which is not possible without Template Metaprogrmming.
 ** 
 ** @see lumiera::query::ConfigRules usage example
 ** @see typelist.hpp
 ** 
 */


#ifndef LUMIERA_TYPELISTUTIL_H
#define LUMIERA_TYPELISTUTIL_H

#include "common/typelist.hpp"



namespace lumiera
  {
  namespace typelist
    {
    
    /** 
     * Apply a template to a collection of types. 
     * The resulting class ends up inheriting from an instantiation
     * of the template for each of the types in the list. The iheritance
     * graph is built in a "mixin" (multiple inheritance) style. 
     */
    template
      < class TYPES                  // List of Types 
      , template<class> class _X_   //  your-template-goes-here
      , class BASE = NullType      //   Base class at end of chain
      >
    class InstantiateForEach;
    
    
    template<template<class> class _X_, class BASE>
    class InstantiateForEach<NullType, _X_, BASE>
      : public BASE
      { 
      public:
        typedef BASE     Unit;
        typedef NullType Next;
      };
    
      
    template
      < class TY, typename TYPES
      , template<class> class _X_
      , class BASE
      >
    class InstantiateForEach<Node<TY, TYPES>, _X_, BASE> 
      : public _X_<TY>,
        public InstantiateForEach<TYPES, _X_, BASE>
      { 
      public:
        typedef _X_<TY> Unit;
        typedef InstantiateForEach<TYPES,_X_> Next;
      };
    
    
    
    /** 
     * Build a single inheritance chain of template instantiations.
     * Needs the help of the user provided Template, which now has
     * to take a second parameter and use this as Base class. 
     * The resulting class ends up (single) inheriting from an 
     * instantiation of the templace for each of the types, while
     * overrideing/implementing the provided base class.
     */
    template
      < class TYPES                      // List of Types 
      , template<class,class> class _X_ //  your-template-goes-here
      , class BASE = NullType          //   Base class at end of chain
      >
    class InstantiateChained;
    
    
    template<template<class,class> class _X_, class BASE>
    class InstantiateChained<NullType, _X_, BASE>
      : public BASE
      { 
      public:
        typedef BASE     Unit;
        typedef NullType Next;
      };
    
      
    template
      < class TY, typename TYPES
      , template<class,class> class _X_
      , class BASE
      >
    class InstantiateChained<Node<TY, TYPES>, _X_, BASE> 
      : public _X_< TY
                  , InstantiateChained<TYPES, _X_, BASE>
                  >
      { 
      public:
        typedef InstantiateChained<TYPES,_X_,BASE> Next;
        typedef _X_<TY,Next> Unit;
      };
    
    
    
    /** 
     * A Variation of InstantiateChained providing an incremented
     * Index value template parameter. This index can e.g. be used
     * to store pointers in a dispatcher table in the Base class.
     * Similar to InstantiateChained, this template builds a linear
     * chain of inheritance. The user-provided template, which is
     * to be instantiated for all types in the Typelist, now has to
     * accept an additional third parameter (uint i).
     */
    template
      < class TYPES                           // List of Types
      , template<class,class,uint> class _X_ //  your-template-goes-here
      , class BASE = NullType               //   Base class at end of chain
      , uint i = 0                         //    incremented on each instantiaton
      >
    class InstantiateWithIndex;
    
    
    template< template<class,class,uint> class _X_
            , class BASE
            , uint i
            >
    class InstantiateWithIndex<NullType, _X_, BASE, i>
      : public BASE
      { 
      public:
        typedef BASE     Unit;
        typedef NullType Next;
        enum{ COUNT = i };
      };
    
    
    template
      < class TY, typename TYPES
      , template<class,class,uint> class _X_
      , class BASE
      , uint i
      >
    class InstantiateWithIndex<Node<TY, TYPES>, _X_, BASE, i> 
      : public _X_< TY
                  , InstantiateWithIndex<TYPES, _X_, BASE, i+1 >
                  , i
                  >
      { 
      public:
        typedef InstantiateWithIndex<TYPES,_X_,BASE,i+1> Next;
        typedef _X_<TY,Next,i> Unit;
        enum{ COUNT = Next::COUNT };
      };
    
    
    /**
     * Metafunction counting the number of Types in the collection
     */
    template<class TYPES>
    struct count;
    template<>
    struct count<NullType>
      {
        enum{ value = 0 };
      };
    template<class TY, class TYPES>
    struct count<Node<TY,TYPES> >
      {
        enum{ value = 1 + count<TYPES>::value };
      };
    
    /**
     * Metafunction " max( sizeof(T) ) for T in TYPES "
     */
    template<class TYPES>
    struct maxSize;
    template<>
    struct maxSize<NullType>
      {
        enum{ value = 0 };
      };
    template<class TY, class TYPES>
    struct maxSize<Node<TY,TYPES> >
      {
        enum{ thisval = sizeof(TY)
            , nextval = maxSize<TYPES>::value
            , value   = nextval > thisval?  nextval:thisval
            };
      };
    
    
  } // namespace typelist

} // namespace lumiera
#endif
