#include<cib/cib.hpp>

#include<iostream>

//Services - basically like a function prototypes - callback function declaration!!
struct send_byte_t : public cib::callback_meta<uint8_t>
{};

struct get_byte_t : public cib::callback_meta<>
{};

struct serial_port_init_t : public cib::callback_meta<>
{};

struct run_t : public cib::callback_meta<>
{};

//components defintion!!
struct core_component_t
{
  constexpr static auto config = cib::config(

    cib::exports<send_byte_t>,
    cib::extend<send_byte_t>([](uint8_t byte){
      std::cout << "From CoreComponent: " << byte << std::endl;
    }),

    cib::extend<get_byte_t>([](){
      uint8_t readByte;
      std::cin >> readByte;

      cib::service<send_byte_t>(readByte); // This invokes both implementation of the SendByte - TxComponent and CoreComponent implementation
    }),
    
    cib::extend<serial_port_init_t>([](){
      std::cout<<"Initialized\n";
    }),

    cib::extend<run_t>([](){
      std::cout << "Type Something!!\n";
    })
  );
};

struct tx_component_t
{
  //tx - feature which implement a function which takes uint8_t argument! - send_byte_t service!!
  constexpr static auto tx = [](uint8_t byte) {
    std::cout << "From TxComponent: " << byte << std::endl;
  };

  constexpr static auto config = cib::config(
    cib::exports<send_byte_t>,
    cib::extend<send_byte_t>(tx)
    );
};

struct export_components_t
{
  constexpr static auto config = cib::exports<get_byte_t, serial_port_init_t, run_t>;
};

//Project definition with mutiple components
struct dummy_serial_port_project_t
{
  constexpr static auto config = cib::components<core_component_t, tx_component_t, export_components_t>;
};

static cib::nexus<dummy_serial_port_project_t> nexus{};

int main()
{
  nexus.init(); // This is critical!! otherwise accessing service from another service will result in the segmentation fault!!
  nexus.service<serial_port_init_t>();

  while(true)
  {
    nexus.service<run_t>();
    nexus.service<get_byte_t>();
  }
  return 0;
}
