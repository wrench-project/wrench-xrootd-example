#include "platformCreator.h"
#include <math.h>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;
namespace sg4 = simgrid::s4u;

void operator()() {
		std::srand(std::time(nullptr));
		//create graph
		vector<vector<int>> graph;
		vector<int> prevLevel=vector<int>(leafs);
		while(true){
			int level=round(prevLevel.size()/(64*density));
			if(level*64<prevLevel.size()){
				level=(prevLevel/64)+1;//I dont think this will ever happen, but it might due to rounding weirdness, this will just insure that there are always enough nodes
			}
			if(level<=1){
				graph.push_back(prevLevel);
				break;//this level is root, no need to have a level with 1 blank entry
			}
			vector<int> parrents=vector<int>(level);
			int unallocated=0;
			for(int i=0;i<prevLevel.size();i++){
				int n=std::rand()/((RAND_MAX + 1u)/level);
				if(parrents[n]<64){
					prevLevel[i]=n;
					parrents[n]++;
				}else{
					prevLevel[i]=-1;
					unallocated++;
				}
				
			}
			int j=0;
			for(int i=0;i<prevLevel.size()&&unallocated>0;i++){
				if(prevLevel[i]==-1){
					while(parrents[j]>=64){
						j=(j+1)%parrents.size();
					}
					parrents[j]++;
					prevLevel[i]=j;
				}
			}
			
			graph.push_back(prevLevel);
			prevLevel=vector<int>(level);
			
		}
	
        // Create the top-level zone
        auto zone = sg4::create_full_zone("AS0");
        // Create the WMSHost host with its disk
        auto wms_host = zone->create_host("WMSHost", "10Gf");
        wms_host->set_core_count(1);
        auto wms_host_disk = wms_host->create_disk("hard_drive",
                                                   "100MBps",
                                                   "100MBps");
        wms_host_disk->set_property("size", "5000GiB");
        wms_host_disk->set_property("mount", "/");

        // Create a ComputeHost
        auto compute_host = zone->create_host("ComputeHost", "1Gf");
        compute_host->set_core_count(10);
        compute_host->set_property("ram", "16GB");

        // Create three network links
        auto network_link = zone->create_link("network_link", link_bw)->set_latency("20us");
        auto loopback_WMSHost = zone->create_link("loopback_WMSHost", "1000EBps")->set_latency("0us");
        auto loopback_ComputeHost = zone->create_link("loopback_ComputeHost", "1000EBps")->set_latency("0us");

        // Add routes
        {
            sg4::LinkInRoute network_link_in_route{network_link};
            zone->add_route(compute_host->get_netpoint(),
                            wms_host->get_netpoint(),
                            nullptr,
                            nullptr,
                            {network_link_in_route});
        }
        {
            sg4::LinkInRoute network_link_in_route{loopback_WMSHost};
            zone->add_route(wms_host->get_netpoint(),
                            wms_host->get_netpoint(),
                            nullptr,
                            nullptr,
                            {network_link_in_route});
        }
        {
            sg4::LinkInRoute network_link_in_route{loopback_ComputeHost};
            zone->add_route(compute_host->get_netpoint(),
                            compute_host->get_netpoint(),
                            nullptr,
                            nullptr,
                            {network_link_in_route});
        }

        zone->seal();
    }