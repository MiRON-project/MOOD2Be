#include <zmq.hpp>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono> 
#include "Intralogistic/json.hpp"

std::string generateVariantRequest(unsigned id, const std::string& value)
{
  nlohmann::json json;
  json["msg-type"] = "variant";
  json["id"] = id;
  auto& result = json["result"] = nlohmann::json({});
  result["value"] = value;
  return json.dump(1);
}

unsigned parseVariantResponse(const std::string& result_string) {
  nlohmann::json json = nlohmann::json::parse(result_string);
  unsigned msg_id = json["id"];
  std::string msg_type  = json["msg-type"];
  return msg_id;
}

void variantServer() {
  zmq::context_t context (1);

  //  Socket to talk to server
  std::cout << "Variant server…\n" << std::endl;
  zmq::socket_t request_socket(context, ZMQ_REP);
  zmq::socket_t reply_socket(context, ZMQ_PUB);
  
  request_socket.bind("tcp://*:8274");
  reply_socket.bind("tcp://*:8275");

  while(1)
  {
    zmq::message_t request;
    request_socket.recv(&request);

    std::string rcv_msg(static_cast<const char*>(request.data()), 
      request.size());
    auto id = parseVariantResponse(rcv_msg);

    zmq::message_t ack;
    request_socket.send(ack);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto reply_msg = generateVariantRequest(id, "two");
    zmq::message_t reply(reply_msg.size());
    memcpy(reply.data(), reply_msg.c_str(), reply_msg.size());
    reply_socket.send(reply);
  }
}

std::string generateSkillMoveRequest(unsigned id, const std::string& result) {
  nlohmann::json json;
  json["msg-type"] = "skill-result";
  json["id"] = id;
  auto& result_json = json["result"] = nlohmann::json({});
  result_json["result"] = result;
  result_json["result-value"] = "OK";
  return json.dump(1);
}

unsigned parseSkillMoveResponse(const std::string& result_string) {
  std::cout << "parsing skill...";
  nlohmann::json json = nlohmann::json::parse(result_string);
  unsigned msg_id = json["id"];
  std::string msg_type  = json["msg-type"];
  auto skill = json["skill"];
  auto in_attribute = skill["in-attribute"];
  std::cout << "x: " << in_attribute["x"] << ", y: " << in_attribute["y"] <<
    ", approachRadius: " << in_attribute["approachRadius"] << "\n";
  return msg_id;
}

void skillServer() {
  zmq::context_t context(1);

  //  Socket to talk to server
  std::cout << "Skill server…\n" << std::endl;
  zmq::socket_t request_socket(context, ZMQ_REP);
  zmq::socket_t reply_socket(context, ZMQ_PUB);

  request_socket.bind("tcp://*:5557");
  reply_socket.bind("tcp://*:5558");

  while(1)
  {
    std::cout << "\nWaiting request"<< std::endl;

    zmq::message_t request;
    request_socket.recv(&request);

    std::string rcv_msg(static_cast<const char*>(request.data()), 
      request.size());
    auto id = parseSkillMoveResponse(rcv_msg);
    std::cout << "\n---------- Received: "<< rcv_msg << std::endl;

    zmq::message_t ack;
    request_socket.send(ack);
    std::cout << "---------- ack sent: ----------\n" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto reply_msg = generateSkillMoveRequest(id, "SUCCESS");
    zmq::message_t reply(reply_msg.size());
    memcpy(reply.data(), reply_msg.c_str(), reply_msg.size());
    reply_socket.send(reply);
    std::cout << "-------- reply sent ----------" << std::endl;
  }
}

int main (int argc, char *argv[])
{
  std::thread variant(variantServer);
  std::thread skill(skillServer);
  variant.join();
  skill.join();
  return 0;
}
