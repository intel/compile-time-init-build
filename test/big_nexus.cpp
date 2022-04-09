#include <cib/cib.hpp>


using cib::detail::int_;


template<int Id>
[[maybe_unused]] static bool is_callback_invoked = false;

template<int Id, typename... Args>
struct TestCallback : public cib::callback_meta<Args...> {};

template<int Id>
struct TestComponent {
    constexpr static auto offset = Id * 100;

    constexpr static auto config =
        cib::config(
            cib::exports<
                TestCallback<0 + offset>,
                TestCallback<1 + offset>,
                TestCallback<2 + offset>,
                TestCallback<3 + offset>,
                TestCallback<4 + offset>,
                TestCallback<5 + offset>,
                TestCallback<6 + offset>,
                TestCallback<7 + offset>,
                TestCallback<8 + offset>,
                TestCallback<9 + offset>//,
//                TestCallback<10 + offset>,
//                TestCallback<11 + offset>,
//                TestCallback<12 + offset>,
//                TestCallback<13 + offset>,
//                TestCallback<14 + offset>,
//                TestCallback<15 + offset>,
//                TestCallback<16 + offset>,
//                TestCallback<17 + offset>,
//                TestCallback<18 + offset>,
//                TestCallback<19 + offset>
            >,

            cib::extend<TestCallback<offset>>([](){
                is_callback_invoked<offset> = true;
            }),

            cib::extend<TestCallback<offset>>([](){
                is_callback_invoked<offset> = true;
            }),

            cib::extend<TestCallback<offset>>([](){
                is_callback_invoked<offset> = true;
            }),

            cib::extend<TestCallback<offset>>([](){
                is_callback_invoked<offset> = true;
            }),

            cib::extend<TestCallback<offset>>([](){
                is_callback_invoked<offset> = true;
            })//,
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            }),
//
//            cib::extend<TestCallback<offset>>([](){
//                is_callback_invoked<offset> = true;
//            })
        );
};

struct BigConfig {
    constexpr static auto config =
        cib::config(
            cib::components<
                cib::args<>,
                TestComponent<0>,
                TestComponent<1>,
                TestComponent<2>,
                TestComponent<3>,
                TestComponent<4>,
                TestComponent<5>,
                TestComponent<6>,
                TestComponent<7>,
                TestComponent<8>,
                TestComponent<9>
//                TestComponent<10>,
//                TestComponent<11>,
//                TestComponent<12>,
//                TestComponent<13>,
//                TestComponent<14>,
//                TestComponent<15>,
//                TestComponent<16>,
//                TestComponent<17>,
//                TestComponent<18>,
//                TestComponent<19>,
//                TestComponent<20>,
//                TestComponent<21>,
//                TestComponent<22>,
//                TestComponent<23>,
//                TestComponent<24>,
//                TestComponent<25>,
//                TestComponent<26>,
//                TestComponent<27>,
//                TestComponent<28>,
//                TestComponent<29>,
//                TestComponent<30>,
//                TestComponent<31>,
//                TestComponent<32>,
//                TestComponent<33>,
//                TestComponent<34>,
//                TestComponent<35>,
//                TestComponent<36>,
//                TestComponent<37>,
//                TestComponent<38>,
//                TestComponent<39>,
//                TestComponent<40>,
//                TestComponent<41>,
//                TestComponent<42>,
//                TestComponent<43>,
//                TestComponent<44>,
//                TestComponent<45>,
//                TestComponent<46>,
//                TestComponent<47>,
//                TestComponent<48>,
//                TestComponent<49>
            >
        );
};

void make_nexus() {
    cib::nexus<BigConfig> nexus{};
    nexus.init();
}
