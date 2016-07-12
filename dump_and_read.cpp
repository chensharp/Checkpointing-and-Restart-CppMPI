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
#include <vector>
#include <iterator>
#include <numeric>

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
	int id;
        int ierr;
        int p;
        double wtime;

	//  Initialize MPI.
        ierr = MPI_Init ( &argc, &argv );

        //  Get the number of processes.
        ierr = MPI_Comm_size ( MPI_COMM_WORLD, &p );

        //  Get the individual process ID.
        ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );
 
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
			if(id == 0){
            			cout << "\nUsage: mpirun -np <n_process> ./a.out [--size ARG]* [--seconds ARG] [--iterations ARG]\n";
				cout << "   ...or...\n";
				cout << "       mpirun -np <n_process> ./a.out n_mb* n_seconds n_iterations\n";
	    			cout << "* Required Argument\n\n";
            			cout << desc << "\n";
			}
			MPI_Finalize ( );
            		return 0;
        	}
		po::notify(vm);
    	}
	catch(std::exception& e){
        	cout << e.what() << "\n";
        	return 1;
	}

	int n = 0, size_byte = 0;
	float byte = 0.;

	if(mb <= 0){
		std::cout << "ERROR!! \"size\"must be positive and not zero.\n"<< std::endl;
	}
	else{
		if(mb > 1000000){
			std::cout << "ERROR SIZE!! Please choose size < 1000000 MB"<< std::endl;
		}
		else{
			byte = mb * 1000000;
			size_byte = (int)std::ceil(byte);
		
			int i;
			
			//for each process, create a vector (size = tot iterations) that it will contain delta write times of each iteration.
			std::vector<double> delta_writeXprocess(its);

			for(i = 0; i < its; i++){
				if(id == 0){
					cout << "\n";				
				}
			
			//	ifstream infile;
			//	infile.open("/dev/zero" ,ios::binary|ios::in);
			//	outfile.open(name_output_file, ios::binary|ios::out);

				/* when execution leaves this scope,
				   it is guaranteed that vector object will be destructed and memory will be freed.*/
				{
					//char* buffer;
					//buffer = new char[size_byte];
					std::vector<char> buffer(size_byte);
					double start_writeXprocess = 0.;
					//infile.read(buffer,byte);

//					cout << "Iteration = "<< i <<" - Process " << id << ": barrier!\n";
					//synchronization: no process can pass the barrier until all of them call the function.
					MPI_Barrier(MPI_COMM_WORLD);
					
					std::ofstream outfile;
					std::string name_output_file = "outFile_it" + std::to_string(i) + "_p" + std::to_string(id)+".txt";
					
					start_writeXprocess = MPI_Wtime ( );
					outfile.open(name_output_file, ios::binary|ios::out);
					
				//	outfile.write(buffer,byte);
					copy(buffer.begin(), buffer.end(), std::ostreambuf_iterator<char>(outfile));	

					//infile.close();
					outfile.close();
					
					delta_writeXprocess[i] = MPI_Wtime ( ) - start_writeXprocess;
					cout << "I wrote " << mb << " MB to file " << name_output_file << " in "<< delta_writeXprocess[i] <<" seconds.\n";
					//delete[] buffer;
				}
				wait (sec);
			}

			std::vector<double> delta_write;
			if(id == 0){
				//create the main vectors that contain all delta times of all processes.
				delta_write.assign(its*p, 0.);
			}
			int disposition[p];
			int count[p];
			for(int k=0;k<p;k++){
				disposition[k] = k*its;
				count[k] = its;
				if(id == 0){cout<<"disposition["<<k<<"] = "<<disposition[k]<<" -- count["<<k<<"] = "<<count[k]<<"\n";}
			}	
			MPI_Gatherv(&delta_writeXprocess.front(), its, MPI_DOUBLE, &delta_write.front(), count, disposition, MPI_DOUBLE, 0,MPI_COMM_WORLD);
			cout<<"passo...\n";
			if(id == 0){
				double average_write = std::accumulate(delta_write.begin(), delta_write.end(), 0.0)/delta_write.size();
				cout << "average_write (by function) = "<<average_write<<"\n";
			
			//	double tot = 0.;
				for(int j=0; j<delta_write.size();j++){
					cout << "delta_write["<<j<<"] = "<<delta_write[j] <<"\n";
			//		tot = tot + delta_write[j];
				}
			//	cout << ".. tot = "<< tot << " - av = " << tot/delta_write.size() << "\n\n";
			}
		}
	}
	//  Terminate MPI
	MPI_Finalize ( );
	return 0;
}
