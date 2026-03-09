aux_source_directory(./ src_Task) 
aux_source_directory(./CentralizedSwarmTask src_CSTask) 
aux_source_directory(./DecentralizedSwarmTask src_DSTask) 
add_library(Task STATIC ${src_Task} ${src_CSTask} ${src_DSTask} )