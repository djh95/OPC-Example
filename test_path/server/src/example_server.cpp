/// @brief OPC UA Server main.
/// @license GNU LGPL
///
/// Distributed under the GNU LGPL License
/// (See accompanying file LICENSE or copy at
/// http://www.gnu.org/licenses/lgpl.html)
///
#include <iostream>
#include <algorithm>
#include <time.h>

#include <thread>
#include <chrono>

#include <opc/ua/node.h>
#include <opc/ua/subscription.h>
#include <opc/ua/server/server.h>




using namespace OpcUa;

class SubClient : public SubscriptionHandler
{
  void DataChange(uint32_t handle, const Node & node, const Variant & val, AttributeId attr) override
  {
    std::cout << "Received DataChange event for Node " << node << std::endl;
  }
};

std::vector<OpcUa::Variant> MyMethod(NodeId context, std::vector<OpcUa::Variant> arguments)
{
  std::cout << "MyMethod called! " << std::endl;
  std::vector<OpcUa::Variant> result;
  result.push_back(Variant(static_cast<uint8_t>(0)));
  return result;
}

void showNode(Node* n, std::shared_ptr<spdlog::logger> logger, bool showType=false, std::string prefix="", std::string suffix="")
{
  QualifiedName qn = n->GetBrowseName();
  Variant val = n->GetValue();
  if(val.IsNul())  
  {
    logger->info(prefix + "{1}, Name={0}" + suffix, qn.Name, *n);
    //logger->info(prefix + "{1}, Name={0}, Value is {2}({3})," + suffix, qn.Name, *n, n->GetValue(), n->GetValue().Type());
  }
  else{
    std::string typeS;
    switch (val.Type()) {
      case OpcUa::VariantType::BOOLEAN:
        typeS = "BOOLEAN";
        break;
      case OpcUa::VariantType::STRING:
        typeS = "STRING";
        break;
      case OpcUa::VariantType::QUALIFIED_NAME:
        typeS = "QUALIFIED_NAME";
        break;
      case OpcUa::VariantType::LOCALIZED_TEXT:
        typeS = "LOCALIZED_TEXT";		
        break;
      case OpcUa::VariantType::BYTE:
        typeS = "BYTE";
        break;
      case OpcUa::VariantType::UINT16:
        typeS = "UINT16";
        break;
      case OpcUa::VariantType::UINT32:
        typeS = "UINT32";
        break;
      case OpcUa::VariantType::UINT64:
        typeS = "UINT64";
        break;
      case OpcUa::VariantType::INT16:
        typeS = "INT16";
        break;
      case OpcUa::VariantType::INT32:
        typeS = "INT32";
        break;
      case OpcUa::VariantType::INT64:
        typeS = "INT64";
        break;
      case OpcUa::VariantType::DOUBLE:
        typeS = "DOUBLE";
        break;}

      //logger->info(prefix + "{1}, Name={0}" + suffix, qn.Name, *n);
      logger->info(prefix + "{1}, Name={0}, Value is {2} ({3})," + suffix, qn.Name, *n, val.ToString(), typeS);
      //logger->info(prefix + "{1}, Name={0}, Value type is ({3})," + suffix, qn.Name, *n, n->GetValue(), n->GetValue().Type());
      //logger->info(prefix + "{1}, Name={0}, Value is {2}," + suffix, qn.Name, *n, n->GetValue(), n->GetValue().Type());
      //logger->info(prefix + "{1}, Name={0}, Value is {2}({3})," + suffix, qn.Name, *n, n->GetValue(), n->GetValue().Type());
    }
}

void showChildren(Node* n, std::shared_ptr<spdlog::logger> logger)
{
  showNode(n, logger, false, "Children of ", ":");
  for(Node child : n->GetChildren())
  {
    showNode(&child, logger, true, "  ");
  }
}


void AddVariables(Node* n, uint32_t idx, std::vector<std::string> name, std::vector<Variant> type)
{
  for(int i=0; i<name.size(); i++)
  {
    n->AddVariable(idx, name[i], type[i]);
    
  }
}

void AddVariables(Node* n, uint32_t idx, std::vector<std::string> name, Variant v= Variant(0))
{
  for( std::string s : name)
  {
    n->AddVariable(idx, s, v);
  }
}

void AddObjects(Node* n, uint32_t idx, std::vector<std::string> name)
{
  for( std::string s : name)
  {
    n->AddObject(idx, s);
  }
}

void RunServer()
{
  //First setup our server
  auto logger = spdlog::stderr_color_mt("server");
  OpcUa::UaServer server(logger);
  server.SetEndpoint("opc.tcp://127.0.0.1:48401/");
  //server.SetServerURI("urn://exampleserver.freeopcua.github.io");
  server.Start();

  //then register our server namespace and get its index in server
  uint32_t idx2 = server.RegisterNamespace("http://test2");
  uint32_t idx3 = server.RegisterNamespace("http://test3");
  uint32_t idx4 = server.RegisterNamespace("http://test4");
  
  //Create our address space using different methods
  Node objects = server.GetObjectsNode();

  //Add a custom object with specific nodeid
  NodeId nid2(99, idx2);
  NodeId nid3(2009, idx3);
  NodeId nid4(2010, idx4);
  QualifiedName qn_device("DeviceSet", idx2);
  QualifiedName qn("DeviceSet", idx2);
  
  Node deviceSet2 = objects.AddObject(idx2, "DeviceSet");
  Node newobject = objects.AddObject(nid2,qn); 
  //Add a variable and a property with auto-generated nodeid to our custom object
  Node myvar = newobject.AddVariable(idx2, "MyVariable", Variant(8));
  Node myprop = newobject.AddVariable(idx2, "MyProperty", Variant(8.8));
  Node mymethod = newobject.AddMethod(idx2, "MyMethod", MyMethod);
  
  //{"Objects","2:DeviceSet","4:CPX-E-CEC-C1-PN","4:Resources","4:Application","3:GlobalVars","4:G","4:Basic"}
  Node CPX4 =  deviceSet2.AddObject(idx4, "CPX-E-CEC-C1-PN"); 
  Node Resources4 =  CPX4.AddObject(idx4, "Resources");
  Node Application4 =  Resources4.AddObject(idx4, "Application");
  Node GlobalVars3 =  Application4.AddObject(idx3, "GlobalVars");
  Node G4 =  GlobalVars3.AddObject(idx4, "G");
  Node Basic4 =  G4.AddObject(idx4, "Basic");
  Node In4 = G4.AddObject(idx4, "In");
  
  Node P4Basic = Basic4.AddObject(idx4, "p"); 
  Node P4In = In4.AddObject(idx4, "p");

  showChildren(&Basic4, logger);
  showChildren(&In4,logger); 

  //"4:ActionId" "4:BarCode" "4:Data" "4:Error" "4:SlideCnt"
  std::vector<std::string> nameSet({"ActionId", "BarCode", /*"Data",,*/ "Error", "SlideCnt"});
  std::vector<Variant> typeSet = {uint16_t(0), uint16_t(0), uint8_t(0), uint16_t(0)};

  AddVariables(&P4Basic, idx4, nameSet, typeSet);
  AddVariables(&P4In, idx4, nameSet, typeSet);

  showChildren(&P4Basic, logger);
  showChildren(&P4In, logger);
  
  Node Data4P4Basic = P4Basic.AddObject(idx4, "Data");
  AddVariables(&Data4P4Basic, idx4, {"payload1", "payload2"}, uint16_t(0));
  showChildren(&Data4P4Basic, logger);
  
  Node Data4P4In = P4In.AddObject(idx4, "Data");
  AddVariables(&Data4P4In, idx4, {"payload1", "payload2"}, uint16_t(0));
  showChildren(&Data4P4In, logger);

  //
  Node Status4P4Basic = P4Basic.AddObject(idx4, "Status");
  Node Status4P4In = P4In.AddObject(idx4, "Status");

  //"4:Busy", "4:Enable", "4:Error", "4:Ready"
  nameSet = {"4:Busy", "4:Enable", "4:Error", "4:Ready"};
  AddVariables(&Status4P4Basic, idx4, nameSet, bool(0));
  showChildren(&Status4P4Basic, logger);
  AddVariables(&Status4P4In, idx4, nameSet, bool(0));
  showChildren(&Status4P4In, logger);
 
  //test
  /*
  std::vector<std::string> BASIC_NODE_PATH({"Objects",
					"2:DeviceSet",
					"4:CPX-E-CEC-C1-PN",
					"4:Resources",
					"4:Application",
					"3:GlobalVars",
					"4:G",
					"4:Basic"});
  Node Basic4New = server.GetRootNode().GetChild(BASIC_NODE_PATH);
  logger->info("Basic4 node is: {}", Basic4New);
  
  std::vector<std::string> IN_NODE_PATH({"Objects",
                                         "2:DeviceSet",
                                         "4:CPX-E-CEC-C1-PN",
                                         "4:Resources",
                                         "4:Application",
                                         "3:GlobalVars",
                                         "4:G",
                                         "4:In"}); 
 
  Node In4New = server.GetRootNode().GetChild(IN_NODE_PATH);
  logger->info("In4 node is: {}", In4New);
   
  std::vector<std::string> path_objects({"Objects"});
  std::vector<std::string> path_deviceSet({"Objects","2:DeviceSet"});
  logger->info("Objects node is: {}", server.GetRootNode().GetChild(path_objects));
  logger->info("deviceSet node is: {}", server.GetRootNode().GetChild(path_deviceSet));
  */


  //Uncomment following to subscribe to datachange events inside server
  /*
  SubClient clt;
  std::unique_ptr<Subscription> sub = server.CreateSubscription(100, clt);
  sub->SubscribeDataChange(myvar);
  */



  //Now write values to address space and send events so clients can have some fun
  uint32_t counter = 0;
  myvar.SetValue(Variant(counter)); //will change value and trigger datachange event

  //Create event
  server.EnableEventNotification();
  Event ev(ObjectId::BaseEventType); //you should create your own type
  ev.Severity = 2;
  ev.SourceNode = ObjectId::Server;
  ev.SourceName = "Event from FreeOpcUA";
  ev.Time = DateTime::Current();


  logger->info("Ctrl-C to exit");

  for (;;)
    {
      myvar.SetValue(Variant(++counter)); //will change value and trigger datachange event
      std::stringstream ss;
      ss << "This is event number: " << counter;
      ev.Message = LocalizedText(ss.str());
      server.TriggerEvent(ev);
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

  server.Stop();
}

int main(int argc, char ** argv)
{
  try
    {
      RunServer();
    }

  catch (const std::exception & exc)
    {
      std::cout << exc.what() << std::endl;
    }

  return 0;
}

