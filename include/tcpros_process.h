#ifndef _TCPROS_PROCESS_H_
#define _TCPROS_PROCESS_H_

#include "tcpip_socket.h"

/*! \defgroup tcpros_process TCPROS process */

/*! \addtogroup tcpros_process
 *  @{
 */

typedef enum
{
  TCPROS_PROCESS_STATE_IDLE,
  TCPROS_PROCESS_STATE_WAIT_FOR_CONNECTING,
  TCPROS_PROCESS_STATE_CONNECTING,
  TCPROS_PROCESS_STATE_READING_HEADER_SIZE,
  TCPROS_PROCESS_STATE_READING_HEADER,
  TCPROS_PROCESS_STATE_WRITING_HEADER,
  TCPROS_PROCESS_STATE_WAIT_FOR_WRITING,
  TCPROS_PROCESS_STATE_START_WRITING,
  TCPROS_PROCESS_STATE_READING_SIZE,
  TCPROS_PROCESS_STATE_READING,
  TCPROS_PROCESS_STATE_WRITING
} TcprosProcessState;

/*! \brief The TcprosProcess object represents a client or server connection used to manage
 *         peer to peer TCPROS connections between nodes. It is internally used to emulate the
 *         "precess descriptor" in a multi-task system (here used in a mono task system), including
 *         the process file descriptors (i.e., a socket), process memory and the state.
 *         NOTE: this is a cROS internal object, usually you don't need to use it.
 */
typedef struct TcprosProcess TcprosProcess;
struct TcprosProcess
{
  TcprosProcessState state;             //! The state of the process
  TcpIpSocket socket;                   //! The socket used for the TCPROS or RPCROS communication
  DynString topic;                      //! The name of the topic
  DynString service;                    //! The name of the service
  DynString type;                       //! The message/service type
  DynString servicerequest_type;        //! The service request type
  DynString serviceresponse_type;       //! The service response type
  DynString md5sum;                     //! The MD5 sum of the message type
  DynString caller_id;                  //! The name of subscriber or service caller
  unsigned char latching;               //! If 1, the publisher is sending latched messages. Otherwise
  unsigned char tcp_nodelay;            //! If 1, the publisher should set TCP_NODELAY on the socket, if possible. Otherwise 0
  unsigned char persistent;             //! If 1, the service connection should be kept open for multiple requests. Otherwise it should be 0
  DynBuffer packet;                     //! The incoming/outgoing TCPROS packet
  uint64_t last_change_time;            //! Last state change time (in ms)
  uint64_t wake_up_time_ms;             //! The time for the next automatic cycle (in msec, since the Epoch)
  int topic_idx;                        //! Index used to associate the process to a publisher or a subscribed
  int service_idx;                      //! Index used to associate the process to a service provider or a service client
  size_t left_to_recv;                  //! Remaining to receive
  uint8_t ok_byte;						          //! 'ok' byte send by a service provider in response to the last service request
  int probe;							              //! The current session is a probing one
  int sub_tcpros_port;                  //! Port (obtained from a publisher node) to which the process must connect
  char *sub_tcpros_host;                //! Host (obtained from a publisher node) to which the process must connect
  int send_msg_now;                     //! When different from 0 the publisher/caller should send the message in the buffer now (used for non-periodic sending)
};


/*! \brief Initialize an TcprosProcess object, starting to allocate memory
 *         and settings default values for the objects' fields
 *
 *  \param s Pointer to TcprosProcess object to be initialized
 */
void tcprosProcessInit( TcprosProcess *p );

/*! \brief Release all the internally allocated memory of an TcprosProcess object
 *
 *  \param s Pointer to TcprosProcess object to be released
 */
void tcprosProcessRelease( TcprosProcess *p );

/*! \brief Clear internal data of an TcprosProcess object (the internal memory IS NOT released)
 *
 *  \param s Pointer to TcprosProcess object
 *  \param fullreset true to do perform a full reset, false to just allow a new packet to be read
 */
void tcprosProcessClear( TcprosProcess *p , int fullreset );

/*! \brief Change the internal state of an TcprosProcess object, and update its timer
 *
 *  \param s Pointer to TcprosProcess object
 *  \param state The new state
 */
void tcprosProcessChangeState( TcprosProcess *p, TcprosProcessState state );

/*! @}*/

#endif
