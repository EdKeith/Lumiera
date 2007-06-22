format 38

componentcanvas 128133 component_ref 128005 // Builder
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 117 113 2000 204 156
componentcanvas 128261 component_ref 128261 // Controller
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 573 100 2000 166 82
fragment 128389 "Backend Layer"
  xyzwh 89 690 2000 632 188
end
componentcanvas 128517 component_ref 128901 // Engine
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 95 318 2000 487 323
componentcanvas 128645 component_ref 129029 // Frame (Stream) Provider
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 254 711 2005 180 160
deploymentnodecanvas 128901 deploymentnode_ref 128005 // timeline
  xyzwh 142 3 2000 323 90
hubcanvas 129157 xyz 192 63 2005
hubcanvas 129285 xyz 222 62 2005
hubcanvas 129413 xyz 255 62 2005
hubcanvas 129541 xyz 290 62 2005
deploymentnodecanvas 130053 deploymentnode_ref 128261 // playlist
  xyzwh 148 389 2005 401 80
deploymentnodecanvas 130181 deploymentnode_ref 128389 // proc
  xyzwh 178 435 2010 97 69
deploymentnodecanvas 130309 deploymentnode_ref 128389 // proc
  xyzwh 424 430 2010 97 69
deploymentnodecanvas 130437 deploymentnode_ref 128517 // pnode
  xyzwh 202 507 2005 56 36
deploymentnodecanvas 130565 deploymentnode_ref 128517 // pnode
  xyzwh 201 546 2005 56 36
deploymentnodecanvas 130693 deploymentnode_ref 128517 // pnode
  xyzwh 201 585 2005 56 36
deploymentnodecanvas 130821 deploymentnode_ref 128517 // pnode
  xyzwh 318 505 2005 56 36
deploymentnodecanvas 130949 deploymentnode_ref 128389 // proc
  xyzwh 294 433 2010 97 69
deploymentnodecanvas 131077 deploymentnode_ref 128517 // pnode
  xyzwh 318 544 2005 56 36
deploymentnodecanvas 131205 deploymentnode_ref 128517 // pnode
  xyzwh 317 584 2005 56 36
deploymentnodecanvas 133125 deploymentnode_ref 128645 // Frame
  xyzwh 309 800 2010 70 36
deploymentnodecanvas 134021 deploymentnode_ref 128517 // pnode
  xyzwh 442 503 2005 56 36
deploymentnodecanvas 134149 deploymentnode_ref 128517 // pnode
  xyzwh 411 548 2005 56 36
deploymentnodecanvas 134277 deploymentnode_ref 128517 // pnode
  xyzwh 466 548 2005 56 36
deploymentnodecanvas 134405 deploymentnode_ref 128517 // pnode
  xyzwh 467 586 2005 56 36
componentcanvas 135045 component_ref 129285 // RenderPathManager
  draw_component_as_icon default show_component_req_prov default show_component_rea default
  xyzwh 505 222 2000 165 65
deploymentnodecanvas 136069 deploymentnode_ref 128773 // State
  xyzwh 703 226 2000 83 52
textcanvas 136325 "Render Engine carries out the actual processing by pulling output from the aprropriate processor "
  xyzwh 615 363 2000 135 89
note 136453 "Input for Builder"
  xyzwh 125 83 2005 104 36
note 136581 "Output of Builder"
  xyzwh 119 350 2005 104 36
note 136709 "optimizes"
  xyzwh 616 286 2000 75 35
packagecanvas 136837 
  package_ref 129029 // Controller
    xyzwh 486 68 1994 326 228
textcanvas 136965 "Builder is instructed by  Controller and transforms given timeline into a Render Engine, i.e.a network of processing nodes"
  xyzwh 326 196 2005 160 101
simplerelationcanvas 128773 simplerelation_ref 128133
  from ref 128517 z 1999 to ref 128645
line 129925 ----
  from ref 129157 z 2004 to ref 129541
line 133765 ----
  from ref 130949 z 2009 to ref 133125
simplerelationcanvas 134533 simplerelation_ref 128261 geometry HVr
  from ref 128261 z 1999 to point 653 188
  line 134789 z 1999 to ref 128133
end
