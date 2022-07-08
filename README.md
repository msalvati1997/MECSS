# MECSS
Mobile Edge Cloud Simulating System

 Project's description
==================
Lo streaming video e i giochi per computer sono tra i media più popolari e con il più alto consumo di larghezza di banda in Internet. I contenuti video consumano oggi circa il 70% dell'utilizzo totale della larghezza di banda in Internet. I progressi negli strumenti di generazione multimediale, l'elevata potenza di elaborazione e la connettività ad alta velocità hanno consentito la generazione di contenuti multimediali live, interattivi e multi-view. L’edge computing ha lo scopo di fornire risorse di storage e computazionali vicino all'utente all'edge della rete, per ridurre al minimo la latenza e i tempi di risposta. L'edge computing estende  i servizi e le risorse cloud alla fine della rete per fornire una latenza molto bassa e una comunicazione in tempo reale.
Lo scopo del progetto è implementare uno studio su un algoritmo di offload dei contenuti video che mira all'ottimizzazione congiunta in termini di ritardo e consumo energetico. 
Lo streaming video viene trasmesso in meno di 1 secondo, in genere 100-500 millisecondi.
Quando i servizi video in tempo reale vengono forniti a singoli utenti come lo streaming video è importante:
- ridurre al minimo il tempo di esecuzione totale. (inferiore ai 500 millisecondi)
-	ridurre al minimo il consumo totale di energia dei dispositivi.



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
The project's results are located in the results folder and are divided by simulation and algorithm type. 
Csv files were used to graph the results and carry out statistical studies.

Author
=======================
- [Martina Salvati](https://github.com/msalvati1997) 
- [Simone Benedetti](https://github.com/simobenny8)
