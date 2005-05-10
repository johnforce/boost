//////////////////////////////////////////////////////////////////////////////
// (c) Copyright Andreas Huber Doenni 2005
// Distributed under the Boost Software License, Version 1.0. (See accompany-
// ing file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////



#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include <boost/mpl/list.hpp>

#include <boost/test/test_tools.hpp>

#include <set>
#include <map>
#include <string>



namespace sc = boost::statechart;
namespace mpl = boost::mpl;



struct EvToC : sc::event< EvToC > {};
struct EvToD : sc::event< EvToD > {};

struct EvDiscardNever : sc::event< EvDiscardNever > {};
struct EvDiscardInB : sc::event< EvDiscardInB > {};
struct EvDiscardInD : sc::event< EvDiscardInD > {};

struct EvTransit : sc::event< EvTransit > {};
struct EvTransitWithAction : sc::event< EvTransitWithAction > {};
struct EvDefer : sc::event< EvDefer > {};
struct EvTerminate : sc::event< EvTerminate > {};


struct A;
struct CustomReactionTest : sc::state_machine< CustomReactionTest, A >
{
  public:
    //////////////////////////////////////////////////////////////////////////
    CustomReactionTest();

    void Visited( const state_base_type & stt )
    {
      const StateNamesMap::const_iterator found =
        stateNamesMap_.find( stt.dynamic_type() );
      BOOST_REQUIRE( found != stateNamesMap_.end() );
      visitedStates_.insert( found->second );
    }

    void ClearVisited()
    {
      visitedStates_.clear();
    }

    void AssertVisitedAll( const std::string & stateNames ) const
    {
      for ( std::string::const_iterator expectedName = stateNames.begin();
        expectedName != stateNames.end(); ++expectedName )
      {
        BOOST_REQUIRE( visitedStates_.find(
          std::string( 1, *expectedName ) ) != visitedStates_.end() );
      }
    }

    void AssertVisitedOne( const std::string & stateNames ) const
    {
      size_t found = 0;

      for ( std::string::const_iterator actualName = stateNames.begin();
        actualName != stateNames.end(); ++actualName )
      {
        found = found + ( visitedStates_.find(
          std::string( 1, *actualName ) ) != visitedStates_.end() ) ? 1 : 0;
      }

      BOOST_REQUIRE( found == 1 );
    }

    void TransitionAction( const EvTransitWithAction & ) {}

  private:
    //////////////////////////////////////////////////////////////////////////
    typedef std::map< state_base_type::id_type, std::string > StateNamesMap;
    typedef std::set< std::string > VisitedStates;

    StateNamesMap stateNamesMap_;
    VisitedStates visitedStates_;
};

struct B;
struct A : sc::simple_state< A, CustomReactionTest, mpl::list<
  sc::custom_reaction< EvTransit >,
  sc::custom_reaction< EvTransitWithAction >,
  sc::custom_reaction< EvDefer >,
  sc::custom_reaction< EvTerminate >,
  sc::custom_reaction< EvDiscardNever >,
  sc::custom_reaction< EvDiscardInB >,
  sc::custom_reaction< EvDiscardInD > >, B >
{
  sc::result react( const EvDiscardNever & )
  {
    outermost_context().Visited( *this );
    return forward_event();
  }

  sc::result react( const EvDiscardInB & )
  {
    BOOST_FAIL( "An event discarded in B must never reach A" );
    return discard_event();
  }

  sc::result react( const EvDiscardInD & )
  {
    BOOST_FAIL( "An event discarded in D must never reach B" );
    return discard_event();
  }

  // The following code is here just to make sure that calls to the transit<>,
  // defer_event and terminate functions actually compile.
  // Their functionality is tested extensively in TransitionTest,
  // DeferralTest and TerminationTest with appropriate reactions. Internally,
  // these reactions call exactly the same functions as the following custom
  // reactions call.
  sc::result react( const EvTransit & )
  {
    return transit< A >();
  }

  sc::result react( const EvTransitWithAction & evt )
  {
    return transit< A >( &CustomReactionTest::TransitionAction, evt );
  }

  sc::result react( const EvDefer & )
  {
    return defer_event();
  }

  sc::result react( const EvTerminate & )
  {
    return terminate();
  }
};

  struct C;
  struct B : sc::simple_state< B, A, mpl::list<
    sc::custom_reaction< EvDiscardNever >,
    sc::custom_reaction< EvDiscardInB >,
    sc::custom_reaction< EvDiscardInD > >, C >
  {
    sc::result react( const EvDiscardNever & )
    {
      outermost_context().Visited( *this );
      return forward_event();
    }

    sc::result react( const EvDiscardInB & )
    {
      outermost_context().Visited( *this );
      return discard_event();
    }

    sc::result react( const EvDiscardInD & )
    {
      BOOST_FAIL( "An event discarded in D must never reach B" );
      return discard_event();
    }
  };

    struct E;
    struct F;
    struct D : sc::simple_state< D, B, mpl::list<
      sc::transition< EvToC, C >,
      sc::custom_reaction< EvDiscardNever >,
      sc::custom_reaction< EvDiscardInB >,
      sc::custom_reaction< EvDiscardInD > >, mpl::list< E, F > >
    {
      sc::result react( const EvDiscardNever & )
      {
        outermost_context().Visited( *this );
        return forward_event();
      }

      sc::result react( const EvDiscardInB & )
      {
        outermost_context().Visited( *this );
        return forward_event();
      }

      sc::result react( const EvDiscardInD & )
      {
        outermost_context().Visited( *this );
        return discard_event();
      }
    };

      struct E : sc::simple_state< E, D::orthogonal< 0 >, mpl::list<
        sc::custom_reaction< EvDiscardNever >,
        sc::custom_reaction< EvDiscardInB >,
        sc::custom_reaction< EvDiscardInD > > >
      {
        sc::result react( const EvDiscardNever & )
        {
          outermost_context().Visited( *this );
          return forward_event();
        }

        sc::result react( const EvDiscardInB & )
        {
          outermost_context().Visited( *this );
          return forward_event();
        }

        sc::result react( const EvDiscardInD & )
        {
          outermost_context().Visited( *this );
          return forward_event();
        }
      };

      struct F : sc::simple_state< F, D::orthogonal< 1 >, mpl::list<
        sc::custom_reaction< EvDiscardNever >,
        sc::custom_reaction< EvDiscardInB >,
        sc::custom_reaction< EvDiscardInD > > >
      {
        sc::result react( const EvDiscardNever & )
        {
          outermost_context().Visited( *this );
          return forward_event();
        }

        sc::result react( const EvDiscardInB & )
        {
          outermost_context().Visited( *this );
          return forward_event();
        }

        sc::result react( const EvDiscardInD & )
        {
          outermost_context().Visited( *this );
          return forward_event();
        }
      };

    struct C : sc::simple_state< C, B, mpl::list<
      sc::transition< EvToD, D >,
      sc::custom_reaction< EvDiscardNever >,
      sc::custom_reaction< EvDiscardInB >,
      sc::custom_reaction< EvDiscardInD > > >
    {
      sc::result react( const EvDiscardNever & )
      {
        outermost_context().Visited( *this );
        return forward_event();
      }

      sc::result react( const EvDiscardInB & )
      {
        outermost_context().Visited( *this );
        return forward_event();
      }

      sc::result react( const EvDiscardInD & )
      {
        outermost_context().Visited( *this );
        return forward_event();
      }
    };

CustomReactionTest::CustomReactionTest()
{
  // We're not using custom type information to make this test work even when
  // BOOST_STATECHART_USE_NATIVE_RTTI is defined
  stateNamesMap_[ A::static_type() ] = "A";
  stateNamesMap_[ B::static_type() ] = "B";
  stateNamesMap_[ C::static_type() ] = "C";
  stateNamesMap_[ D::static_type() ] = "D";
  stateNamesMap_[ E::static_type() ] = "E";
  stateNamesMap_[ F::static_type() ] = "F";
}

int test_main( int, char* [] )
{
  CustomReactionTest machine;
  machine.initiate();

  machine.process_event( EvDiscardNever() );
  machine.AssertVisitedAll( "ABC" );
  machine.ClearVisited();

  machine.process_event( EvDiscardInB() );
  machine.AssertVisitedAll( "BC" );
  machine.process_event( EvToD() );
  machine.ClearVisited();

  machine.process_event( EvDiscardNever() );
  machine.AssertVisitedAll( "ABDEF" );
  machine.ClearVisited();

  machine.process_event( EvDiscardInD() );
  machine.AssertVisitedAll( "D" );
  machine.AssertVisitedOne( "EF" );
  machine.ClearVisited();

  machine.process_event( EvDiscardInB() );
  machine.AssertVisitedAll( "BD" );
  machine.AssertVisitedOne( "EF" );
  machine.ClearVisited();

  return 0;
}
