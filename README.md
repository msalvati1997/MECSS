A MOBILE EDGE COMPUTING ARCHITECTURE SYSTEM BASED ON THE VIDEO STREAMING CASE STUDY
==================
Modeling, simulation and performance evaluation's project
A.A 2021/2022
-----------------------------

 Project's description
==================
Video streaming and computer games are among the most popular and bandwidth-consuming media on the Internet. Video content consumes about 70% of total Internet bandwidth usage today. Advances in multimedia generation tools, high processing power and high-speed connectivity have enabled the generation of live, interactive and multi-view multimedia content. Edge computing aims to provide storage and computational resources close to the user at the network edge, to minimize latency and response times. Edge computing extends cloud resources to the end of the service network to provide very low latency and real-time communication.
The aim of the project is to implement a study on a video content offload algorithm that aims to optimize both in terms of delay and energy consumption.
Video streaming is transmitted in less than 1 second, typically 100-500 milliseconds.
When real-time video services are being provided to individual users such as video streaming is important:
- minimize the total execution time. (less than 500 milliseconds)



Project installation and organization
==================

1. Clone this repo

```bash
cd
git clone https://github.com/msalvati1997/MECCS.git
```

2. Run simulation

- You can start simulation runs using the makefile. 
There are two run modes: 

|  RUN MODE 	|  DESCRIPTION  	|   	
|---	|---	|
|   RELEASE | high-performance run, does not print further details of simulations	|
|   DEBUG	| run that allows you to read every step of the simulation, less performing 	|  


  
```bash
./make 
Available targets:
  help
  clean
  clean_finite_alg1
  clean_finite_alg1_migliorativo
  clean_finite_alg2
  clean_infinite_alg1
  clean_infinite_alg1_migliorativo
  clean_infinite_alg2
  release_finite_alg1
  release_finite_alg1_migliorativo
  release_finite_alg2
  release_infinite_alg1
  release_infinite_alg1_migliorativo
  release_infinite_alg2
  debug_finite_alg1
  debug_finite_alg1_migliorativo
  debug_finite_alg2
  debug_infinite_alg1
  debug_infinite_alg1_migliorativo
  debug_infinite_alg2

```
Results
 -----------------------------
The project's results are located in the ./results folder and are divided by simulation and algorithm type. 
Csv files were used to graph the results and carry out statistical studies.

Author
======================= 
- [Martina Salvati](https://github.com/msalvati1997)   (0292307)
- [Simone Benedetti](https://github.com/simobenny8)    (0295385)

