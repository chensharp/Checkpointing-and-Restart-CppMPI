# README #

Simulator of checkpointing and restart of MPI applications.
*****
## To Compile ##
```
$ mpic++ dump_and_read.cpp -o a.out -std=c++11 -lboost_program_options
```
*****
## To Run ##
```         
$ mpirun -np <n_process> ./a.out [--size ARG] [--seconds ARG] [--iterations ARG] [--readFromFile ARG](--help)
```
Or..
```         
$ mpirun -np <n_process> ./a.out n_mb n_seconds n_iterations readFromFile
```
Where:

* *-np <n_process>* : nr. of parallel process (mpi option) **REQUIRED!**
* *size arg* : size to be copied (MB) **REQUIRED!**
* *seconds arg* (=60) : seconds to wait between two iterations
* *iterations arg* (=1) : iterations of process
* *readFromFile arg* (=0) : enable to read from files (on/off, true/false, 1/0)
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

4.  prevedere che i processi invece di prendere i dati da /dev/zero leggano altrettanti file - anche questo va parametrizzato da linea di comando - ad esempio con un flag finale 1/0.

5. Il tutto va messo in un loop che riparte dopo un tempo di sleep configurabile - il numero di loop e il tempo di sleep da linea di comando con default 1 e 60s.

6. includere un help (--help) e se uno mette meno parametri del necessario, almeno n e m,  se ne deve accorgere e stampare l'help - mettete anche un controllo di sicurezza su n e m in modo che l'utonto non provi a leggere e scrivere exabyte di dati. (hint: in c++ per le opzioni di linea di comando date un'occhiata a boost::program_options, in python è più semplice)
*******
## Troubleshooting ##
**1. Error while loading shared libraries**
```
$ root@avoton1:~/dump_and_read# mpirun -np 2 ./a.out1 1 2 3
$ ./a.out1: error while loading shared libraries: libboost_program_options.so.1.61.0: cannot open shared object file: No such file or directory
$ ./a.out1: error while loading shared libraries: libboost_program_options.so.1.61.0: cannot open shared object file: No such file or directory
--------------------------------------------------------------------------
$ mpirun noticed that the job aborted, but has no info as to the process
$ that caused that situation.
--------------------------------------------------------------------------
```

Find the folder where is located the missing file (in this case "libboost_program_options.so.1.61.0").
Into this folder you can see:
```
$ root@avoton1:/usr/local/lib# ll
$ total 153016
$ drwxr-xr-x  5 root root      4096 giu 29 11:07 ./
$ drwxr-xr-x 12 root root      4096 giu 25  2015 ../
$ -rw-r--r--  1 root root      2602 giu 29 11:07 libboost_atomic.a
$ lrwxrwxrwx  1 root root        25 giu 29 11:07 libboost_atomic.so -> libboost_atomic.so.1.61.0*
$ -rwxr-xr-x  1 root root      8143 giu 29 11:07 libboost_atomic.so.1.61.0*
$ -rw-r--r--  1 root root    113270 giu 29 11:07 libboost_chrono.a
$ lrwxrwxrwx  1 root root        25 giu 29 11:07 libboost_chrono.so -> libboost_chrono.so.1.61.0*
         ...
```
... so the path will be:
```
$ root@avoton1:/usr/local/lib# pwd
$ /usr/local/lib
```
Finally you can fix the problem using:
```
$ root@avoton1:~# export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```
