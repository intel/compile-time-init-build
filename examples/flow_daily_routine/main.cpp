#include <cib/cib.hpp>
#include <flow/flow.hpp>

#include <chrono>
#include <iostream>

// In this example we shall see the Daily routine of a individual person
/*
  Following actions to be done in order in a day by a person
    + WakeUp
    + GetReadyForExercise
    + Exercise
    + TakeBath
    + GetReadyToWork
    + HaveBreakfast
    + GoToOffice
    + ReturnHome
    + GetReadyForExercise
    + Exercise
    + TakeBath
    + RelaxForSometime
    + HaveDinner
    + GoToBed

  Morning_Routine_ts are:  WakeUp, GetReadyForExercise, Exercise, TakeBath,
  GetReadyToWork, HaveBreakfast, GoToOffice
  Evening_Routine_ts are:  ReturnHome, GetReadyToExercise, Exercise, TakeBath,
  RelaxForSometime, HaveDinner, GoToBed

  These things should be done in order:
  Flow part of the CIB library will help us to achieve the same
  Following concents in the flow can be used to achieve the same
    + Flow::Service<>
    + Flow::Action<>
    + Flow::MileStone<>
    + operator >>
    + operator &&

  Flow::Service ==> 2 or more Actions and/or MileStones
  >>            ==> used to link actions/milestones to be executed in order.
  &&            ==> used to link actions/milestones without any order.

  Note::
    services can be defined using
      + cib::callback_meta<>  - can have only one function pointer assigned to
  execute.
      + flow::service<>       - set of actions/milestones which are to be
  executed in specific order. So basically sequential execution of multiple
  function pointers.

    both services are different.

  Flow in short:
    + Flow can be treated like a Directed Acyclic graph.
    + Any component can implement actions/milestones.
    + Any component can mention a sequence of execution of actions/milestones.
    + Services can be executed with flow::run() in an action.
    + That action should extend the MainLoop of the cib library.
*/

/*
  Service definitions
*/
// "morning_routine_log_t" - used during the logging
// struct morning_routine_log_t
//     : public flow::service<"morning_routine_log_t"> {};
struct morning_routine_t : public flow::service<> {};

// struct evening_routine_t : public
// flow::service<"evening_routine_t">
struct evening_routine_t : public flow::service<> {};

/*
  Component definitions
*/
struct self_care_component_t {
    /*
      Actions: WAKE_UP, EXERCISE, GO_TO_BED, RELAX, TAKE_BATH
    */
    constexpr static auto WAKE_UP = flow::action<"WakeUp">(
        []() { std::cout << "Wake up at 6:00 AM" << std::endl; });

    constexpr static auto EXERCISE = flow::action<"Exercise">(
        []() { std::cout << "Gym activities" << std::endl; });

    constexpr static auto GO_TO_BED = flow::action<"GoToBed">(
        []() { std::cout << "Go to bed at 10:00 PM" << std::endl; });

    constexpr static auto RELAX = flow::action<"Relax">(
        []() { std::cout << "Have relax before Dinner" << std::endl; });

    constexpr static auto TAKE_BATH = flow::action<"TakeBath">(
        []() { std::cout << "Take a bath" << std::endl; });

    // Extend flow services
    constexpr static auto config = cib::config(

        cib::extend<morning_routine_t>(self_care_component_t::WAKE_UP >>
                                       self_care_component_t::EXERCISE >>
                                       self_care_component_t::TAKE_BATH),

        cib::extend<evening_routine_t>(self_care_component_t::EXERCISE >>
                                       self_care_component_t::TAKE_BATH >>
                                       self_care_component_t::RELAX >>
                                       self_care_component_t::GO_TO_BED));
};

struct food_component_t {
    /*
      Actions: BREAKFAST, DINNER
    */
    constexpr static auto BREAKFAST = flow::action<"Breakfast">(
        []() { std::cout << "Have healthy breakfast" << std::endl; });

    constexpr static auto DINNER = flow::action<"Dinner">(
        []() { std::cout << "Have early dinner" << std::endl; });

    // Extend flow services
    constexpr static auto config = cib::config(

        cib::extend<morning_routine_t>(self_care_component_t::TAKE_BATH >>
                                       food_component_t::BREAKFAST),

        cib::extend<evening_routine_t>(self_care_component_t::RELAX >>
                                       food_component_t::DINNER >>
                                       self_care_component_t::GO_TO_BED));
};

struct dress_up_component_t {
    /*
      Actions: GET_READY_FOR_EXERCISE, GET_READY_TO_WORK
    */
    constexpr static auto GET_READY_FOR_EXERCISE =
        flow::action<"GetReadyForExercise">(
            []() { std::cout << "Sports wear" << std::endl; });

    constexpr static auto GET_READY_TO_WORK = flow::action<"GetReadyToWork">(
        []() { std::cout << "Office wear" << std::endl; });

    // Extend flow services
    constexpr static auto config = cib::config(

        cib::extend<morning_routine_t>(
            self_care_component_t::WAKE_UP >>
            dress_up_component_t::GET_READY_FOR_EXERCISE >>
            self_care_component_t::EXERCISE >>
            self_care_component_t::TAKE_BATH >>
            dress_up_component_t::GET_READY_TO_WORK >>
            food_component_t::BREAKFAST),

        cib::extend<evening_routine_t>(
            dress_up_component_t::GET_READY_FOR_EXERCISE >>
            self_care_component_t::EXERCISE));
};

struct commute_component_t {
    /*
      Actions: GO_TO_OFFICE, RETURN_HOME
    */
    constexpr static auto GO_TO_OFFICE = flow::action<"GoToOffice">(
        []() { std::cout << "Commute to office" << std::endl; });

    constexpr static auto RETURN_HOME = flow::action<"ReturnHome">(
        []() { std::cout << "Commute to home" << std::endl; });

    // Extend flow services
    constexpr static auto config = cib::config(

        cib::extend<morning_routine_t>(food_component_t::BREAKFAST >>
                                       commute_component_t::GO_TO_OFFICE),

        cib::extend<evening_routine_t>(
            commute_component_t::RETURN_HOME >>
            dress_up_component_t::GET_READY_FOR_EXERCISE));
};

struct daily_routine_component_t {
    constexpr static auto DAILY_ROUTINES = flow::action<"Daily Routines">([]() {
        static auto const ONE_DAY_IN_MILLISECONDS = 24 * 60 * 60 * 1000;

        static bool daysRoutineComplete = false;
        static uint32_t dayCount = 1;
        static auto startTime = std::chrono::system_clock::now();

        if (!daysRoutineComplete) {
            std::cout << "----- Day: " << dayCount << " -----\n";
            flow::run<morning_routine_t>();
            flow::run<evening_routine_t>();
            daysRoutineComplete = true;
        }

        auto currentTime = std::chrono::system_clock::now();
        auto elapsedTime =
            std::chrono::duration_cast<std::chrono::milliseconds>(currentTime -
                                                                  startTime);
        if (elapsedTime.count() >= ONE_DAY_IN_MILLISECONDS) {
            dayCount++;
            startTime = currentTime;
            daysRoutineComplete = false;
        }
    });

    constexpr static auto config = cib::config(

        cib::exports<morning_routine_t, evening_routine_t>,

        // we need to extend the MainLoop as cib::top implements
        // MainLoop service
        cib::extend<cib::MainLoop>(DAILY_ROUTINES));
};

struct person_routine_proj {
    constexpr static auto config =
        cib::components<daily_routine_component_t, self_care_component_t,
                        food_component_t, dress_up_component_t,
                        commute_component_t>;
};

cib::top<person_routine_proj> top{};

int main() {
    top.main();

    return 0;
}
