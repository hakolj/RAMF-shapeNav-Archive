aux_source_directory(./ src_Actor) 
aux_source_directory(./DecentralizedSwarmActor src_DCSActor) 
add_library(Actor STATIC ${src_Actor} ${src_DCSActor} )
