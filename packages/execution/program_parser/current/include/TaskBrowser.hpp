/***************************************************************************
  tag: Peter Soetens  Tue Dec 21 22:43:08 CET 2004  TaskBrowser.hpp 

                        TaskBrowser.hpp -  description
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
 
 
#ifndef ORO_TASKBROWSER_HPP
#define ORO_TASKBROWSER_HPP
 

#include "MethodFactoryInterface.hpp"
#include "DataSourceFactoryInterface.hpp"
#include "CommandFactoryInterface.hpp"
#include <deque>
#include <string>
#include <sstream>
#include <vector>


#ifdef ORO_PRAGMA_INTERFACE
#pragma interface
#endif

namespace ORO_CoreLib {
    class ConditionInterface;
    class CommandInterface;
}

namespace ORO_Execution
{
    class TaskContext;

    /**
     * @brief This class allows a text client to browse the
     * peers of a TaskContext and execute commands.
     * If your console does not support colors or you want a different
     * prompt, the member variables which control these 'escape sequences'
     * are public and may be changed.
     */
    class TaskBrowser
    {
        // the 'current' task context
        static TaskContext* taskcontext;
        // the TC we are using for completion.
        static TaskContext* peer;
        ORO_CoreLib::ConditionInterface* condition;
        ORO_CoreLib::CommandInterface*   command;
        ORO_CoreLib::DataSource<bool>::shared_ptr   accepted;
        const CommandFactoryInterface* command_fact;
        const DataSourceFactoryInterface* datasource_fact;
        const MethodFactoryInterface* method_fact;

        int debug;
        /* A static variable for holding the line. */
        char *line_read;
        int lastc; // last command's number

        std::string storedname; //! last program listed to screen
        int storedline; //!last program line number listed to screen

        std::deque<TaskContext*> taskHistory;

        /* Read a string, and return a pointer to it.
           Returns NULL on EOF. */
        char *rl_gets ();

        // use this vector to generate candidate strings
        static std::vector<std::string> candidates;
        // Add successful matches of candidate strings to completes.
        static std::vector<std::string> completes;
        static std::vector<std::string>::iterator complete_iter;

        static std::string component;
        static std::string object;
        static std::string peerpath;
        static std::string method;
        static std::string datasource;
        static std::string text;

        // helper function
        static char* dupstr( const char *s );

        static TaskContext* findPeer( std::string comm );

        static void find_completes();
        
        static void find_command(std::string::size_type startpos);
        static void find_peers(std::string::size_type startpos);

        static void find_datasource(std::string::size_type startpos);

        static void find_method(std::string::size_type startpos);

        static void find_attribute(std::string::size_type startpos);

        static char ** orocos_hmi_completion ( const char *text, int start, int end );

        void switchBack();

        void switchTask(std::string& c);

        void evalCommand(std::string& comm );

        void browserAction(std::string& act );

        void doPrint( ORO_CoreLib::DataSourceBase* ds, bool recurse);

        void printResult( ORO_CoreLib::DataSourceBase* ds, bool recurse);

        void printHelp();
        void printInfo(const std::string& peerp);
        
        void printCommand( const std::string m );
                
        void printSource( const std::string m );
                
        void printMethod( const std::string m );

        void printProgram( const std::string& pn, int line = -1 );
        void printProgram( int line = -1 );

        void listText(std::stringstream& txtss,int start, int end, int ln, char s);

        static char *command_generator( const char *_text, int state );

    public :
        /**
         * Create a TaskBrowser which initially visits a given
         * TaskContext \a c.
         */
        TaskBrowser( TaskContext* c );

        ~TaskBrowser();

        /**
         * @brief Call this method from ORO_main() to 
         * process keyboard input and thus startup the
         * TaskBrowser.
         */
        void loop();

        /**
         * The prompt.
         */
        static std::string prompt;
        /**
         * The 'turn color on' escape sequence.
         */
        static std::string coloron;
        /**
         * The 'underline' escape sequence.
         */
        static std::string underline;
        /**
         * The 'turn color off' escape sequence.
         */
        static std::string coloroff;

    };

}

#endif
