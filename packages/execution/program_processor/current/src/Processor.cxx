/***************************************************************************
  tag: Peter Soetens  Mon May 10 19:10:36 CEST 2004  Processor.cxx

                        Processor.cxx -  description
                           -------------------
    begin                : Mon May 10 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/
#include "execution/Processor.hpp"
#include "execution/ProgramInterface.hpp"
#include "execution/StateContextTree.hpp"
#include <corelib/CommandInterface.hpp>


#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <os/MutexLock.hpp>

namespace ORO_Execution
{

    using boost::bind;
    using namespace std;
    using ORO_OS::MutexLock;

        struct Processor::ProgramInfo
        {
            ProgramInfo(const std::string&_name, ProgramInterface* p)
                : program(p),
                  running(false),
                  stepping(false), name(_name) {}
            ProgramInterface* program;
            bool running;
            bool stepping;
            std::string name;
        };

        struct Processor::StateInfo
        {
            // The 'global' state the state machine is in.
            enum state_states { inactive, active, running, stopped, paused, todelete };
            state_states gstate; // 'global' state of this StateContext
            StateInfo(const std::string& _name, StateContextTree* s)
                : gstate( inactive ),
                  state(s),
                  action(0),
                  stepping(true),
                  name(_name)
            {}

            StateContextTree* state;
            boost::function<void(void)> action; // set action to zero to 'pause'

            // (de)activate may be called directly
            void activate() {
                state->activate();
                action = 0;
                gstate = active;
            }
            void deactivate() {
                state->deactivate();
                action = 0;
                gstate = inactive;
            }

            // start, stop , pause and reset work with the
            // action functor.
            void start() {
                gstate = running;
                // keep repeating the run action
                action = bind (&StateInfo::run, this);
                this->run(); // execute the first time from here.
            }

            void pause() {
                action = 0;
                gstate = paused;
            }

            void stop() {
                state->requestFinalState();
                action = 0;
                gstate = stopped;
            }
            void reset() {
                state->requestInitialState();
                action = 0;
                gstate = active;
            }
            bool stepping;
            std::string name;
        protected:
            void run() {
                if (stepping)
                    state->requestNextState(); // one state at a time
                else {
                    ORO_CoreLib::StateInterface* cur_s = state->currentState();
                    while ( cur_s != state->requestNextState() ) // go until no transition found.
                        cur_s = state->currentState();
                }
            }
            //StateInfo( const StateInfo& ) {}
        };


    Processor::Processor()
        :command(0)
    {
        programs = new list<ProgramInfo>();
        states   = new list<StateInfo>();
    }

    Processor::~Processor()
    {
        delete programs;
        delete states;
    }

    bool program_lookup( const Processor::ProgramInfo& pi, const std::string& name)
    {
        return (pi.name == name);
    }

    bool state_lookup( const Processor::StateInfo& si, const std::string& name)
    {
        return (si.name == name);
    }

	bool Processor::loadProgram(ProgramInterface* pi)
    {
        program_iter it =
            find_if(programs->begin(), programs->end(), bind(program_lookup, _1, pi->getName() ) );
        if ( it != programs->end() )
            return false;
        programs->push_back( Processor::ProgramInfo(pi->getName(), pi) );
        pi->reset();
        return true;
    }

	bool Processor::resetProgram(const std::string& name)
    {
        program_iter it =
            find_if(programs->begin(), programs->end(), bind(program_lookup, _1, name) );
        if ( it != programs->end() && it->running == false && it->stepping == false)
            {
                it->program->reset();
                return true;
            }
        return false;
    }

	bool Processor::startProgram(const std::string& name)
    {
        program_iter it =
            find_if(programs->begin(), programs->end(), bind(program_lookup, _1, name) );
        if ( it != programs->end() )
            it->running = true;
        return it != programs->end();
    }

	bool Processor::isProgramRunning(const std::string& name) const
    {
        cprogram_iter it =
            find_if(programs->begin(), programs->end(), bind(program_lookup, _1, name) );
        if ( it != programs->end() )
            return it->running;
        return false;
    }

	bool Processor::startStepping(const std::string& name)
    {
        program_iter it =
            find_if(programs->begin(), programs->end(), bind(program_lookup, _1, name) );
        if ( it != programs->end() )
            it->stepping = true;
        return it != programs->end();
    }

	bool Processor::stopProgram(const std::string& name)
    {
        program_iter it =
            find_if(programs->begin(), programs->end(), bind(program_lookup, _1, name) );
        if ( it != programs->end() )
            it->running = false;
        return it != programs->end();
    }

	bool Processor::deleteProgram(const std::string& name)
    {
        program_iter it =
            find_if(programs->begin(), programs->end(), bind(program_lookup, _1, name) );
        if ( it != programs->end() && it->running == false && it->stepping == false)
            {
                delete it->program;
                programs->erase(it);
                return true;
            }
        return false;
    }

	bool Processor::loadStateContext( StateContextTree* sc )
    {
        // test if parent ...
        if ( sc->getParent() != 0 ) {
            std::string error(
                "Could not register StateContext \"" + sc->getName() +
                "\" with the processor. It is not a root StateContext." );
            throw program_load_exception( error );
        }

        this->recursiveCheckLoadStateContext( sc ); // throws load_exception
        this->recursiveLoadStateContext( sc );
        return true;
    }
    
    void Processor::recursiveCheckLoadStateContext( StateContextTree* sc )
    {
        // test if already present..., this cannot detect corrupt
        // trees with double names...
        state_iter it =
            find_if(states->begin(), states->end(), bind(state_lookup, _1, sc->getName() ) );
        if ( it != states->end() ) {
            std::string error(
                "Could not register StateContext \"" + sc->getName() +
                "\" with the processor. A StateContext with that name is already present." );
            throw program_load_exception( error );

            std::vector<StateContextTree*>::const_iterator it2;
            for (it2 = sc->getChildren().begin(); it2 != sc->getChildren().end(); ++it2)
                {
                    this->recursiveCheckLoadStateContext( *it2 );
                }
        }
    }

    void Processor::recursiveLoadStateContext( StateContextTree* sc )
    {
        std::vector<StateContextTree*>::const_iterator it;
        for (it = sc->getChildren().begin(); it != sc->getChildren().end(); ++it)
            {
                this->recursiveLoadStateContext( *it );
            }
        
        states->push_back(Processor::StateInfo(sc->getName(), sc));
    }

	bool Processor::startStateContext(const std::string& name)
    {
        state_iter it =
            find_if(states->begin(), states->end(), bind(state_lookup, _1, name) );
        if ( it != states->end() && it->gstate == StateInfo::active ) {
            MutexLock lock( statemonitor );
            it->action = bind( &StateInfo::start, &(*it) );
            return true;
        }
        return false;
    }

	bool Processor::steppedStateContext(const std::string& name)
    {
        state_iter it =
            find_if(states->begin(), states->end(), bind(state_lookup, _1, name) );
        if ( it != states->end() )
            {
                it->stepping = true;
            }
        return it != states->end();
    }

	bool Processor::continuousStateContext(const std::string& name)
    {
        state_iter it =
            find_if(states->begin(), states->end(), bind(state_lookup, _1, name) );
        if ( it != states->end() )
            {
                it->stepping = false;
            }
        return it != states->end();
    }

	bool Processor::isStateContextRunning(const std::string& name) const
    {
        cstate_iter it =
            find_if(states->begin(), states->end(), bind(state_lookup, _1, name) );
        if ( it != states->end() )
            return it->gstate == StateInfo::running;
        return false;
    }

	bool Processor::isStateContextStepped(const std::string& name) const
    {
        cstate_iter it =
            find_if(states->begin(), states->end(), bind(state_lookup, _1, name) );
        if ( it != states->end() )
            return it->stepping;
        return false;
    }

    bool Processor::activateStateContext( const std::string& name )
    {
        state_iter it =
            find_if(states->begin(), states->end(), bind(state_lookup, _1, name) );
        if ( it != states->end() && it->gstate == StateInfo::inactive )
            {
                it->activate();
                return true;
            }
        return false;
    }

    bool Processor::deactivateStateContext( const std::string& name )
    {
        state_iter it =
            find_if(states->begin(), states->end(), bind(state_lookup, _1, name) );
        if ( it != states->end() && it->gstate == StateInfo::stopped )
            {
                it->deactivate();
                return true;
            }
        return false;
    }

    bool Processor::pauseStateContext(const std::string& name)
    {
        state_iter it =
            find_if(states->begin(), states->end(), bind(state_lookup, _1, name) );
        if ( it != states->end() && it->gstate == StateInfo::running )
            {
                MutexLock lock( statemonitor );
                it->action = bind( &StateInfo::pause, &(*it) );
                return true;
            }
        return false;
    }

	bool Processor::stopStateContext(const std::string& name)
    {
        state_iter it =
            find_if(states->begin(), states->end(), bind(state_lookup, _1, name) );
        if ( it != states->end() && ( it->gstate == StateInfo::paused
                                     || it->gstate == StateInfo::active
                                     || it->gstate == StateInfo::running) )
            {
                MutexLock lock( statemonitor );
                it->action = bind( &StateInfo::stop, &(*it) );
                return true;
            }
        return false;
    }

	bool Processor::resetStateContext(const std::string& name)
    {
        // We can only reset when stopped.
        state_iter it =
            find_if(states->begin(), states->end(), bind(state_lookup, _1, name) );
        if ( it != states->end() && it->gstate == StateInfo::stopped )
            {
                MutexLock lock( statemonitor );
                it->action = bind( &StateInfo::reset, &(*it) );
                return true;
            }
        return false;
    }

    bool Processor::unloadStateContext( const std::string& name )
    {
        // this does the same as deleteStateContext, except for deleting
        // the unloaded context..
        state_iter it =
            find_if(states->begin(), states->end(), bind(state_lookup, _1, name) );
        if ( it != states->end() )
        {
            // test if parent ...
            if ( it->state->getParent() != 0 ) {
                std::string error(
                                  "Could not unload StateContext \"" + it->state->getName() +
                                  "\" with the processor. It is not a root StateContext." );
                throw program_unload_exception( error );
            }
            recursiveCheckUnloadStateContext( *it );
            recursiveUnloadStateContext( it->state );
            return true;
        }
        return false;
    }

    void Processor::recursiveCheckUnloadStateContext(const StateInfo& si)
    {
        if ( si.gstate != StateInfo::inactive ) {
            std::string error(
                              "Could not unload StateContext \"" + si.state->getName() +
                              "\" with the processor. It is still active." );
            throw program_unload_exception( error );
            std::vector<StateContextTree*>::const_iterator it2;
            for (it2 = si.state->getChildren().begin();
                 it2 != si.state->getChildren().end();
                 ++it2)
            {
                state_iter it =
                    find_if(states->begin(),
                            states->end(),
                            bind(state_lookup, _1, (*it2)->getName() ) );
                if ( it == states->end() ) {
                    std::string error(
                              "Could not unload StateContext \"" + si.state->getName() +
                              "\" with the processor. It contains not loaded children." );
                    throw program_unload_exception( error );
                }
                // all is ok, check child :
                this->recursiveCheckUnloadStateContext( *it );
            }
        }
    }

    void Processor::recursiveUnloadStateContext(StateContextTree* sc) {
        std::vector<StateContextTree*>::const_iterator it;
        // erase children
        for (it = sc->getChildren().begin(); it != sc->getChildren().end(); ++it)
            {
                this->recursiveUnloadStateContext( *it );
            }
        
        // erase this sc :
        state_iter it2 =
            find_if(states->begin(),
                    states->end(),
                    bind(state_lookup, _1, sc->getName() ) );
        assert( it2 != states->end() ); // we checked that this is possible

        states->erase(it2);
    }

	bool Processor::deleteStateContext(const std::string& name)
    {
        state_iter it =
            find_if(states->begin(), states->end(), bind(state_lookup, _1, name) );
        if ( it != states->end() )
            {
                if ( it->state->getParent() != 0 ) {
                    std::string error(
                                      "Could not unload StateContext \"" + it->state->getName() +
                                      "\" with the processor. It is not a root StateContext." );
                    throw program_unload_exception( error );
                }
                StateContextTree* todelete = it->state;
                MutexLock lock( statemonitor );
                // same pre-conditions for delete as for unload :
                recursiveCheckUnloadStateContext( *it );
                recursiveUnloadStateContext( it->state ); // this invalidates it !
                delete todelete;
                return true;
            }
        return false;
    }
    void executeState( Processor::StateInfo& s)
    {
        if ( s.action )
            s.action();
    }

    void executeProgram( Processor::ProgramInfo& p)
    {
        if (p.running)
            p.program->execute();
    }

    void stepProgram( Processor::ProgramInfo& p)
    {
        if (p.stepping)
            {
                p.program->execute();
                p.stepping=false;
            }
    }

	void Processor::doStep()
    {
        {
            MutexLock lock( statemonitor );
            // Evaluate all states->
            for_each(states->begin(), states->end(), executeState );
        }

        // Execute any additional (deferred/external) command.
        if ( command )
            {
                command->execute();
                command = 0;
                // should we allow the system context to check validity ??
                // external commands can gravely interfere with the
                // system...
                // for_each(states->begin(), states->end(), executeState);
            }
        {
            MutexLock lock( progmonitor );
            //Execute all normal programs->
            for_each(programs->begin(), programs->end(), executeProgram);

            //Execute all programs in Stepping mode.
            for_each(programs->begin(), programs->end(), stepProgram );
        }
    }


    bool Processor::nextStep(const std::string& name)
    {
        program_iter it =
            find_if(programs->begin(), programs->end(), bind(program_lookup, _1, name) );
        if ( it != programs->end() )
            it->stepping = true;
        return it != programs->end();
    }


    bool Processor::process( CommandInterface* c)
    {
        if (command != 0)
            return false;
        command = c;
        return true;
    }

    std::vector<std::string> Processor::getProgramList()
    {
        std::vector<std::string> ret;
        for ( program_iter i = programs->begin(); i != programs->end(); ++i )
            ret.push_back( i->name );
        return ret;
    }

    std::vector<std::string> Processor::getStateContextList()
    {
        std::vector<std::string> ret;
        for ( state_iter i = states->begin(); i != states->end(); ++i )
            ret.push_back( i->name );
        return ret;
    }

    bool Processor::isCommandProcessed( CommandInterface* c )
    {
        return command != c;
    }

    void Processor::abandonCommand( CommandInterface* c )
    {
        if ( command == c )
            command = 0;
    }
}

