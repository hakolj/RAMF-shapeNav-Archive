aux_source_directory("./" src_Sensor) 
aux_source_directory("./CentralizedSwarmSensor" src_CSSensor) 
aux_source_directory("./DecentralizedSwarmSensor" src_DSSensor) 

# include_directories("./")
add_library(Sensor STATIC ${src_Sensor} ${src_CSSensor} ${src_DSSensor})