/***************************************************************************
  tag: Peter Soetens  Tue Dec 21 22:43:08 CET 2004  TaskContext.hpp 

                        TaskContext.hpp -  description
                           -------------------
    begin                : Tue December 21 2004
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
 
 
#ifndef ORO_TASK_CONTEXT_HPP
#define ORO_TASK_CONTEXT_HPP

#include "Factories.hpp"
#include "AttributeRepository.hpp"
#include "EventService.hpp"
#include "ExecutionEngine.hpp"

#include <string>
#include <map>

namespace ORO_Execution
{
    class CommandProcessor;

    /**
     * A TaskContext groups the operations, events, datasources,
     * peer-tasks and processor a task has. It links related tasks
     * and allows to iterate over its peers.
     *
     * When a peer is added, the (script) programs of this task can access
     * the peer using peername.methodname() or peername.objname.methodname().
     * The commands of this TaskContext must be executed by this TaskContext's
     * CommandProcessor (which is what scripts take care of). The methods and datasources
     * can be queried by any peer TaskContext at any time.
     *
     * The ExecutionEngine::step() must be invoked seperately from a
     * PeriodicTask or other TaskInterface implementation, as long as
     * that Task is not started, this TaskContext will not accept any
     * commands.  In this way, the user of this class can determine
     * himself at which point and at which moment remote commands and
     * local programs can be executed.
     */
    class TaskContext
    {
        // non copyable
        TaskContext( TaskContext& );
    protected:
        std::string    _task_name;
    
        typedef std::map< std::string, TaskContext* > PeerMap;
        PeerMap         _task_map;

        ExecutionEngine ee;
    public:
        typedef std::vector< std::string > PeerList;

        /**
         * Create a TaskContext visible with \a name.
         * It's ExecutionEngine will be newly constructed with private 
         * ExecutionEngine processing its commands, events,
         * programs and state machines.
         */
        TaskContext( const std::string& name );

        /**
         * Create a TaskContext visible with \a name. Its commands
         * programs and state machines are processed by \a parent.
         * Use this constructor to share execution engines among task contexts, such that
         * the execution of their functionality is serialised (executed in the same thread).
         */
        TaskContext(const std::string& name, ExecutionEngine* parent );

        ~TaskContext();

        /**
         * Queue a command.
         * @return True if the Processor accepted the command.
         */
        bool executeCommand( CommandInterface* c);

        /**
         * Queue a command. If the Processor is not running or not accepting commands
         * this will fail and return zero.
         * @return The command id the processor returned.
         */
        int queueCommand( CommandInterface* c);

        /**
         * Get the name of this TaskContext.
         */
        const std::string& getName()
        {
            return _task_name;
        }

        /**
         * Change the name of this TaskContext.
         */
        void setName(const std::string& n)
        {
            _task_name = n;
        }
        /**
         * Add a one-way connection from this task to a peer task.
         * @param peer The peer to add.
         * @param alias An optional alias (another name) for the peer.
         * defaults to \a peer->getName()
         */
        bool addPeer( TaskContext* peer, std::string alias = "" );

        /**
         * Remove a one-way connection from this task to a peer task.
         */
        void removePeer( const std::string& name );

        /**
         * Add a two-way connection from  this task to a peer task.
         * This method is strict : both peers must not be connected to succeed.
         */
        bool connectPeers( TaskContext* peer );

        /**
         * Remove a two-way connection from this task to a peer task.
         * This method is strict : both peers must be connected to succeed.
         */
        void disconnectPeers( const std::string& name );

        /**
         * Return a standard container which contains all the Peer's names
         */
        PeerList getPeerList() const;

        /**
         * Return true if it knows a peer by that name.
         */
        bool hasPeer( const std::string& peer_name ) const;

        /**
         * Get a pointer to a peer of this task.
         * @return null if no such peer.
         */
        TaskContext* getPeer(const std::string& peer_name ) const;

        /**
         * Get a const pointer to the ExecutionEngine of this Task.
         * @see getExecutionEngine()
         */
        const ExecutionEngine* engine() const
        {
            return &ee;
        }

        /**
         * Get a const pointer to the ExecutionEngine of this Task.
         * @see engine()
         */
        const ExecutionEngine* getExecutionEngine() const
        {
            return &ee;
        }

        /**
         * Get a pointer to the ExecutionEngine of this Task.
         * @see getExecutionEngine()
         */
        ExecutionEngine* engine()
        {
            return &ee;
        }

        /**
         * Get a pointer to the ExecutionEngine of this Task.
         * @see engine()
         */
        ExecutionEngine* getExecutionEngine()
        {
            return &ee;
        }

        /**
         * Returns the Processor of this task.
         * @deprecated by getExecutionEngine()
         */
        ExecutionEngine* getProcessor()
        {
            return &ee;
        }

        /**
         * The Commands of this TaskContext.
         */
        GlobalCommandFactory* commands() {
            return &commandFactory;
        }

        /**
         * The Commands of this TaskContext.
         */
        const GlobalCommandFactory* commands() const{
            return &commandFactory;
        }

        /**
         * The Methods of this TaskContext.
         */
        GlobalMethodFactory* methods() {
            return &methodFactory;
        }

        /**
         * The Methods of this TaskContext.
         */
        const GlobalMethodFactory* methods() const{
            return &methodFactory;
        }

        /**
         * The Data of this TaskContext.
         */
        GlobalDataSourceFactory* data() {
            return &dataFactory;
        }

        /**
         * The Data of this TaskContext.
         */
        const GlobalDataSourceFactory* data() const{
            return &dataFactory;
        }

        /**
         * The task-local values ( attributes and properties ) of this TaskContext.
         */
        AttributeRepository* attributes() {
            return &attributeRepository;
        }

        /**
         * The task-local values ( attributes and properties ) of this TaskContext.
         */
        const AttributeRepository* attributes() const{
            return &attributeRepository;
        }

        /**
         * The task-local events ( 'signals' ) of this TaskContext.
         */
        EventService* events() {
            return &eventService;
        }

        /**
         * The task-local events ( 'signals' ) of this TaskContext.
         */
        const EventService* events() const{
            return &eventService;
        }


        /**
         * The Command Factory of this TaskContext.
         * @deprecated by commands()
         */
        GlobalCommandFactory    commandFactory;
        /**
         * The DataSource Factory of this TaskContext.
         * @deprecated by data()
         */
        GlobalDataSourceFactory dataFactory;
        /**
         * The Method Factory of this TaskContext.
         * @deprecated by methods()
         */
        GlobalMethodFactory     methodFactory;

        /**
         * The task-local values ( attributes ) of this TaskContext.
         * @deprecated by attributes()
         */
        AttributeRepository     attributeRepository;

        /**
         * The task-local events ( 'signals' ) of this TaskContext.
         * @deprecated by events()
         */
        EventService            eventService;
    };
}

#endif
