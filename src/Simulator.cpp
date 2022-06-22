
/**
 * Copyright (c) 2017-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

/**
 ** This is the main function for a WRENCH simulator. The simulator takes
 ** a input an XML platform description file. It generates a workflow with
 ** a simple diamond structure, instantiates a few services on the platform, and
 ** starts an execution controller to execute the workflow using these services
 ** using a simple greedy algorithm.
 **/

#include <iostream>
#include <wrench-dev.h>
#include <wrench/services/storage/xrootd/XRootD.h>

#include <wrench/services/storage/xrootd/Node.h>
#include "Controller.h"

/**
 * @brief The Simulator's main function
 *
 * @param argc: argument count
 * @param argv: argument array
 * @return 0 on success, non-zero otherwise
 */
int main(int argc, char **argv) {

    /* Create a WRENCH simulation object */
    auto simulation = wrench::Simulation::createSimulation();

    /* Initialize the simulation */
    simulation->init(&argc, argv);

    /* Parsing of the command-line arguments */
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <xml platform file> [--log=controller.threshold=info | --wrench-full-log]" << std::endl;
        exit(1);
    }

    /* Instantiating the simulated platform */
    simulation->instantiatePlatform(argv[1]);

    /* Instantiate a storage service on the platform */
    //auto storage_service = simulation->add(new wrench::SimpleStorageService("StorageHost", {"/"}, {}, {}));

    /* Instantiate a bare-metal compute service on the platform */
    auto baremetal_service = simulation->add(new wrench::BareMetalComputeService("Fafard", {"Fafard"}, "", {}, {}));
	simulation->add(baremetal_service);

    /* Instantiate an execution controller */
    
	/*Construct XRootD tree that looks like
	             Ginette
	          /     |     \ 	
	 Timberlay Jacquellin  Jupiter
	                      /       \ 	
	                  Bourassa   Boivin
	*/
	wrench::XRootD::XRootD xrootdManager(simulation);
	std::shared_ptr<wrench::XRootD::Node> root=xrootdManager.createSupervisor("Ginette");
	std::shared_ptr<wrench::XRootD::Node> activeNode=xrootdManager.createStorageServer("Tremblay",{},{});
	root->addChild(activeNode);
	activeNode=xrootdManager.createStorageServer("Jacquelin",{},{});
	root->addChild(activeNode);
	activeNode=xrootdManager.createSupervisor("Jupiter");
	root->addChild(activeNode);
	
	std::shared_ptr<wrench::XRootD::Node> leaf=xrootdManager.createStorageServer("Bourassa",{},{});
	activeNode->addChild(leaf);
	leaf=xrootdManager.createStorageServer("Boivin",{},{});
	activeNode->addChild(leaf);
    /* Launch the simulation */
	auto controller = simulation->add(        new wrench::Controller(baremetal_service, root,&xrootdManager, "Fafard"));
    simulation->launch();

    return 0;
}
