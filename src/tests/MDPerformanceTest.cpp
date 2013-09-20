#include "core/global.h"
#include "messagedirector/MessageDirector.h"
#include <boost/random.hpp>
#include <ctime>

LogCategory mdperf_log("PerfTestMD", "Performance Test - MessageDirector");

#define MD_PERF_NUM_CHANNELS 100000
#define MD_PERF_NUM_DEST_CHANNELS 1
#define MD_PERF_NUM_PARTICIPANTS 10
#define MD_PERF_TIME 5.0
#define MD_PERF_DATASIZE 70 //must be atleast 1+(MD_PERF_NUM_DEST_CHANNELS*8)
uint8_t *data = NULL;

boost::random::mt19937_64 gen;

class MDPerformanceParticipant : public MDParticipantInterface
{
	public:
		MDPerformanceParticipant() : MDParticipantInterface(), num_messages(0)
		{
			boost::random::uniform_int_distribution<uint64_t> dist(0,0xFFFFFFFFFFFFFFFF);
			for(uint32_t i = 0; i < MD_PERF_NUM_CHANNELS; ++i)
			{
				subscribe_channel(dist(gen));
			}
		}

		virtual void handle_datagram(Datagram &dg, DatagramIterator &dgi)
		{
		}

		void spam()
		{
			Datagram dg(data, MD_PERF_DATASIZE);
			send(dg);
			num_messages++;
		}

		uint32_t num_messages;
};

class MDPerformanceTest
{
	public:
		MDPerformanceTest()
		{
			mdperf_log.info() << "Starting perf test..." << std::endl;
			mdperf_log.info() << "Creating random data..." << std::endl;
			data = new uint8_t[MD_PERF_DATASIZE];
			data[0] = MD_PERF_NUM_DEST_CHANNELS;
			for(uint32_t i = 1; i < MD_PERF_DATASIZE; ++i)
			{
				data[i] = rand()%256;
			}
			mdperf_log.info() << "Creating MDPerformanceParticipants" << std::endl;
			MDPerformanceParticipant **participants = new MDPerformanceParticipant*[MD_PERF_NUM_PARTICIPANTS];
			for(uint32_t i = 0; i < MD_PERF_NUM_PARTICIPANTS; ++i)
			{
				participants[i] = new MDPerformanceParticipant;
			}

			mdperf_log.info() << "Starting  test..." << std::endl;
			clock_t startTime = clock();
			while((clock()-startTime)/CLOCKS_PER_SEC < MD_PERF_TIME)
			{
				for(uint32_t i = 0; i < MD_PERF_NUM_PARTICIPANTS; ++i)
				{
					participants[i]->spam();
				}
			}
			mdperf_log.info() << "Test over. Averaging messages..." << std::endl;
			double num_messages = 0;
			for(uint32_t i = 0; i < MD_PERF_NUM_PARTICIPANTS; ++i)
			{
				num_messages += double(participants[i]->num_messages)/double(MD_PERF_NUM_PARTICIPANTS);
			}

			mdperf_log.info() << "An average of " << num_messages << " messages were processed. "
				"this comes out to be " << num_messages/MD_PERF_TIME << " messages/second" << std::endl;
			mdperf_log.info() << "Cleaning up..." << std::endl;
			for(uint32_t i = 0; i < MD_PERF_NUM_PARTICIPANTS; ++i)
			{
				delete participants[i];
			}
			delete [] participants;
			delete [] data;
		}
};

MDPerformanceTest perftest_md;
