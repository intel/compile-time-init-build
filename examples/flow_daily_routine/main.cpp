#include<iostream>
#include<flow/flow.hpp>
#include<cib/cib.hpp>
#include<chrono>

//In this example we shall see the Daily routine of a individual person
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

  Morning_Routine_ts are:  WakeUp, GetReadyForExercise, Exercise, TakeBath, GetReadyToWork, HaveBreakfast, GoToOffice
  Evening_Routine_ts are:  ReturnHome, GetReadyToExercise, Exercise, TakeBath, RelaxForSometime, HaveDinner, GoToBed

  These things should be done in order!!
  Flow part of the CIB library will help us to achieve the same
  Following concents in the flow can be used to achieve the same
    + Flow::Service<>
    + Flow::Action<>
    + Flow::MileStone<>
    + operator >>
    + operator &&
  
  Flow::Service ==> 2 or more Actions and/or MileStones
  >>            ==> used to link actions/milestones to be executed in order
  &&            ==> used to link actions/milestones without any order!!

  Note:: 
    services can be defined using 
      + cib::callback_meta<>  - can have only one function pointer assigned to execute
      + flow::service<>       - set of actions/milestones which are to be executed in specific order. So basically sequential execution of multiple function pointers!!

    both services are different.

    Flow can be treated like a Directed Acyclic graph
    Any component can implement actions/milestones
    Any component can mention a sequence of execution of actions/milestones
    Services can be executed with flow::run() in an action
    That action should extend the MainLoop of the cib library!!
*/

/*
  Service definitions!!
*/
//struct Morning_Routine_t : public flow::service<decltype("Morning_Routine_t"_sc)> //decltype("Morning_Routine_t"_sc) - string constant "Morning_Routine_t" is used during the logging
struct Morning_Routine_t : public flow::service<> //When logging or naming is not needed!!
{};

//struct Evening_Routine_t : public flow::service<decltype("Evening_Routine_t"_sc)>
struct Evening_Routine_t : public flow::service<>
{};

/*
  Component definitions!!
*/
struct Self_Care_Component_t
{
  /*
    Actions: WAKE_UP, EXERCISE, GO_TO_BED, RELAX, TAKE_BATH
  */
  constexpr static auto WAKE_UP = flow::action(
    "WakeUp"_sc,
    []()
    {
      std::cout<<"Wake up at 6:00 AM" <<std::endl;
    }
  );

  constexpr static auto EXERCISE = flow::action(
    "Exercise"_sc,
    []()
    {
      std::cout<<"Gym activities!!" <<std::endl;
    }
  );


  constexpr static auto GO_TO_BED = flow::action(
    "GoToBed"_sc,
    []()
    {
      std::cout<<"Go to bed at 10:00 PM" <<std::endl;
    }
  );

  constexpr static auto RELAX = flow::action(
    "Relax"_sc,
    []()
    {
      std::cout << "Have relax before Dinner" << std::endl;
    }
  );

  constexpr static auto TAKE_BATH = flow::action(
    "TakeBath"_sc,
    []()
    {
      std::cout << "Take a bath" << std::endl;
    }
  );

  //Extend flow services!!
  constexpr static auto config = cib::config(
    cib::extend<Morning_Routine_t>(
      Self_Care_Component_t::WAKE_UP >>
      Self_Care_Component_t::EXERCISE >>
      Self_Care_Component_t::TAKE_BATH
    ),

    cib::extend<Evening_Routine_t>(
      Self_Care_Component_t::EXERCISE >>
      Self_Care_Component_t::TAKE_BATH >>
      Self_Care_Component_t::RELAX >>
      Self_Care_Component_t::GO_TO_BED
    )
  );
};

struct Food_Component_t
{
  /*
    Actions: BREAKFAST, DINNER
  */
  constexpr static auto BREAKFAST = flow::action(
    "Breakfast"_sc,
    []()
    {
      std::cout << "Have healthy breakfast" << std::endl;
    }
  );

  constexpr static auto DINNER = flow::action(
    "Dinner"_sc,
    []()
    {
      std::cout << "Have early dinner" << std::endl;
    }
  );

  //Extend flow services!!
  constexpr static auto config = cib::config(
    cib::extend<Morning_Routine_t>(
      Self_Care_Component_t::TAKE_BATH >>
      Food_Component_t::BREAKFAST
    ),

    cib::extend<Evening_Routine_t>(
      Self_Care_Component_t::RELAX >>
      Food_Component_t::DINNER >>
      Self_Care_Component_t::GO_TO_BED
    )
  );
};

struct Dress_Up_Component_t
{
  /*
    Actions: GET_READY_FOR_EXERCISE, GET_READY_TO_WORK
  */
  constexpr static auto GET_READY_FOR_EXERCISE = flow::action(
    "GetReadyForExercise"_sc,
    []()
    {
      std::cout << "Sports wear" << std::endl;
    }
  );

  constexpr static auto GET_READY_TO_WORK = flow::action(
    "GetReadyToWork"_sc,
    []()
    {
      std::cout << "Office wear" << std::endl;
    }
  );

  //Extend flow services!!
  constexpr static auto config = cib::config(
    cib::extend<Morning_Routine_t>(
      Self_Care_Component_t::WAKE_UP >>
      Dress_Up_Component_t::GET_READY_FOR_EXERCISE >>
      Self_Care_Component_t::EXERCISE >>
      Self_Care_Component_t::TAKE_BATH >>
      Dress_Up_Component_t::GET_READY_TO_WORK >>
      Food_Component_t::BREAKFAST
    ),
  
    cib::extend<Evening_Routine_t>(
      Dress_Up_Component_t::GET_READY_FOR_EXERCISE >>
      Self_Care_Component_t::EXERCISE
    )
  );
};

struct Commute_Component_t
{
  /*
    Actions: GO_TO_OFFICE, RETURN_HOME
  */
  constexpr static auto GO_TO_OFFICE = flow::action(
    "GoToOffice"_sc,
    []()
    {
      std::cout << "Commute to office" << std::endl;
    }
  );

  constexpr static auto RETURN_HOME = flow::action(
    "ReturnHome"_sc,
    []()
    {
      std::cout << "Commute to home" << std::endl;
    }
  );

  //Extend flow services!!
  constexpr static auto config = cib::config(
    cib::extend<Morning_Routine_t>(
      Food_Component_t::BREAKFAST  >>
      Commute_Component_t::GO_TO_OFFICE
    ),
    cib::extend<Evening_Routine_t>(
      Commute_Component_t::RETURN_HOME >>
      Dress_Up_Component_t::GET_READY_FOR_EXERCISE
    )
  );
};

struct Daily_Routine_Component_t
{
  constexpr static auto DAILY_ROUTINES = flow::action(
    "Daily Routines"_sc,
    []()
    {
      static const auto ONE_DAY_IN_MILLISECONDS = 24 * 60 * 60 * 1000;

      static bool daysRoutineComplete = false;
      static uint32_t dayCount = 1;
      static auto startTime = std::chrono::system_clock::now();
      

      if(!daysRoutineComplete)
      {
        std::cout << "----- Day: " << dayCount << " -----\n";
        flow::run<Morning_Routine_t>();
        flow::run<Evening_Routine_t>();
        daysRoutineComplete = true;
      }

      auto currentTime = std::chrono::system_clock::now();
      auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
      if(elapsedTime.count() >= ONE_DAY_IN_MILLISECONDS)
      {
        dayCount++;
        startTime = currentTime;
        daysRoutineComplete = false;
      }
    }
  );

  constexpr static auto config = cib::config(
    cib::exports<
      Morning_Routine_t,
      Evening_Routine_t>,    

    // we need to extend the MainLoop as cib::top implements MainLoop service
    cib::extend<cib::MainLoop>(
      DAILY_ROUTINES
    )
  );
};

struct Person_Routine_Proj
{
  constexpr static auto config = cib::components<
      Daily_Routine_Component_t,
      Self_Care_Component_t,
      Food_Component_t,
      Dress_Up_Component_t,
      Commute_Component_t
    >;
};

cib::top<Person_Routine_Proj> top{};

int main()
{
  top.main();

  return 0;
}