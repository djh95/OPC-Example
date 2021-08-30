#include <opc/ua/client/client.h>
#include <opc/ua/node.h>
#include <opc/ua/subscription.h>

#include <opc/common/logger.h>

#include <iostream>
#include <stdexcept>
#include <thread>

using namespace OpcUa;

class SubClient : public SubscriptionHandler
{
  void DataChange(uint32_t handle, const Node & node, const Variant & val, AttributeId attr) override
  {
    std::cout << "Received DataChange event, value of Node " << node << " is now: "  << val.ToString() << std::endl;
  }
};

int main(int argc, char ** argv)
{
  auto logger = spdlog::stderr_color_mt("client");
  //logger->set_level(spdlog::level::debug);
  try
    {
      std::string endpoint = "opc.tcp://127.0.0.1:48401/";

      if (argc > 1)
        { endpoint = argv[1]; }

      logger->info("Connecting to: {}", endpoint);

      OpcUa::UaClient client(logger);
      client.Connect(endpoint);

      //get Root node on server
      OpcUa::Node root = client.GetRootNode();
      logger->info("Root node is: {}", root);

      //get and browse Objects node
      logger->info("Children of objects node are:");
      Node objects = client.GetObjectsNode();

      for (OpcUa::Node node : objects.GetChildren())
        { logger->info("    {}", node); }

      //get a node from standard namespace using objectId
      logger->info("NamespaceArray is:");
      OpcUa::Node nsnode = client.GetNode(ObjectId::Server_NamespaceArray);
      OpcUa::Variant ns = nsnode.GetValue();

      for (std::string d : ns.As<std::vector<std::string>>())
        { logger->info("    {}", d); }

      OpcUa::Node myvar;
      OpcUa::Node myobject;
      OpcUa::Node mymethod;

      //Initialize Node myvar:

      //Get namespace index we are interested in

      // From freeOpcUa Server:
      uint32_t idx = client.GetNamespaceIndex("http://test.bs");
      ////Get Node using path (BrowsePathToNodeId call)
      std::vector<std::string> varpath({ std::to_string(idx) + ":NewObject", "MyVariable" });
      myvar = objects.GetChild(varpath);

      std::vector<std::string> methodpath({ std::to_string(idx) + ":NewObject" });
      myobject = objects.GetChild(methodpath);

      methodpath = { std::to_string(idx) + ":NewObject", "MyMethod" };
      mymethod = objects.GetChild(methodpath);

      std::vector<OpcUa::Variant> arguments;
      arguments.push_back(static_cast<uint8_t>(0));
      myobject.CallMethod(mymethod.GetId(), arguments);

      // Example data from Prosys server:
      //std::vector<std::string> varpath({"Objects", "5:Simulation", "5:Random1"});
      //myvar = root.GetChild(varpath);

      //Example from any UA server, standard dynamic variable node:
      std::vector<std::string> varpath2{ "Objects", "Server", "ServerStatus", "CurrentTime" };
      OpcUa::Node myvar2  = root.GetChild(varpath2);

      logger->info("Node newobject is: {}", myobject);                             
      logger->info("Node myvar is: {}", myvar);                                     
      logger->info("Node myvar2 is: {}", myvar2);                                   
      logger->info("Node mymethod is: {}", mymethod); 

      //Subscription
      SubClient sclt;
      Subscription::SharedPtr sub = client.CreateSubscription(100, sclt);
      uint32_t handle = sub->SubscribeDataChange(myvar);
      uint32_t handle2 = sub->SubscribeDataChange(myvar2); 
      logger->info("Got sub handle: {}", handle);
      logger->info("Got sub handle2: {}, sleeping 20 seconds", handle);
      std::this_thread::sleep_for(std::chrono::seconds(20));

      logger->info("Disconnecting");
      client.Disconnect();
      logger->flush();
      return 0;
    }

  catch (const std::exception & exc)
    {
      logger->error("Error: {}", exc.what());
    }

  catch (...)
    {
      logger->error("Unknown error.");
    }

  return -1;
}
