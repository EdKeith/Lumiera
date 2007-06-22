format 38

fragment 128005 "UI Layer"
  xyzwh 321 22 2000 829 100
end
fragment 128133 "Processing Layer"
  xyzwh 64 156 2000 1089 655
  separator 4833
end
fragment 128261 "Backend Layer"
  xyzwh 64 848 2000 1089 105
end
packagecanvas 128389 
  package_ref 128133 // AssetManager
    xyzwh 561 201 2005 252 214
packagecanvas 128517 
  package_ref 128261 // MObject
    xyzwh 94 201 2006 458 335
packagecanvas 128645 
  package_ref 128389 // RenderEngine
    xyzwh 94 551 2005 458 235
componentcanvas 128773 component_ref 128005 // Builder
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 121 419 2015 229 105
componentcanvas 128901 component_ref 128133 // Session
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 122 243 2011 323 156
componentcanvas 129029 component_ref 128261 // Controller
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 372 419 2011 175 105
componentcanvas 129157 component_ref 128389 // EDL
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 288 302 2016 143 79
componentcanvas 129285 component_ref 128517 // Fixture
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 124 319 2016 143 69
note 129541 "Structures edited by the User"
  xyzwh 43 269 2016 181 41
componentcanvas 129669 component_ref 128645 // AssetManagement
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 577 241 2010 208 159
componentcanvas 129797 component_ref 128773 // Dispatcher
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 406 31 2005 179 69
componentcanvas 129925 component_ref 128901 // Engine
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 118 602 2010 235 176
componentcanvas 130053 component_ref 129029 // Frame (Stream) Provider
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 121 876 2005 229 69
componentcanvas 130181 component_ref 129157 // Cache
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 391 876 2005 179 69
note 131717 "Coordinates Playback and Rendering"
  xyzwh 483 494 2016 149 63
note 131845 "border of the low-level, performance-critical part of the system"
  xyzwh 666 450 2006 167 84
note 131973 "just works, never decides"
  xyzwh 317 668 2015 110 59
note 132101 "codecs, stream I/O here"
  xyzwh 647 904 2005 166 39
simplerelationcanvas 131205 simplerelation_ref 128005
  from ref 130053 z 2004 to ref 130181
simplerelationcanvas 131333 simplerelation_ref 128133
  from ref 129925 z 2004 to ref 130053
line 130309 -_-_
  from ref 129797 z 2004 to ref 128901
line 130821 -_-_ geometry HVr
  from ref 129797 z 2004 to point 493 469
  line 130949 z 2004 to ref 129029
line 131077 -_-_
  from ref 129797 z 2004 to ref 129669
end
