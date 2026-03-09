# Overview
RAMF, which stands for `Reinforcement learning of Active Matter in Fluid`, is a framework for Eulerian-Lagrangian simulation of smart active matters in fluid flow.
In a word, active matters are considered as point-like objects and their Lagrangian trajectories are simulated with flow field information. The flow field is simulated or loaded on Eulerian grid. The concept `Smart` is embodied by the integration of reinforcement learning algorithm (with pytorch's c++ interfacce, libtorch) to train optimal strategy for different kind of tasks. 

RAMF is organized by two main modules:
1. Environment
2. Agent


The purpose of [Environment](#Environment) is to simulate active matter and fluid flow. It works similar to gym environment.

The purpose of [Agent](#Agent) is to tell the smart active matter how to react to environment according to their perception signals. RAMF supports reinforcement learning agents, which means user can use provided RL algorithm or write their own ones to train smart active matters to complete certain tasks. Except for RL agents, RAMF users can also write pro-programmed strategy for certain task configurations. 



## <span id="Environment"> Environment </span>
 Module Environment has five submodules:
1. Active Matter
2. Fluid
3. Sensor
4. Actor
5. Task


According to the specific scientific problem, the actual implementation of these submodules can be selected individually and work together, all controlled by config files.
For instance, one wants to simulate microorganisms in a homogeneous isotropic turbulent flow field, and use reinforcement learning algorithm (say DDPG) to find optimal swimming strategy to navigate against the gravity direction. Then, one should use `Inertialess swimmers` for `Active Matter` component, `HIT` for `Fluid` component, `NavigationDirection` for `Task` and corresponding Sensor and Actors. 
Any of the submodules can be changed flexibly in config files, with very minor or even no effect on other submodules. 

More detailed introduction to each submodule are provided below. 

### 1. Active Matter
To simulate the Lagrangian dynamics of several kinds of active matter. 
Example: inertialess microswimmers.

### 2. Fluid
To simulate or load the Eulerian flow field. Second-order Lagrangian interpolation is used for interpolation of flow information (e.g. fluid velocity) at the position of active matters.
Example: homogeneous isotropic turbulent flow.

### 3. Sensor
To calculate the perceived signal of active matters. These signals are used for the input of the smart strategy. Example: local fluid velocity.

### 4. Actor
To alter the dynamics of smart active matters according to the output of strategy.
Example: changing swimming speed of active matters. 

### 5. Task
To define what is the goal and reward function for the active matter. Example: for point-navigation, given the reward function based on the distance to the target.


## <span id="Agent"> Agent </span>
Module Agent contains several classical reinforcement learning algorithms, such as Q-learning, D3QN, DDPG, PPO, etc, as well as some pre-programmed strategy for specific scientific problems. 
Depp RL algorithm is implemented using pytorch C++ interface, [libtorch](https://github.com/pytorch/pytorch/blob/main/docs/libtorch.rst).
Users can also implement their own reinforcement learning algorithms or certain strategies.

