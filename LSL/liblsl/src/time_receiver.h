#ifndef TIME_RECEIVER_H
#define TIME_RECEIVER_H

#include "inlet_connection.h"
#include <lslboost/asio.hpp>
#include <lslboost/thread.hpp>
#include <lslboost/random.hpp>
#include <limits>

namespace boost = lslboost;
using boost::asio::ip::udp;
using boost::asio::deadline_timer;

namespace lsl {

	/// list of time estimates with error bounds
	typedef std::vector<std::pair<double,double> > estimate_list;
	/// pointer to a string
	typedef boost::shared_ptr<std::string> string_p;


	/// Internal class of an inlet that is responsible for retrieving the time-correction data of the inlet.
	/// The actual communication runs in an internal background thread, while the public function (time_correction()) waits for the thread to finish.
	/// The public function has an optional timeout after which it gives up, while the background thread 
	/// continues to do its job (so the next public-function call may succeed within the timeout).
	/// The background thread terminates only if the time_receiver is destroyed or the underlying connection is lost or shut down.
	class time_receiver {
	public:
		/// Construct a new time receiver for a given connection.
		time_receiver(inlet_connection &conn);

		/// Destructor. Stops the background activities.
		~time_receiver();

		/**
		* Retrieve an estimated time correction offset for the given stream.
		* The first call to this function takes several msec for an initial estimate, subsequent calls are instantaneous.
		* The correction offset is periodically re-estimated in the background (once every few sec.).
		* @timeout Timeout for first time-correction estimate.
		* @return The time correction estimate.
		* @throws timeout_error If the initial estimate times out.
		*/
		double time_correction(double timeout=2);

	private:
		/// The time reader / updater thread.
		void time_thread();

		/// Start a new multi-packet exchange for time estimation
		void start_time_estimation();

		/// Handler that gets called once the next time estimation shall be scheduled
		void next_estimate_scheduled(error_code err);

		/// Send the next packet in an exchange
		void send_next_packet(int packet_num);

		/// Handler that gets called once the sending of a packet has completed
		void handle_send_outcome(string_p msg_buffer, error_code err);

		/// Handler that gets called when the next packet shall be scheduled
		void next_packet_scheduled(int packet_num, error_code err);

		/// Request reception of the next time packet
		void receive_next_packet();

		/// Handler that gets called once reception of a time packet has completed
		void handle_receive_outcome(error_code err, std::size_t len);

		/// Handlers that gets called once the time estimation results shall be aggregated.
		void result_aggregation_scheduled(error_code err);

		/// function polled by the condition variable
		bool timeoffset_available() { return (timeoffset_ != std::numeric_limits<double>::max()) || conn_.lost(); }


		// the underlying connection
		inlet_connection &conn_;					// our connection
		
		// background reader thread and the data generated by it
		boost::thread time_thread_;					// updates time offset
		double timeoffset_;							// the current time offset (or NOT_ASSIGNED if not yet assigned)
		boost::mutex timeoffset_mut_;				// mutex to protect the time offset
		boost::condition_variable timeoffset_upd_;	// condition variable to indicate that an update for the time offset is available

		// data used internally by the background thread
		const api_config *cfg_;						// the configuration object
		boost::asio::io_service time_io_;			// an IO service for async time operations
		char recv_buffer_[16384];					// a buffer to hold inbound packet contents
		boost::random::mt19937 rng_;				// a random number generator
		udp::socket time_sock_;						// the socket through which the time thread communicates
		deadline_timer next_estimate_;				// schedule the next time estimate
		deadline_timer aggregate_results_;			// schedules result aggregation
		deadline_timer next_packet_;				// schedules the next packet transfer
		udp::endpoint remote_endpoint_;				// a dummy endpoint
		estimate_list estimates_;					// a vector of time estimates collected so far during the current exchange
		int current_wave_id_;						// an id for the current wave of time packets
	};

}

#endif

