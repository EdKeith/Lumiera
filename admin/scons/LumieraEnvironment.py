# -*- python -*-
##
## LumieraEnvironment.py  -  custom SCons Environment
##

#  Copyright (C)         Lumiera.org
#    2008,               Hermann Vosseler <Ichthyostega@web.de>
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 2 of
#  the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#####################################################################


import os
from os import path

import SCons
import SCons.SConf
from SCons.Environment import Environment

from Buildhelper import *



class LumieraEnvironment(Environment):
    """ Custom SCons build environment for Lumiera
        This allows us to carry structured config data without
        using global vars. Idea inspired by Ardour. 
    """
    def __init__(self, pathConfig, **kw):
        Environment.__init__ (self,**kw)
        self.path = Record (pathConfig)
        self.libInfo = {}
        self.Tool("BuilderGCH")
        self.Tool("BuilderDoxygen")  
        self.Tool("ToolDistCC")
        self.Tool("ToolCCache")
        RegisterIcon_Builder(self)
    
    def Configure (self, *args, **kw):
        kw['env'] = self
        return apply(LumieraConfigContext, args, kw)
    
    def mergeConf (self,other):
        """ extract the library/compiler flags from other Environment.
            Optionally accepts a list or just sting(s) representing keys
            in our own libInfo Dictionary
        """
        if isinstance(other, list):
            for elm in other:
                self.mergeConf(elm)
        elif isinstance(other, str):
            if other in self.libInfo:
                self.mergeConf(self.libInfo[other])
        else:
            self.Append (LIBS = other.get ('LIBS',[]))
            self.Append (LIBPATH = other.get ('LIBPATH', []))    
            self.Append (CPPPATH = other.get('CPPPATH', []))
            self.Append (LINKFLAGS = other.get('LINKFLAGS', []))
        
        return self
    
    
    def addLibInfo (self, libID, minVersion=0, alias=None):
        """ use pkg-config to create an Environment describing the lib.
            Don't add this defs to the current Environment, rather store
            them in the libInfo Dictionary. 
        """
        minVersion = str(minVersion)
        if 0 != os.system('pkg-config --print-errors --exists "%s >= %s"' % (libID,minVersion)):
            print "Problems configuring the Library %s (>= %s)" % (libID,minVersion)
            return False
        
        self.libInfo[libID] = libInfo = Environment()
        libInfo["ENV"]["PKG_CONFIG_PATH"] = os.environ.get("PKG_CONFIG_PATH")
        libInfo.ParseConfig ('pkg-config --cflags --libs '+ libID )
        if alias:
            self.libInfo[alias] = libInfo
        return libInfo
    
    
    def SymLink(self, target, source, linktext=None):
        """ use python to create a symlink
        """
        def makeLink(target,source,env):
            if linktext:
                dest = linktext
            else:
                dest = str(source[0])
            link = str(target[0])
            os.symlink(dest, link)
        def reportLink(target,source,env):
            dest = str(source[0])
            link = str(target[0])
            return "Install link %s -> %s" % (link,dest)
        
        action = Action(makeLink,reportLink)
        self.Command (target,source, action)
    
    
    def defineSoname (self, *args,**kw):
        """ internal helper to extract or guess
            a suitable library SONAME, either using an
            explicit spec, falling back on the lib filename
        """
        if 'soname' in kw:
            soname = self.subst(kw['soname'])  # explicitely defined by user
        else:
            if len(args) > 0:
                pathname = args[0]
            elif 'target' in kw:
                pathname = kw['target']
            else:
                raise SyntaxError("Library builder requires target spec. Arguments: %s %s" % (args,kw))
            (dirprefix, libname) = path.split(pathname)
            if not libname:
                raise ValueError("Library name missing. Only got a directory: "+pathname)
            
            libname = "${SHLIBPREFIX}%s$SHLIBSUFFIX" % libname
            soname = self.subst(libname)       # else: use the library filename as DT_SONAME
        assert soname
        return soname
    
    
    def LumieraLibrary (self, *args,**kw):
        """ augments the built-in SharedLibrary() builder to add 
            some tweaks missing in SCons 1.0, like setting a SONAME proper
            instead of just passing the relative pathname to the linker
        """
        subEnv = self.Clone()
        subEnv.Append(LINKFLAGS = "-Wl,-soname="+self.defineSoname(*args,**kw))
        
        libBuilder = self.get_builder('SharedLibrary')
        return libBuilder(subEnv, *args,**kw); # invoke the predefined builder on the augmented environment
    
    
    def LumieraPlugin (self, *args,**kw):
        """ builds a shared library, autmented by some defaults for lumiera plugins.
        """
        subEnv = self.Clone()
        subEnv.Append(LINKFLAGS = "-Wl,-soname="+self.defineSoname(*args,**kw))
        
        libBuilder = self.get_builder('LoadableModule')
        return libBuilder(subEnv, *args,**kw); # invoke the predefined builder on the augmented environment
    
    
    def LumieraExe (self, *args,**kw):
        """ augments the built-in Program() builder to add a fixed rpath based on $ORIGIN
            That is: after searching LD_LIBRARY_PATH, but before the standard linker search,
            the directory relative to the position of the executable ($ORIGIN) is searched.
            This search path is active not only for the executable, but for all libraries
            it is linked with
        """
        subEnv = self.Clone()
        subEnv.Append( LINKFLAGS = "-Wl,-rpath=\\$$ORIGIN/$MODULES,--enable-new-dtags" )
        
        programBuilder = self.get_builder('Program')
        return programBuilder (subEnv, *args,**kw);





# extending the 'Configure' functionality of SCons,
# especially for library dependency checking
ConfigBase = SCons.SConf.SConfBase



class LumieraConfigContext(ConfigBase):
    """ Extends the SCons Configure context with some convenience methods
    """
    def __init__(self, *args,**kw):
        ConfigBase.__init__(self,*args,**kw)
    
    def CheckPkgConfig (self, libID, minVersion=0, alias=None):
        print "Checking for library configuration: %s " % libID
        # self.Message(self,"Checking for library configuration: %s " % libID)
        return self.env.addLibInfo (libID, minVersion, alias)



###### Lumiera custom tools and builders ########################


def RegisterIcon_Builder(env):
    """ Registers Custom Builders for generating and installing Icons.
        Additionally you need to build the tool (rsvg-convert.c)
        used to generate png from the svg source using librsvg. 
    """
    
    import render_icon as renderer  # load Joel's python script for invoking the rsvg-convert (SVG render)
    renderer.rsvgPath = env.subst("$TARGDIR/rsvg-convert")
    
    def invokeRenderer(target, source, env):
        source = str(source[0])
        targetdir = env.subst("$TARGDIR")
        renderer.main([source,targetdir])
        return 0
        
    def createIconTargets(target,source,env):
        """ parse the SVG to get the target file names """
        source = str(source[0])
        targetdir = os.path.basename(str(target[0]))
        targetfiles = renderer.getTargetNames(source)    # parse SVG
        return (["$TARGDIR/%s" % name for name in targetfiles], source)
    
    def IconCopy(env, source):
         """Copy icon to corresponding icon dir. """
         subdir = getDirname(source)
         return env.Install("$TARGDIR/%s" % subdir, source)
    
     
    buildIcon = env.Builder( action = Action(invokeRenderer, "rendering Icon: $SOURCE --> $TARGETS")
                           , single_source = True
                           , emitter = createIconTargets 
                           )
    env.Append(BUILDERS = {'IconRender' : buildIcon})    
    env.AddMethod(IconCopy)

