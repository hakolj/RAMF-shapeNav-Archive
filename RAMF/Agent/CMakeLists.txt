aux_source_directory(./ src_Agent) 
aux_source_directory(./ShapeNavigationAgents src_ShapeNavAgents) 
add_library(Agent STATIC ${src_Agent} ${src_ShapeNavAgents} )