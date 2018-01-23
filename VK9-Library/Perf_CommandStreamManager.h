/*
Copyright(c) 2018 Christopher Joseph Dean Schaefer

This software is provided 'as-is', without any express or implied
warranty.In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions :

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software.If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include <atomic>
#include <thread>
#include <boost/lockfree/queue.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

#include "Perf_RenderManager.h"

#ifndef COMMANDSTREAMMANAGER_H
#define COMMANDSTREAMMANAGER_H

enum WorkItemType
{
	Instance_Create
	,Instance_GetAdapterIdentifier
	,Instance_GetDeviceCaps
	,Instance_Destroy
	,Device_Create
	,Device_BeginScene
	,Device_Destroy
};

struct WorkItem
{
	WorkItemType WorkItemType;
	size_t Id;
	void* Argument1;
	void* Argument2;
	void* Argument3;
};

struct CommandStreamManager;

void ProcessQueue(CommandStreamManager* commandStreamManager);

struct CommandStreamManager
{ 	
	boost::program_options::variables_map mOptions;
	boost::program_options::options_description mOptionDescriptions;
	std::thread mWorkerThread;
	RenderManager mRenderManager;
	boost::lockfree::queue<WorkItem> mWorkItems;
	std::atomic_bool IsRunning = 1;
	std::atomic_bool IsBusy = 0;

	CommandStreamManager();
	~CommandStreamManager();

	size_t RequestWork(const WorkItem& workItem);
	size_t RequestWorkAndWait(const WorkItem& workItem);
};

#endif // COMMANDSTREAMMANAGER_H