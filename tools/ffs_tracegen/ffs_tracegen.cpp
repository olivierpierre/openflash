#include <iostream>
#include <stdint.h>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <getopt.h>

#define LINUX_PAGE_SIZE 4096

using namespace std;

typedef enum
{
  RANDOM,
  SEQUENTIAL
} pattern_type;

typedef enum
{
  READ,
  WRITE
} req_type;

typedef struct
{
  int random_rate; 	// percentage of random req.
  int ran_aligned; 	// page laigned ran req
  uint64_t file_size;	// file size in bytes (address space)
  double ia_time;	// inter arrival time in ms
  double time_offset;	// stat at time_offset
  int inode;		// inode of the file to access
  int req_num;		// number of requests
  int create;		// we must create the file
  int read_rate;	// percentage of read requests
  int req_size;		// request size
  int print_config;	// print config
} config;

void print_help_and_exit(int argc, char **argv);
void default_config(config &cfg);
void print_config(config &cfg);

int main(int argc, char **argv)
{
  int c;
  config cfg;
  
  default_config(cfg);
  
  while(1)
  {
    static struct option long_options[] =
    {
      {"ran-rate", required_argument, 0, 'r'},
      {"ran-align", no_argument, 0, 'a'},
      {"file-size", required_argument, 0, 's'},
      {"ia", required_argument, 0, 'i'},
      {"time-offset", required_argument, 0, 'o'},
      {"inode", required_argument, 0, 'd'},
      {"requests-num", required_argument, 0, 'n'},
      {"create", required_argument, 0, 'c'},
      {"read-rate", required_argument, 0, 'l'},
      {"request-size", required_argument, 0, 'p'},
      {"print-config", no_argument, 0, 'x'},
      {0,0,0,0}
    };
    int option_index = 0;
    
    c = getopt_long(argc, argv, "r:as:i:o:d:n:l:c:p:x", long_options, &option_index);
    
    if(c == -1)
      break;
      
     switch(c)
    {
      case 'r':
	cfg.random_rate = atoi(optarg);
	break;
	
      case 'a':
	cfg.ran_aligned = 1;
	break;
	
      case 's':
	cfg.file_size = atoll(optarg);
	break;
	
      case 'i':
	cfg.ia_time = atof(optarg);
	break;
	
      case 'o':
	cfg.time_offset = atof(optarg);
	break;
	
      case 'd':
	cfg.inode = atoi(optarg);
	break;
	
      case 'n':
	cfg.req_num = atoi(optarg);
	break;
	
      case 'l':
	cfg.read_rate = atoi(optarg);
	break;
	
      case 'c':
	cfg.create = atoi(optarg);
	
      case 'p':
	cfg.req_size = atoi(optarg);
	break;
	
      case 'x':
	cfg.print_config = 1;
	break;
      
      case 'h':
      default:
	print_help_and_exit(argc, argv);
    }
  }
  
  if(cfg.print_config)
    print_config(cfg);
   
  
  if(cfg.file_size == 0xFFFFFFFFFFFFFFFF)
  {
    cerr << "ERROR : please indicate at least the file size" << endl;
    print_help_and_exit(argc, argv);
  }
  
  // create
  if(cfg.create)
  {
    cout << cfg.time_offset << ";\tcreate;\t\t" << cfg.inode << ";\t" << cfg.create << endl;
    cfg.time_offset += cfg.ia_time;
  }
  
  // open
  cout << cfg.time_offset << ";\topen;\t\t" << cfg.inode << ";\t0;\t0;\t0" << endl;
  cfg.time_offset += cfg.ia_time;
  
  uint64_t prev_addr = 0xFFFFFFFFFFFFFFFF;
  for(int i=0; i<cfg.req_num; i++)
  {
    uint64_t addr;
    string op_string;
    pattern_type pt = SEQUENTIAL;
    req_type rt = WRITE;
    
    if(rand()%100 < cfg.random_rate)
      pt = RANDOM;
      
    if(rand()%100 < cfg.read_rate)
      rt = READ;
      
    if(pt == SEQUENTIAL)
    {
      if(prev_addr == 0xFFFFFFFFFFFFFFFF)
	addr = 0;
      else
	addr = prev_addr + cfg.req_size;
    }
    else
    {
      if(cfg.ran_aligned)
	addr = (rand()%((cfg.file_size-cfg.req_size)/4096))*4096;
      else
	addr = rand()%(cfg.file_size-cfg.req_size);
    }
    
    if(rt == READ)
      op_string = "read";
    else
      op_string = "write";
      
    cout << cfg.time_offset << ";\t" << op_string << ";\t\t" << cfg.inode <<";\t" << addr << ";\t" << cfg.req_size << endl;
    cfg.time_offset += cfg.ia_time;
    
    prev_addr = addr;
  }
  
  // close
  cout << cfg.time_offset << ";\tclose;\t\t" << cfg.inode << endl;
  cfg.time_offset += cfg.ia_time;
  
  return 0;
}

void default_config(config &cfg)
{
  cfg.random_rate = 0;
  cfg.ran_aligned = 0;
  cfg.file_size = 0xFFFFFFFFFFFFFFFF;
  cfg.ia_time = 100.0;
  cfg.time_offset = 0.0;
  cfg.inode = 2;
  cfg.req_num = 10;
  cfg.create = 0;
  cfg.read_rate = 100;
  cfg.req_size = 4096;
  cfg.print_config = 0;
}

void print_config(config &cfg)
{
  cout << "cfg.random_rate = " << cfg.random_rate << endl;
  cout << "cfg.ran_aligned = " << cfg.ran_aligned << endl;
  cout << "cfg.file_size = " << cfg.file_size << endl;
  cout << "cfg.ia_time =" << cfg.ia_time << endl;
  cout << "cfg.time_offset = " << cfg.time_offset << endl;
  cout << "cfg.inode = " << cfg.inode << endl;
  cout << "cfg.req_num = " << cfg.req_num << endl;
  cout << "cfg.create = " << cfg.create << endl;
  cout << "cfg.read_rate = " << cfg.read_rate << endl;
  cout << "cfg.req_size = " << cfg.req_size << endl;
  cout << "cfg.print_config = " << cfg.print_config << endl;
}

void print_help_and_exit(int argc, char **argv)
{
  cout << "Usage : " << argv[0] << " -s <file size in bytes> [options]" << endl;
  cout << "Options are : " << endl;
  cout << " -r --ran-rate <int> random rate in percents" << endl;
  cout << " -a --ran-align align random requests on pages (4KB)" << endl;
  cout << " -s --file-size <long> file size in bytes" << endl;
  cout << " -i --ia <double> inter-arrival times in ms" << endl;
  cout << " -o --time-offset <double> arrival time for the first request" << endl;
  cout << " -d --inode <int> inode number for the accessed file" << endl;
  cout << " -n --requests-num <int> number of requests to generate" << endl;
  cout << " -c --create <int> create the file with name size in param" << endl;
  cout << " -l --read-rate <int> percentage of read requests" << endl;
  cout << " -p --request-size <int> request size" << endl;
  cout << " --print-config print setup" << endl;
  
        //{"ran-rate", required_argument, 0, 'r'},
      //{"ran-align", no_argument, 0, 'a'},
      //{"size", required_argument, 0, 's'},
      //{"ia", required_argument, 0, 'i'},
      //{"time-offset", required_argument, 0, 'o'},
      //{"inode", required_argument, 0, 'd'},
      //{"requests-num", required_argument, 0, 'n'},
      //{"create", required_argument, 0, 'c'},
      //{"read-rate", required_argument, 0, 'l'},
      //{"request-size", required_argument, 0, 'p'},
  
  exit(-1);
}
