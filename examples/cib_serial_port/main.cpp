#include<iostream>

#include<cib/cib.hpp>

//Services - basically like a function prototypes - callback function declaration!!
struct Send_Byte_t : public cib::callback_meta<uint8_t>
{};

struct Get_Byte_t : public cib::callback_meta<>
{};

struct Serial_Port_Init_t : public cib::callback_meta<>
{};

struct Run_t : public cib::callback_meta<>
{};

//components defintion!!
struct Core_Component_t
{
  constexpr static auto config = cib::config(

    cib::exports<Send_Byte_t>,
    cib::extend<Send_Byte_t>([](uint8_t byte){
      std::cout << "From CoreComponent: " << byte << std::endl;
    }),

    cib::extend<Get_Byte_t>([](){
      uint8_t readByte;
      std::cin >> readByte;

      cib::service<Send_Byte_t>(readByte); // This invokes both implementation of the SendByte - TxComponent and CoreComponent implementation
    }),
    
    cib::extend<Serial_Port_Init_t>([](){
      std::cout<<"Initialized\n";
    }),

    cib::extend<Run_t>([](){
      std::cout << "Type Something!!\n";
    })
  );
};

struct Tx_Component_t
{
  //tx - feature which implement a function which takes uint8_t argument! - Send_Byte_t service!!
  constexpr static auto tx = [](uint8_t byte) {
    std::cout << "From TxComponent: " << byte << std::endl;
  };

  constexpr static auto config = cib::config(
    cib::exports<Send_Byte_t>,
    cib::extend<Send_Byte_t>(tx)
    );
};

struct Export_Components_t
{
  constexpr static auto config = cib::exports<Get_Byte_t, Serial_Port_Init_t, Run_t>;
};

//Project definition with mutiple components
struct Dummy_Serial_Port_Project_t
{
  constexpr static auto config = cib::components<Core_Component_t, Tx_Component_t, Export_Components_t>;
};

static cib::nexus<Dummy_Serial_Port_Project_t> nexus{};

int main()
{
  nexus.init(); // This is critical!! otherwise accessing service from another service will result in the segmentation fault!!
  nexus.service<Serial_Port_Init_t>();

  while(true)
  {
    nexus.service<Run_t>();
    nexus.service<Get_Byte_t>();
  }
  return 0;
}
