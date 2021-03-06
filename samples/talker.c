/*! \file talker.c
 *  \brief This file is an example of cROS usage implementing a publisher and a service caller, which
 *         Can be used together with listener to prove the operation of message transmissions and service
 *         calls between two nodes.
 *
 *  It creates a publisher to the topic /chatter. Each 100ms the callback function callback_pub() is
 *  executed composing a string that is sent to the subscriber through a message of this topic.
 *  This node also implements a caller of the service /sum. Each 200ms the service is called:
 *  First the callback function callback_srv_add_two_ints is executed with call_resp_flag parameter set to 0,
 *  in this way two 64bit integers are generated by the function, which are sent to the service provider as
 *  service call parameters. The result of the service call (sum of the integers) is sent to the service
 *  caller and the callback function is executed again receiving the result with the call_resp_flag
 *  parameter set to 1.
 *  When the number of service calls or published messages is 10 the ROS node exits and the program finishes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "cros.h"

CrosNode *node; //! Pointer to object storing the ROS node. This object includes all the ROS node state variables
unsigned char exit_flag = 0; //! ROS node loop exit flag. When set to 1 the cRosNodeStart() function exits

// This callback will be invoked when it's our turn to publish a new message
static CallbackResponse callback_pub(cRosMessage *message, void* data_context)
{
  static int pub_count = 0;
  char buf[1024];
  // We need to index into the message structure and then assign to fields
  cRosMessageField *data_field;

  data_field = cRosMessageGetField(message, "data");
  if(data_field)
  {
    snprintf(buf, sizeof(buf), "hello world %d", pub_count);
    if(cRosMessageSetFieldValueString(data_field, buf) == 0)
    {
      ROS_INFO(node, "%s\n", buf);
    }
  }

  if(++pub_count > 10) exit_flag=1;

  return 0; // 0=success
}

static CallbackResponse callback_srv_add_two_ints(cRosMessage *request, cRosMessage *response, int call_resp_flag, void* context)
{
  static int call_count = 0;

  if(!call_resp_flag) // Check if this callback function has been called to provide the service call arguments or to collect the response
  {
    cRosMessageField *a_field = cRosMessageGetField(request, "a");
    cRosMessageField *b_field = cRosMessageGetField(request, "b");
    if(a_field != NULL && b_field != NULL)
    {
      a_field->data.as_int64=10;
      b_field->data.as_int64=call_count;
      ROS_INFO(node, "Service add 2 ints call arguments: {a: %lld, b: %lld}\b\n", (long long)a_field->data.as_int64, (long long)b_field->data.as_int64);
    }
  }
  else // Service call response available
  {
    cRosMessageField *sum_field = cRosMessageGetField(response, "sum");
    if(sum_field != NULL)
      ROS_INFO(node, "Service add 2 ints response: %lld (call_count: %i)\n", (long long)sum_field->data.as_int64, call_count++);
  }

  if(call_count > 10) exit_flag=1;
  return 0; // 0=success
}

int main(int argc, char **argv)
{
  // We need to tell our node where to find the .msg files that we'll be using
  char path[1024];
  char *node_name;
  cRosErrCodePack err_cod;
  int pubidx, svcidx;

  if(argc>1)
    node_name=argv[1];
  else
    node_name="/talker"; // Default node name if no command-line parameters are specified
  getcwd(path, sizeof(path));
  strncat(path, "/rosdb", sizeof(path) - strlen(path) - 1);
  // Create a new node and tell it to connect to roscore in the usual place
  node = cRosNodeCreate(node_name, "127.0.0.1", "127.0.0.1", 11311, path);

  // Create a publisher to topic /chatter of type "std_msgs/String" and request that the associated callback be invoked every 100ms (10Hz)
  err_cod = cRosApiRegisterPublisher(node, "/chatter","std_msgs/String", 100, callback_pub, NULL, NULL, &pubidx);
  if(err_cod != CROS_SUCCESS_ERR_PACK)
  {
    cRosPrintErrCodePack(err_cod, "cRosApiRegisterPublisher() failed; did you run this program one directory above 'rosdb'?");
    cRosNodeDestroy( node );
    return EXIT_FAILURE;
  }

  // Create a service caller named /sum of type "roscpp_tutorials/TwoInts" and request that the associated callback be invoked every 200ms (5Hz)
  err_cod = cRosApiRegisterServiceCaller(node,"/sum","roscpp_tutorials/TwoInts", 200, callback_srv_add_two_ints, NULL, NULL, 1, 1, &svcidx);
  if(err_cod != CROS_SUCCESS_ERR_PACK)
  {
    cRosPrintErrCodePack(err_cod, "cRosApiRegisterServiceCaller() failed; did you run this program one directory above 'rosdb'?");
    cRosNodeDestroy( node );
    return EXIT_FAILURE;
  }

  printf("Node TCPROS port: %i\n", node->tcpros_port);

  // Run the main loop
  struct timeval start_time, end_time;
  float elapsed_time;

  gettimeofday(&start_time, NULL);
  err_cod = cRosNodeStart( node, CROS_INFINITE_TIMEOUT, &exit_flag );
  gettimeofday(&end_time, NULL);

  elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0;    // sec to ms
  elapsed_time += (end_time.tv_usec - start_time.tv_usec) / 1000.0; // us to ms

  printf("Elapsed time: %.1fms\n", elapsed_time);
  if(err_cod != CROS_SUCCESS_ERR_PACK)
    cRosPrintErrCodePack(err_cod, "cRosNodeStart() returned an error code");


  // All done: free memory and unregister from ROS master
  err_cod=cRosNodeDestroy( node );
  if(err_cod != CROS_SUCCESS_ERR_PACK)
  {
    cRosPrintErrCodePack(err_cod, "cRosNodeDestroy() failed; Error unregistering from ROS master");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
