
/**
 * Copyright (c) 2017-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

/**
 ** An execution controller to execute a workflow
 **/

#define GFLOP (1000.0 * 1000.0 * 1000.0)
#define MBYTE (1000.0 * 1000.0)
#define GBYTE (1000.0 * 1000.0 * 1000.0)

#include <iostream>
#include <wrench/services/storage/xrootd/Node.h>
#include "Controller.h"

WRENCH_LOG_CATEGORY(controller, "Log category for Controller");

namespace wrench {

    /**
     * @brief Constructor
     *
     * @param bare_metal_compute_service: a set of compute services available to run actions
     * @param storage_services: a set of storage services available to store files
     * @param hostname: the name of the host on which to start the WMS
     */
    Controller::Controller(std::shared_ptr<BareMetalComputeService> bare_metal_compute_service,
                           const std::shared_ptr<XRootD::Node> &root,
                           const std::string &hostname) : ExecutionController(hostname, "controller"),
                                                          bare_metal_compute_service(bare_metal_compute_service), root(root) {}

    /**
     * @brief main method of the Controller
     *
     * @return 0 on completion
     *
     * @throw std::runtime_error
     */
    int Controller::main() {

        /* Set the logging output to GREEN */
        TerminalOutput::setThisProcessLoggingColor(TerminalOutput::COLOR_GREEN);
        WRENCH_INFO("Controller starting");

        /* Create a files */
        std::vector<std::shared_ptr<DataFile>> files ={ 
			wrench::Simulation::addFile("file1", 1 * GBYTE),
			wrench::Simulation::addFile("file2", 1 * GBYTE),
			wrench::Simulation::addFile("file3", 100 * GBYTE),
			wrench::Simulation::addFile("file4", 1 * GBYTE),
		};
        
        wrench::Simulation::createFile(files[0], root->getChild(0)->getStorageServer());
		wrench::Simulation::createFile(files[1], root->getChild(1)->getStorageServer());
		wrench::Simulation::createFile(files[2], root->getChild(2)->getChild(0)->getStorageServer());
		wrench::Simulation::createFile(files[3], root->getChild(2)->getChild(1)->getStorageServer());

        /* Create a job manager so that we can create/submit jobs */
        auto job_manager = this->createJobManager();

        WRENCH_INFO("Creating a compound job with an assortment of file reads");
        auto job1 = job_manager->createCompoundJob("job1");
        auto fileread1 = job1->addFileReadAction("fileread1", files[0], root);
		auto fileread2 = job1->addFileReadAction("fileread2", files[1], root);
		auto fileread3 = job1->addFileReadAction("fileread3", files[2], root);
		auto fileread4 = job1->addFileReadAction("fileread4", files[3], root);

        auto compute = job1->addComputeAction("compute", 100 * GFLOP, 50 * MBYTE, 1, 3, wrench::ParallelModel::AMDAHL(0.8));
        job1->addActionDependency(fileread1, compute);
        job1->addActionDependency(fileread2, fileread3);
        job1->addActionDependency(fileread3, fileread4);
		job1->addActionDependency(fileread4, compute);
        
        
        job_manager->submitJob(job1, this->bare_metal_compute_service);
        this->waitForAndProcessNextEvent();

        WRENCH_INFO("Execution complete!");

        std::vector<std::shared_ptr<wrench::Action>> actions = {fileread1, fileread2, fileread3, fileread4,compute};
        for (auto const &a: actions) {
            printf("Action %s: %.2fs - %.2fs\n", a->getName().c_str(), a->getStartDate(), a->getEndDate());
        }

        return 0;
    }

    /**
     * @brief Process a compound job completion event
     *
     * @param event: the event
     */
    void Controller::processEventCompoundJobCompletion(std::shared_ptr<CompoundJobCompletedEvent> event) {
        /* Retrieve the job that this event is for */
        auto job = event->job;
        /* Print info about all actions in the job */
        WRENCH_INFO("Notified that compound job %s has completed:", job->getName().c_str());
    }
}// namespace wrench
