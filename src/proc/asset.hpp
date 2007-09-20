/*
  ASSET.hpp  -  Superinterface: bookeeping view of "things" present in the session
 
  Copyright (C)         CinelerraCV
    2007,               Christian Thaeter <ct@pipapo.org>
 
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

/** @file asset.hpp
 ** Proc-Layer Interface: Assets.
 ** Declares the Asset top level Interface, the Asset::Ident identification tuple
 ** and the asset::ID primary key wrapper. Normally, Assets should be handled
 ** using asset::PAsset, a ref counting smart pointer.
 ** 
 ** These classes are placed into namespace asset and proc_interface.
 **
 ** Assets are handled by a hierarchy of interfaces. Below the top level Asset interface
 ** there are interfaces for various different <i>Kinds</i> of Assets, like asset::Media,
 ** asset::Proc, etc. Code utilizing the specific properties of e.g. Media assets, will
 ** be implemented directly against the asset::Media interface. To make this feasible 
 ** while at the same time being able to handle all asset Kinds in a uniform manner, 
 ** we use a hierarchy of ID classes. These IDs are actually just thin wrappers around 
 ** a hash value, but they carry a template parameter specifying the Asset Kind and the
 ** Asset Kind subinterfaces provide a overloaded getID method with a covariant return
 ** value. For example the asset::Media#getID returns an ID<Media>. By using the
 ** templated query function AssetManager#getAsset, we can get at references to the more 
 ** specific subinterface asset::media just by using the ID value in a typesafe manner.
 ** This helps avoiding dynamic typing and switch-on-type, leading to much more robust,
 ** extensible and clear code.
 **
 ** (Implementation detail: as g++ is not able to handle member function template
 ** instantiations completely automatic, we need to trigger some template instantiations
 ** at the end of assetmanager.cpp )
 **
 ** @see assetmanager.hpp
 ** @see media.hpp
 */


#ifndef PROC_INTERFACE_ASSET_H
#define PROC_INTERFACE_ASSET_H


#include "proc/asset/category.hpp"
#include "common/error.hpp"

#include <cstddef>
#include <string>
#include <vector>
#include <set>
#include <tr1/memory>
#include <boost/utility.hpp>


using std::string;
using std::vector;
using std::set;



namespace asset
  {
  
  using std::size_t;
  using std::tr1::shared_ptr;
  
  
  /** 
   * thin wrapper around a size_t hash ID 
   * used as primary key for all Asset objects.
   * The Templace parameter is intended for tagging
   * the ID with type information, causing the
   * compiler to select specialized behaviour 
   * for the different kinds of Assets.
   * @see Asset
   * @see AssetManager#getID generating ID values
   * @see asset::Media
   * @see ID<asset::Media>
   */
  template<class KIND>
  class ID
    {
    public:
      const size_t hash;
      ID (size_t id)         : hash(id)         {}
      ID (const KIND& asset) : hash(asset.getID()) {}
      operator size_t() const  { return hash; }
    };
    
  class Asset;
  class AssetManager;
  typedef const ID<Asset>& IDA;

  
  
  
  
  /**
   * Superinterface describing especially bookeeping properties.
   * As of 09/2007, there are four  <b>Kinds</b> of Assets, each
   * comprising a sub-Interface of the Asset Interface:
   * <ul><li>asset::Media representing mediafiles</li>
   *     <li>asset::Proc representing media data processing components (e.g. Effects)</li>
   *     <li>asset::Struct representing structural components used in the sesion (e.g. Tracks)</li>
   *     <li>asset::Meta representing meta objects created while editing (e.g. Automation)</li>
   * </ul> 
   * And of course there are various concret Asset subclasses, like asset::Clip,
   * asset::Effect, asset::Codec, asset::Track, asset::Dataset.
   * @note Assets objects have a strict unique identity and because of this are non-copyable.
   *       You can not create an Asset derived object without registering it with the AssetManager
   *       automatically. I is possible to copy the PAsset (smart pointer) though. 
   * 
   * @since 09/2007
   * @author Ichthyo
   */
  class Asset : private boost::noncopyable
    {
    public:

      /** 
       *  a POD comprised of all the information
       *  sufficiently identifying any given Asset. 
       */
      struct Ident
        {
          /** element ID, comprehensible but sanitized.
           *  The tuple (category, name, org) is unique.
           */
          string name;
    
          /** primary tree like classification of the asset.
           *  Includes the distinction of different kinds of Assets,
           *  like Media, Audio, Video, Effects...  */
          asset::Category category;
    
          /** origin or authorship id. 
           *  Can be a project abbreviation, a package id or just the authors nickname or UID.
           *  This allows for the compnent name to be more generic (e.g. "blur"). 
           *  Default for all assets provided by the core cinelerra-3 codebase is "cin3".
           */
          const string org;
    
          /** version number of the thing or concept represented by this asset.
           *  Of each unique tuple (name, category, org) there will be only one version 
           *  in the whole system. Version 0 is reserved for internal purposes. 
           *  Versions are considered to be ordered, and any higher version is 
           *  supposed to be fully backwards compatible to all previous versions.
           */
          const uint version;

          
          Ident (const string& n, 
                 const Category& cat, 
                 const string& o, 
                 const uint ver);
          
          /** @note equality ignores version differences */
          bool operator== (const Ident& other)  const
            {
              return org == other.org
                  && name == other.name 
                  && category == other.category;
            }
          bool operator!= (const Ident& other)  const
            {
              return !operator==(other);
            }
          
          int compare (const Ident& other)  const;
          
          operator string ()  const;
        };
      
         
      /* ===== Asset ID and Datafields ===== */
        
    public:    
      const Ident ident;     ///<  Asset identification tuple

      virtual const ID<Asset>& getID()  const { return id; }

      virtual operator string ()  const;
      
      
    protected:
      const ID<Asset> id;   ///<   Asset primary key.

      /** additional classification, selections or departments this asset belongs to.
       *  Groups are optional, non-exclusive and may be overlapping.
       */
      set<string> groups;

      /** user visible Name-ID. To be localized. */
      const string shortDesc;

      /** user visible qualification of the thing, unit or concept represented by this asset.
       *  perferably "in one line". To be localized.  */
      const string longDesc;

      
      
    protected:
      /** Asset is a Interface class; usually, objects of 
       *  concrete subclasses are created via specialized Factories.
       *  Calling this base ctor causes registration with AssetManager.
       */
      Asset (const Ident& idi);
      virtual ~Asset()           = 0;    ///< @note Asset is abstract
      
      /** release all links to other Asset objects held internally.
       *  The lifecycle of Asset objects is managed by smart pointers
       *  and the Asset manager. Calling \c release() breaks interconnectons
       *  to other Assets in the central Object network comprising the session.
       *  It is up to the AssetManager to asure the notification of any other
       *  components that may need to release references to the Asset object
       *  beeing removed. The rationale is, after releasing all interlinkings,
       *  when the AssetManager removes its DB entry for this asset, the
       *  smart pointer goes out of scope and triggers cleanup.
       */
      virtual void unlink ();
      
      /** variant of #unlink() dropping only the links to the given specific
       *  Asset, leaving all other links intact. Usable for propagating  */
      virtual void unlink (IDA target);
      
      friend class AssetManager;


    
    public:
      /** List of entities this asset depends on or requires to be functional. 
       *  May be empty. The head of this list can be considered the primary prerequisite
       */
      vector<shared_ptr<Asset> >  
      getParents ()  const;
      
      /** All the other assets requiring this asset to be functional. 
       *  For example, all the clips depending on a given media file. 
       *  May be empty. The dependency relation is transitive.
       */
      vector<shared_ptr<Asset> >
      getDependant ()  const;
      
      /** weather this asset is swithced on and consequently 
       *  included in the fixture and participates in rendering
       */
      bool isActive ()  const;
      
      /** change the enabled status of this asset.
       *  Note the corresponding #isActive predicate may 
       *  depend on the enablement status of parent assets as well
       */
      void enable ()  throw(cinelerra::error::State);
      
      
    };
    
    
    /** shorthand for refcounting Asset pointers */
    typedef shared_ptr<Asset> PAsset;
    
    /** ordering of Assets based on Ident tuple */
    inline bool operator<  (const PAsset& a1, const PAsset& a2) { return a1 && a2 && (-1==a1->ident.compare(a2->ident));}
    inline bool operator>  (const PAsset& a1, const PAsset& a2) { return   a2 < a1;  }
    inline bool operator>= (const PAsset& a1, const PAsset& a2) { return !(a1 < a2); }
    inline bool operator<= (const PAsset& a1, const PAsset& a2) { return !(a1 > a2); }

    /** ordering of Asset Ident tuples.
     *  @note version is irrelevant */
    inline int Asset::Ident::compare (const Asset::Ident& oi)  const
    { 
      int res;
      if (1 != (res=category.compare (oi.category)))  return res;
      if (1 != (res=org.compare (oi.org)))            return res;
      return name.compare (oi.name);
    }

    
    /** convienient for debugging */
    inline string str (const PAsset& a) 
    {
      if (a)
        return string (*a.get());
      else
        return "Asset(NULL)";
    }
    
    

} // namespace asset



namespace proc_interface
  {
  using asset::Asset;
  using asset::Category;
  using asset::ID;
  using asset::IDA;
  using asset::PAsset;
}

#endif