
#include <boost/program_options.hpp>

using namespace boost;
namespace po = boost::program_options;

#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mpi.h>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>


using namespace std;

int main ( int argc, char *argv[] );

// A helper function to simplify the main part.
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
    copy(v.begin(), v.end(), ostream_iterator<T>(os, " "));
    return os;
}

void wait ( float seconds )
{
  clock_t endwait;
  endwait = clock () + seconds * CLOCKS_PER_SEC ;
  while (clock() < endwait) {}
}

int main ( int argc, char *argv[] ){
	int its = 0;
	float sec = 0., mb = 0.;  
	try {
        	int opt;
        	int portnum;

		po::options_description desc("Allowed options");
        	desc.add_options()
            		("help",							"produce help message")
            		("size",	po::value<float>(&mb)->required(),		"size to be copied (MB)")
	    		("seconds",	po::value<float>(&sec)->default_value(60),	"seconds to wait between two iterations")
	    		("iterations",	po::value<int>(&its)->default_value(1),		"iterations of process")
        	;

        	po::positional_options_description p;
		
		p.add("size", 1);
		p.add("seconds", 1);
		p.add("iterations", 1);

        	po::variables_map vm;
        	po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

        	if (vm.count("help")) {
            		cout << "\nUsage: mpirun -np <n_process> ./a.out [--size ARG]* [--seconds ARG] [--iterations ARG]\n";
	    		cout << "* Required Argument\n\n";
            		cout << desc << "\n";
            		return 0;
        	}
		
		po::notify(vm);

		cout << "MB = " << mb << "\n";
        	cout << "Seconds = " << sec << "\n";
		cout << "Number iterations = " << its << "\n";
    	}
	catch(std::exception& e){
        	cout << e.what() << "\n";
        	return 1;
	}

	int id;
	int ierr;
	int p;
	double wtime;

	int n = 0, size_byte = 0;
	float byte = 0.;

	if(mb <= 0){
		std::cout << "ERROR!! \"size\"must be positive and not zero.\n"<< std::endl;
	}
	else{
		std::cout << "MB = "<< mb << std::endl;  

		if(mb > 1000000){
			std::cout << "ERROR SIZE!! Please choose size < 1000000 MB"<< std::endl;
		}
		else{
			byte = mb * 1000000;
			size_byte = (int)std::ceil(byte);
			std::cout << "it = "<< its << " - sec = " << sec << std::endl;

			//  Initialize MPI.
			ierr = MPI_Init ( &argc, &argv );

			//  Get the number of processes.
			ierr = MPI_Comm_size ( MPI_COMM_WORLD, &p );

			//  Get the individual process ID.
			ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );

			int i;
			for(i = 0; i < its; i++){
				cout << "----------------------- #it = " << i << " -----------------------\n";
				//  Every process prints a hello.
				if ( id == 0 ) {
					wtime = MPI_Wtime ( );
				}
				
				cout << "  Process " << id << " says 'Hello!! :-)'\n";

				ifstream infile;
				infile.open("/dev/zero" ,ios::binary|ios::in);

				ofstream outfile;

				std::string name_output_file = "outFile" + std::to_string(id);
				std::cout << name_output_file << '\n';

				outfile.open(name_output_file, ios::binary|ios::out);

				char* buffer;
				buffer = new char[size_byte];
				
				infile.read(buffer,byte);
				cout << "prima... id:" <<id <<"\n";
				//synchronization: no process can pass the barrier until all of them call the function.
				MPI_Barrier(MPI_COMM_WORLD);
				cout << "dopo ... id:" <<id <<"\n";

				outfile.write(buffer,byte);

				infile.close();
				outfile.close();

				delete[] buffer;

				if ( id == 0 ){
					wtime = MPI_Wtime ( ) - wtime;
					cout << "  Elapsed wall clock time = " << wtime << " seconds.\n";
				}
				
				wait (sec);
				cout << "------------------------------- END -------------------------------\n";
			} 
			cout << "appena prima il termine\n";
			//  Terminate MPI.
			MPI_Finalize ( );
			cout << "dopo il termine\n";
		}
	}
	return 0;
}
