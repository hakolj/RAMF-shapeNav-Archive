# Minimal Example
Following this instruction, you will be guided to run a minimal example of RAMF.

## Config file
RAMF is controlled by a single config file, which is specified by the variable `configpath` in RAMF_main.cpp.
A config file consists of several parts, which begins with `SET -MODULENAME` and ends with `ENDSET -MODULENAME`. 
Each part is responsible for a certain module, including:
1. GLOBAL
2. AGENT
3. FLUID
4. ACTIVEMATTER
5. TASK
6. SENSOR
7. ACTOR

Each config file must contain and only contain these 7 modules. 

Inside the module, the values of variables are given by the form of `key = value`, where values can be float point numbers, integers, strings, or arrays of these. 
The scope of the key-value pair is within the module. It is allowed to define a key with exactly the same name as a key in another module.
You can use `#` to make comments.

The folder `doc` contains a config file `config_example.cfg` for the minimal example. 
This example contains a simple navigation problem. The task is that some vehicles need to move to a point (3,3,3) in a 3D space. There is no flow. To find optimal navigation route, A classical reinforcement learning algorithm, PPO, is used for finding the strategy. 

To run this example, you need to specify the path to config file. This can be done in two ways. 
1. (Recommended) Specify config path by argument `-c`, e.g. `./RAMF -c path/to/config.cfg`. 
2. Specify the variable `configPath` in `RAMF_main.cpp` and run RAMF without `-c`. Rebuild is needed every time you change `configPath`. 

If the example runs successfully, a folder will be created at the path specified by the Global config entry `caseDir`. By default, the training or simulation will be run for once, and the output files are located at `/path/to/caseDir/0`.

## Case and Subcase
Each config file determines a `Case`, which means each module and their parameters are given. However, due to the random and non-linear nature of the physical problem and the reinforcement learning process, sometimes it is necessary to make multiple simulations or trainings using exactly the same configurations. 

The `Subcase` feature of RAMF can help you with this. By specify `-sc`, you can run several subcases using one config file. For instance, `./RAMF -c ./config.cfg -sc 0_10` runs 11 subcase (indices from 0 to 10) using the configs in `./config.cfg`. You must use `_` as delimiter.

Each subcase will create a folder under `caseDir`, named by the indices of subcases.


## OpenMP
You can also specify the number of thread used by OpenMP. You can either use argument `-n`, e.g. `./RAMF -n 4 -c path/to/config.cfg` (recommended), or edit the value in `omp_set_num_threads(number_of_process)` in `RAMF_main.cpp`.

## Input Arguments
Allowed input arguments are list below.

| Args | Description | Example |
| ---  | ----------- | ------ |
| `-c`   | setting path to config file | `./RAMF -c ./config.cfg` |
| `-sc`   | setting subcase indices | `./RAMF -sc 0_10` |
| `-n`   | setting number of thread used by OpenMP | `./RAMF -n 4` |