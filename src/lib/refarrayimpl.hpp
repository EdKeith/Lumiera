/*
  REFARRAYIMPL.hpp  -  some implementations of the ref-array interface
 
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
 
*/

/** @file refarrayimpl.hpp
 ** Some (library-) implementations of the RefArray interface.
 **
 ** Being an array-like object exposing just a const ref, it is typically used
 ** on interfaces, and the type of the array "elements" usually is a ABC or interface.
 ** The actual implementation usually holds a subclass, and is either based on a vector,
 ** or a fixed storage contained within the implementation. The only price to pay is
 ** that of an virtual call on element access.
 ** 
 ** For advanced use it would be possible to have a pointer-array or even an embedded
 ** storage of variant-records, able to hold a mixture of subclasses. (the latter cases
 ** will be implemented when needed).
 **
 ** @see refarraytest.cpp
 ** 
 */


#ifndef LIB_REFARRAYIMPL_H
#define LIB_REFARRAYIMPL_H


#include "lib/refarray.hpp"
#include "proc/nobugcfg.hpp"

#include <vector>
using std::vector;



namespace lib {
  
  /**
   * Wrap a vector holding objects of a subtype and
   * provide array-like access using the interface type.
   */
  template<class B, class IM = B>
  class RefArrayVectorWrapper
    : public RefArray<B>
    {
      typedef vector<IM> const& Tab;
      Tab table_;
      
    public:
      
      RefArrayVectorWrapper (Tab toWrap)
        : table_(toWrap)
        { }
      
      virtual size_t size()  const
        {
          return table_.size();
        }
      
      virtual B const& operator[] (size_t i)  const 
        {
          REQUIRE (i < size());
          return table_[i];
        }
    };
  
  
  /**
   * This variation of the wrapper actually
   *  \em is a vector, but can act as a RefArray
   */
  template<class B, class IM = B>
  class RefArrayVector
    : public vector<IM>,
      public RefArrayVectorWrapper<B,IM>
    {
      typedef RefArrayVectorWrapper<B,IM> Wrap;
      typedef vector<IM>                 Vect;
      typedef typename Vect::size_type Size_t;
      typedef typename Vect::value_type Val_t;
      
    public:
      RefArrayVector()                                   : Vect(),    Wrap((Vect&)*this) {}
      RefArrayVector(Size_t n, Val_t const& v = Val_t()) : Vect(n,v), Wrap((Vect&)*this) {}
      RefArrayVector(Vect const& ref)                    : Vect(ref), Wrap((Vect&)*this) {}
      
      using Vect::size;
      using Wrap::operator[];
    };
  
  
  
  /**
   * RefArray implementation based on a fix sized array,
   * i.e. the storage is embedded. Embedded subclass obj
   * either need to be default constructible or be
   * placed directly by a factory
   */
  template<class B, size_t n, class IM = B>
  class RefArrayTable
    : public RefArray<B>
    {
      char storage_[n*sizeof(IM)];
      IM* array_;
      
    public:
      RefArrayTable() ///< objects created in-place by default ctor
        : array_ (reinterpret_cast<IM*> (&storage_))
        {
          size_t i=0;
          try
            {
              while (i<n)
                new(&array_[i++]) IM();
            }
          catch(...) { cleanup(i); throw; }
        }
      
      template<class FAC>
      RefArrayTable(FAC& factory) ///< objects created in-place by factory
        : array_ (reinterpret_cast<IM*> (&storage_))
        {
          size_t i=0;
          try
            {
              while (i<n)
                factory(&array_[i++]);
            }
          catch(...) { cleanup(i); throw; }
        }
      
      ~RefArrayTable() { cleanup(); }
      
    private:
      void cleanup(size_t top=n) throw()
        {
          while (top) array_[--top].~IM();
        }
      
      
    public: //-----RefArray-Interface------------
      
      virtual size_t size()  const
        { 
          return n; 
        }
      
      virtual B const& operator[] (size_t i)  const 
        {
          REQUIRE (i < size());
          return array_[i];
        }

    };
  
} // namespace lib
#endif
