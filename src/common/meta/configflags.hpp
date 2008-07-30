/*
  CONFIGFLAGS.hpp  -  Building classes based on configuration cases
 
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


/** @file configflags.hpp
 ** Under some circumstances it is necessary to assemble functionality
 ** out of elementary building blocks. Especially, this assembly can be
 ** expressed as template specialisations directed by a configuration type.
 ** Thus, the purpose of this header is to encode a flag-like configuraiton
 ** as distinct types, which can be used to select such specialisations. 
 ** Each possible configuration can be encoded as a list of flags, which allows
 ** to generate, filter and process those configurations. The final goal is to
 ** automatically generate a factory which will deliver objects configured
 ** according to the configuration in question. 
 ** 
 ** @see proc::engine::config::Strategy usage example
 ** @see proc::engine::config::ConfigSelector 
 ** @see typelist.hpp
 ** 
 */


#ifndef LUMIERA_META_CONFIGFLAGS_H
#define LUMIERA_META_CONFIGFLAGS_H


#include "common/meta/typelist.hpp"


namespace lumiera {
  namespace typelist {
    
    
    template<char bit> struct Flag    { typedef Flag     ID; };
    template<>         struct Flag<0> { typedef NullType ID; };
    
    
    template< char f1=0
            , char f2=0
            , char f3=0
            , char f4=0
            , char f5=0
            >
    struct Flags        
      { 
        typedef typename Types< typename Flag<f1>::ID
                              , typename Flag<f2>::ID
                              , typename Flag<f3>::ID
                              , typename Flag<f4>::ID
                              , typename Flag<f5>::ID
                              >::List       
                              Tuple;
        typedef Tuple List;
      };
    
    
    template< char f1=0
            , char f2=0
            , char f3=0
            , char f4=0
            , char f5=0
            >
    struct Config      ///< distinct type representing a configuration
      { 
        typedef typename Flags<f1,f2,f3,f4,f5>::Tuple  Flags;
        typedef Flags List;
      };
    
    
    
    template<char Fl, class CONF> 
    struct ConfigSetFlag;       ///< set (prepend) the Flag to the given config
    
    template< char Fl
            , char f1
            , char f2
            , char f3
            , char f4
            , char IGN
            >
    struct ConfigSetFlag<Fl, Config<f1,f2,f3,f4,IGN> >
      {
        typedef Config<Fl,f1,f2,f3,f4> Config;
      };
    
    
    
    /** build a configuration type for the given list-of-flags */
    template<class FLAGS, class CONF=Config<> >
    struct DefineConfigByFlags
      {
        typedef CONF Config;
        typedef Config Type;
      };
    template<char Fl, class FLAGS, class CONF>
    struct DefineConfigByFlags< Node<Flag<Fl>,FLAGS>, CONF>
      { 
        typedef typename ConfigSetFlag< Fl
                                      , typename DefineConfigByFlags<FLAGS,CONF>::Config
                                      >::Config Config;
        typedef Config Type;
      };
    
    
    /** 
     * Helper for calculating values and for
     * invoking runtime code based on a given FlagTuple.
     * Can also be used on a Typelist of several Configs.
     * The latter case is typically used to invoke an operation
     * while ennumerating all Flag-Configurations defined in Code.
     * An example would be to build (at runtime) an dispatcher table.
     * Explanation: For the Case covering a List of Configs, we provide
     * a templated visitaion function, which can accept a functor object
     * to be invoked on each Configuration. 
     */
    template<class FLAGS>
    struct FlagInfo;
    
    template<char ff, class FLAGS>
    struct FlagInfo<Node<Flag<ff>, FLAGS> >
      {
        enum{ BITS = MAX(ff,   FlagInfo<FLAGS>::BITS)
            , CODE = (1<<ff) | FlagInfo<FLAGS>::CODE
            }; 
      };
    template<>
    struct FlagInfo<NullType>
      {
        enum{ BITS = 0
            , CODE = 0
            };
        
        template<class FUNC>
        static typename FUNC::Ret
        accept (FUNC& functor)
          {
            return functor.done();
          }
      };
    template<class CONF, class TAIL>
    struct FlagInfo<Node<CONF, TAIL> >
      {
        typedef typename CONF::Flags ThisFlags;
        enum{ BITS = MAX (char(FlagInfo<ThisFlags>::BITS), char(FlagInfo<TAIL>::BITS))
            };
        
        template<class FUNC>
        static typename FUNC::Ret
        accept (FUNC& functor)
          {
            functor.template visit<CONF>(FlagInfo<ThisFlags>::CODE);
            return FlagInfo<TAIL>::accept (functor);
          }
      };
    
    
    
  } // namespace typelist
  
} // namespace lumiera
#endif
