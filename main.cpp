#include "httpd.h"
#include <errno.h>
#include <iostream>
#include <limits.h>
#include <stdlib.h>

using namespace std;

void usage(char *argv0)
{
    cerr << "Usage: " << argv0 << " listen_port docroot_dir" << endl;
}

int main(int argc, char *argv[])
{

    if (argc != 3 && argc != 4 && argc != 5) {
        usage(argv[0]);
        return 1;
    }

    long int port = strtol(argv[1], NULL, 10);

    if (errno == EINVAL || errno == ERANGE) {
        usage(argv[0]);
        return 2;
    }

    if (port <= 0 || port > USHRT_MAX) {
        cerr << "Invalid port: " << port << endl;
        return 3;
    }

    string doc_root = argv[2];

    if (argc == 4 || argc == 3)
    {
        if(argc == 4 && string(argv[3]) != "nopool"){
            cerr << "Invalid input: " << argv[3] << endl;
            return 4;
        }
        start_httpd(port, doc_root);
    }
    else if(argc == 5)
    {
        if(argv[3] != string("pool")){
            cerr << "Invalid input: " << argv[3] << endl;
            return 5;
        }
        start_httpd(port, doc_root, stoi(argv[4]));
    }

    return 0;
}
