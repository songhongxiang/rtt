/***************************************************************************
  tag: Peter Soetens  Thu Apr 22 20:40:59 CEST 2004  FakeAnalogDevice.hpp 

                        FakeAnalogDevice.hpp -  description
                           -------------------
    begin                : Thu April 22 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be
 
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
 
#ifndef FAKEANALOGDEVICE_HPP
#define FAKEANALOGDEVICE_HPP

#include <device_interface/AnalogInInterface.hpp>
#include <device_interface/AnalogOutInterface.hpp>

namespace ORO_DeviceDriver
{
    using namespace ORO_DeviceInterface;

    /**
     * A test class which replaces a real device driver.
     * It reproduces on the output what it gets on the input.
     */
    struct FakeAnalogDevice :
        public AnalogInInterface<unsigned int>,
        public AnalogOutInterface<unsigned int>
    {
        const static int chans = 32;
        unsigned int channels[chans];

        FakeAnalogDevice()
            : AnalogInInterface<unsigned int>("FakeAnalogDevice"),
              AnalogOutInterface<unsigned int>("FakeAnalogDevice")
        {}

        virtual void rangeSet(unsigned int chan, 
                              unsigned int range) {}

        virtual void arefSet(unsigned int chan, 
                             unsigned int aref) {}

        virtual unsigned int nbOfChannels() const {
            return chans;
        }

        virtual void read( unsigned int chan, unsigned int& value ) const 
        {
            value = channels[chan];
        }

        virtual void write( unsigned int chan, unsigned int value ) {
            channels[chan] = value;
        }

        virtual unsigned int binaryRange() const
        {
            return 4096;
        }

        virtual unsigned int binaryLowest() const 
        {
            return 0;
        }

        virtual unsigned int binaryHighest() const
        {
            return 4096;
        }

        virtual double lowest(unsigned int chan) const
        {
            return -5.0;
        }

        virtual double highest(unsigned int chan) const
        {
            return +5.0;
        }

        virtual double resolution(unsigned int chan) const
        {
            return 4096/10.0;
        }

    };    

}

#endif
