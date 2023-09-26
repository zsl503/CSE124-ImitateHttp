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

    if (argc != 3 && argc != 4) {
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

    if (argc == 4)
    {
        int thread_num = stoi(argv[3]);
        start_httpd(port, doc_root, thread_num);
    }
    else
    {
        start_httpd(port, doc_root);
    }

    return 0;
}
