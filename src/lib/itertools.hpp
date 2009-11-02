/*
  ITERTOOLS.hpp  -  collection of tools for building and combining iterators 
 
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

/** @file itertools.hpp
 ** Helpers for working with iterators based on the pipeline model.
 ** Iterators abstract from the underlying data container and provide
 ** the contained data as a source to pull values from. Based on this
 ** model, we can build pipelines, with filters, valves, junction points
 ** and transforming facilities. The templates in this header enable such,
 ** based on the Lumiera Forward Iterator concept. They build on generic
 ** programming techniques, thus are intended to be combined at compile time
 ** using definitive type information. Contrast this to an iterator model
 ** as in Java's Commons-Collections, where Iterator is an Interface based
 ** on virtual functions. Thus, the basic problem to overcome is the lack
 ** of a single common interface, which could serve as foundation for
 ** type inference. As a solution, we use a "onion" approach, where a
 ** generic base gets configured with an active core, implementing
 ** the filtering or processing functionality, while the base class
 ** (IterTool) exposes the operations necessary to comply to the
 ** Forward Iterator Concept. 
 ** 
 ** \par Filtering Iterator
 ** The FilterIter template can be used to build a filter into a pipeline,
 ** as it forwards only those elements from its source iterator, which pass
 ** the predicate evaluation. Anything acceptable as ctor parameter for a
 ** tr1::function object can be passed in as predicate, but of course the
 ** signature must be sensible. Please note, that -- depending on the
 ** predicate -- already the ctor or even a simple \c bool test might
 ** pull and exhaust the source iterator completely, in an attempt
 ** to find the first element passing the predicate test. 
 **  
 ** @todo WIP WIP WIP
 ** @todo see Ticket #347
 ** 
 ** @see IterAdapter
 ** @see itertools-test.cpp
 ** @see contents-query.hpp
 */


#ifndef LIB_ITERTOOLS_H
#define LIB_ITERTOOLS_H


#include "lib/bool-checkable.hpp"
#include "lib/iter-adapter.hpp"
#include "lib/util.hpp"

#include <tr1/functional>



namespace lib {
  
  using std::tr1::function;
  using util::unConst;
  
  
  
  
  /** 
   * A neutral \em identity-function core,
   * also serving as point-of reference how any
   * core is intended to work. Any core is intended
   * to serve as inner part of an iterator tool template.
   * - it provides the trait typedefs
   * - it abstracts the "source"
   * - it abstracts the local operation to be performed
   * - the ctor of the core sets up the configuration.
   * @note cores should be copyable without much overhead
   */
  template<class IT>
  struct IdentityCore
    {
      mutable IT source_;
      
      IdentityCore (IT const& orig)
        : source_(orig)
        { }
      
      IT&
      source ()
        {
          return source_;
        }
      
      IT const&
      source ()  const
        {
          return source_;
        }
      
      bool
      evaluate () const
        {
          return bool(source_);
        }
      
      typedef typename IT::pointer pointer;
      typedef typename IT::reference reference;
      typedef typename IT::value_type value_type;
    };
  
  
  /**
   * Standard functionality to build up any iterator tool.
   * IterTool exposes the frontend functions necessary to
   * comply to the Lumiera Forward Iterator concept.
   * The protected part provides the building blocks
   * to implement the actual processing/filter logic.
   */
  template<class CORE>
  class IterTool
    : public lib::BoolCheckable<IterTool<CORE> >
    {
      
    protected: /* iteration control */
      CORE core_;
      
      bool
      hasData()  const
        {
          return core_.evaluate()
              || unConst(this)->iterate();
        }        // skipping irrelevant results doesn't count as "mutation"
      
      bool
      iterate ()
        {
          if (!core_.source()) return false;
          
          do ++core_.source();
          while (core_.source() && !core_.evaluate());
          return core_.source();
        }
      
      void
      _maybe_throw()  const
        {
          if (!isValid())
            _throwIterExhausted();
        }
      
      
      
    public:
      typedef typename CORE::pointer pointer;
      typedef typename CORE::reference reference;
      typedef typename CORE::value_type value_type;
      
      
      IterTool (CORE const& setup)
        : core_(setup)
        {
          hasData();
        }
      
      
      
      /* === lumiera forward iterator concept === */
      
      reference
      operator*() const
        {
          _maybe_throw();
          return *core_.source();
        }
      
      pointer
      operator->() const
        {
          _maybe_throw();
          return core_.source();
        }
      
      IterTool&
      operator++()
        {
          _maybe_throw();
          iterate();
          return *this;
        }
      
      bool
      isValid ()  const
        {
          return hasData();
        }
      
      bool
      empty ()    const
        {
          return !isValid();
        }
      
      
      /// comparison is allowed to access the source iterator
      template<class CX>
      friend bool operator== (IterTool<CX> const& it1, IterTool<CX> const& it2);
    };
  
  
  template<class CX>
  inline bool
  operator== (IterTool<CX> const& it1, IterTool<CX> const& it2)
  {
    return it1.isValid()      == it2.isValid()
        && it1.core_.source() == it2.core_.source()
        ;
  }
  
  template<class CX>
  inline bool
  operator!= (IterTool<CX> const& ito1, IterTool<CX> const& ito2)
  {
    return !(ito1 == ito2);
  }
  
  
  
  
  
  
  
  
  /**
   * Implementation of the filter logic.
   * This core stores a function object instance,
   * passing each pulled source element to this
   * predicate function for evaluation.
   */
  template<class IT>
  struct FilterCore
    : IdentityCore<IT>
    {
      typedef IdentityCore<IT> _Par;
      typedef typename IT::reference Val;
      
      
      function<bool(Val)> predicate_;
      
      bool
      evaluate () const
        {
          return _Par::source()
              && predicate_(*_Par::source());
        }
      
      
      template<typename PRED>
      FilterCore (IT const& orig, PRED prediDef)
        : _Par(orig)
        , predicate_(prediDef) // induces a signature check
        { }
    };
  
  
  /**
   * Iterator tool filtering pulled data according to a predicate 
   */
  template<class IT>
  class FilterIter
    : public IterTool<FilterCore<IT> >
    {
      typedef FilterCore<IT> _Filter;
      typedef IterTool<_Filter> _Impl;
      
      static bool acceptAll(typename _Filter::Val) { return true; }
      
      
    public:
      FilterIter ()
        : _Impl(FilterCore<IT>(IT(), acceptAll))
        { }
      
      template<typename PRED>
      FilterIter (IT const& src, PRED filterPredicate)
        : _Impl(_Filter(src,filterPredicate))
        { }
      
    };
  
  
  /** Build a FilterIter: convenience free function shortcut,
   *  picking up the involved types automatically.
   *  @param  filterPredicate to be invoked for each source element
   *  @return Iterator filtering contents of the source
   */
  template<class IT, typename PRED>
  inline FilterIter<IT>
  filterIterator (IT const& src, PRED filterPredicate)
  {
    return FilterIter<IT>(src,filterPredicate);
  }
  
  
  
  
  
} // namespace lib
#endif
