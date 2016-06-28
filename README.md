# Checkpointing-and-Restart-CppMPI

Simulator of checkpointing and restart of MPI applications.
*****
## To Compile ##
```
$ mpic++ dump_and_read.cpp -o a.out -std=c++11 -lboost_program_options
```
*****
## To Run ##
```         
$ mpirun -np <n_process> ./a.out [--size ARG] [--seconds ARG] [--iterations ARG] (--help)
```
Or..
```         
$ mpirun -np <n_process> ./a.out n_mb n_seconds n_iterations
```
Where:

* *-np <n_process>* : nr. of parallel process (mpi option) **REQUIRED!**
* *size arg* : size to be copied (MB) **REQUIRED!**
* *seconds arg* (=60) : seconds to wait between two iterations
* *iterations arg* (=1) : iterations of process
* *help*: produce help message
*****
## Work Environment ##
_C++_

_Open MPI:_
```
$ sudo apt-get install libopenmpi-dev openmpi-bin
```
_Boost:_

See [tutorial](http://www.boost.org/doc/libs/1_43_0/more/getting_started/unix-variants.html#link-your-program-to-a-boost-library) or download boost_1_43_0.tar.bz2  and:
```
$ tar --bzip2 -xf /path/to/boost_1_43_0.tar.bz2  
$ cd boost_1_43_0  
$ ./bootsrap.sh  
$ ./b2  
$ ./bjam install
```
In the code add: **#include <boost/whatever.hpp>**

*******
## Code Description ##

1. Creare eseguibile che generi n processi ciascuno dei quali si deve caricare in memoria m MB (vedete voi se in un solo array o in una struttura più complicata ).

2. quando tutti i processi hanno fatto, nello stesso istante iniziano a scrivere su disco, ciascun processo su un file binario. (quindi alla fine si avranno n file ciascuno di m MB.)

3. n e m devono essere parametrizzati da linea di comando.

4. prevedere che i processi invece di prendere i dati da /dev/zero leggano altrettanti file - anche questo va parametrizzato da linea di comando - ad esempio con un flag finale 1/0.

5. Il tutto va messo in un loop che riparte dopo un tempo di sleep configurabile - il numero di loop e il tempo di sleep da linea di comando con default 1 e 60s.

6. includere un help (--help) e se uno mette meno parametri del necessario, almeno n e m,  se ne deve accorgere e stampare l'help - mettete anche un controllo di sicurezza su n e m in modo che l'utonto non provi a leggere e scrivere exabyte di dati. (hint: in c++ per le opzioni di linea di comando date un'occhiata a boost::program_options, in python è più semplice)
