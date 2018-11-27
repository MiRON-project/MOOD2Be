#include <zmq.hpp>
#include <iostream>
#include <sstream>

int main (int argc, char *argv[])
{
    zmq::context_t context (1);

    //  Socket to talk to server
    std::cout << "Collecting updates from weather server…\n" << std::endl;
    zmq::socket_t request_socket (context, ZMQ_REP);
    zmq::socket_t reply_socket (context, ZMQ_PUB);

    request_socket.bind("tcp://*:5557");
    reply_socket.bind("tcp://*:5558");

    while(1)
    {
        std::cout << "\nWaiting request"<< std::endl;

        zmq::message_t request;
        request_socket.recv( &request );

        std::string rcv_msg( static_cast<const char*>(request.data()), request.size() );
        std::cout << "\n---------- Received: "<< rcv_msg << std::endl;

        zmq::message_t ack;
        request_socket.send( ack );
        std::cout << "---------- ack sent: ----------\n" << std::endl;

        zmq::message_t reply(5);
        memcpy( reply.data(), "HELLO", 5);

        reply_socket.send(reply);
        std::cout << "-------- reply HELLO sent ----------" << std::endl;
    }
    return 0;
}
