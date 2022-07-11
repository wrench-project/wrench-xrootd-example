#ifndef platformCreator_H
#define platformCreator_H

#include <wrench/services/storage/xrootd/XRootD.h>
#include <wrench-dev.h>
class PlatformCreator {

public:
    PlatformCreator(wrench::XRootD::XRootD metavisor,
		double density,
		int leafs,
		int files,
		double fileRedundancy) : metavisor(metavisor),density(density),leafs(leafs),files(files),fileRedundancy(fileRedundancy) {}

    void operator()() ;

    wrench::XRootD::XRootD metavisor;
	double density;
	int leafs;
	int files;
	double fileRedundancy;
	//created in init
	std::shared_ptr<wrench::XRootD::Node> root;
	vector<std::shared_ptr<wrench::DataFile>> filePointers;
};
#endif
