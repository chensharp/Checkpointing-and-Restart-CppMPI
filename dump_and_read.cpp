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
	bool readFromFile;

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
			("readFromFile",po::value<bool>(&readFromFile),			"enable to read from files (on/off, true/false, 1/0)")
        	;

        	po::positional_options_description p;
		
		p.add("size", 1);
		p.add("seconds", 1);
		p.add("iterations", 1);
		p.add("readFromFile",1);

        	po::variables_map vm;
        	po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
		
        	if (vm.count("help")) {
			if(id == 0){
            			cout << "\nUsage: mpirun -np <n_process> ./a.out [--size ARG]* [--seconds ARG] [--iterations ARG] [--readFromFile ARG]\n";
				cout << "   ...or...\n";
				cout << "       mpirun -np <n_process> ./a.out n_mb* n_seconds n_iterations readFromFile\n";
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
			cout << "readFromFile = "<<readFromFile<<"\n";
			byte = mb * 1000000;
			size_byte = (int)std::ceil(byte);
		
			int i;
			
			//for each process, create a vector (size = tot iterations) that it will contain delta write times of each iteration.
			std::vector<double> delta_wp(its);
			std::vector<double> delta_rp;
			if(readFromFile){
				delta_rp.assign(its, 0.);
			}

			for(i = 0; i < its; i++){
				if(id == 0){
					cout << "\n";				
				}
			
				/* when execution leaves this scope,
				   it is guaranteed that vector object will be destructed and memory will be freed.*/
				{
					std::vector<char> buffer(size_byte);
					
					if(readFromFile && i>0){
						double start_read = 0.;

						std::string inputFile = "outFile_it" + std::to_string(i-1) + "_p" + std::to_string(id)+".txt";
						start_read = MPI_Wtime ( );
						ifstream infile(inputFile, std::ios::binary);
						buffer.assign((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
						delta_rp[i] = MPI_Wtime ( ) - start_read;
						cout << "I read " << mb << " MB from file " << inputFile << " in "<< delta_rp[i] <<" seconds.\n";
					}

					//synchronization: no process can pass the barrier until all of them call the function.
					MPI_Barrier(MPI_COMM_WORLD);

					double start_write = 0.;

					std::ofstream outfile;
					std::string outputFile = "outFile_it" + std::to_string(i) + "_p" + std::to_string(id)+".txt";
					
					start_write = MPI_Wtime ( );
					outfile.open(outputFile, ios::binary|ios::out);
					
					copy(buffer.begin(), buffer.end(), std::ostreambuf_iterator<char>(outfile));	

					outfile.close();
					
					delta_wp[i] = MPI_Wtime ( ) - start_write;
					cout << "I wrote " << mb << " MB to file " << outputFile << " in "<< delta_wp[i] <<" seconds.\n";
				}
				wait (sec);
			}

			std::vector<double> delta_write;
			std::vector<double> delta_read;
			if(id == 0){
				//create the main vectors that contain all delta times of all processes.
				delta_write.assign(its*p, 0.);
				if(readFromFile){
					delta_read.assign(its*p, 0.);
				}
			}

			int disposition[p];
			int count[p];
			for(int k=0;k<p;k++){
				disposition[k] = k*its;
				count[k] = its;
			//	if(id == 0){cout<<"disposition["<<k<<"] = "<<disposition[k]<<" -- count["<<k<<"] = "<<count[k]<<"\n";}
			}
	
			MPI_Gatherv(&delta_wp.front(), its, MPI_DOUBLE, &delta_write.front(),
								count, disposition, MPI_DOUBLE, 0,MPI_COMM_WORLD);
			if(readFromFile){
				MPI_Gatherv(&delta_rp.front(), its, MPI_DOUBLE, &delta_read.front(),
                                                                count, disposition, MPI_DOUBLE, 0,MPI_COMM_WORLD);
			}
			
			if(id == 0){
				double average_write = std::accumulate(delta_write.begin(), delta_write.end(), 0.0)/delta_write.size();
				
				cout << "\n------------------------\nAverage write time: "<<average_write<<" seconds.\n";
				if(readFromFile){
					double average_read = std::accumulate(delta_read.begin(), delta_read.end(), 0.0)/(its*p-p);
					cout << "Average read time: "<<average_read<<" seconds.\n";
				}
				cout << "------------------------\n\n";
			
			/*	for(int j=0; j<delta_read.size();j++){
					cout << "delta_read["<<j<<"] = "<<delta_read[j] <<"\n";
				}*/
			}
		}
	}
	//  Terminate MPI
	MPI_Finalize ( );
	return 0;
}
