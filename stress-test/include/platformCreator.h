#ifndef platformCreator_H
#define platformCreator_H
class PlatformCreator {

public:
    PlatformCreator(double link_bw) : link_bw(link_bw) {}

    void operator()() ;

    wrench::XRootD::XRootD metavisor;
	double density;
	int leafs;
	int files;
	double fileRedundancy;


};
#endif