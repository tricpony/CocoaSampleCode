/*
 
     File: main.c
 Abstract: n/a
  Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2009 Apple Inc. All Rights Reserved.
 
 
 */

// This example talks to the DevaSys  <http://www.devasys.com> "USB I2C/IO,� Interface Board"
// It is mainly an example of how to get to talk to a USB device from userland. The files
// in this project:
// 
//   main.c: All the steps to discover a USBDeviceInterface and USBInterfaceInterface
// 
//   printInterpretedError.c: The beginnings of a functions to decode cryptic IOReturns
// 
//   deva.c: Some functions to send commands to the I/O port on the Devasys board
// 
//   something.c: Code which does something to the USB interface found. In this case
//                it assumes there are switches and indicators attached to the I/O port
//                It reads the switched and flashes the lights.
// 
// This example is designed to work with the Ezloader example. The driver produced can
// be automaically run by Ezloader. This command is invoked in a manner like:
// 
//   0ABF03E9 2751 1001
// 
// 0ABF03E9 is the name of the command run when Ezload finds a device with VID 0xABF (2751)
// and PID 0x3E9 (1001). It also gives these as decimal parameters.
// 
// The functions in this file are structured so that every time a new data object that 
// will eventually need to be disposed of is generated a new function is called. The
// object is disposed of immediately on return from the function. This obviates the
// need for lots of clean up code in exit clauses. There are a lot of different objects
// generated in order to get to the USBInterfaceInterface object.
// 
// Note: You can define the symbol "VERBOSE" to get more commentary on what this is doing.
//
// A version of this Ap is designed to be launched for every interface in the device.
// If a device has 3 interfaces, you should compile and run 3 copies. Each copy should
// have a different isThisTheInterfaceYoureLookingFor function to discriminate between
// the different interfaces.


#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFPlugIn.h>

#include <IOKit/usb/IOUSBLib.h>

#include <unistd.h>

#include "something.h"
#include "printInterpretedError.h"

    // Set this flag to get more status messages
#define VERBOSE 1

    // Set this flag to match directly to interface, without finding device first.
#define MATCH_INTERFACE 0

void useUSBInterface(IOUSBInterfaceInterface245 **intf)
{
    printf("Now we actually get to do something with this device, wow!!!!\n");
	finallyDoSomethingWithThisDevice(intf);
}

UInt32 openUSBInterface(IOUSBInterfaceInterface245 **intf)
{
IOReturn ret;

#if VERBOSE
UInt8 n;
int i;
UInt8 direction;
UInt8 number;
UInt8 transferType;
UInt16 maxPacketSize;
UInt8 interval;
static char *types[]={
        "Control",
        "Isochronous",
        "Bulk",
        "Interrupt"};
static char *directionStr[]={
        "Out",
        "In",
        "Control"};
#endif

	ret = (*intf)->USBInterfaceOpen(intf);
    if(ret != kIOReturnSuccess)
    {
        printInterpretedError("Could not set configuration on device", ret);
        return(-1);
    }
	
#if VERBOSE
    // We don't use the endpoints in our device, but it has some anyway
    
    ret = (*intf)->GetNumEndpoints(intf, &n);
    if(ret != kIOReturnSuccess)
    {
        printInterpretedError("Could not get number of endpoints in interface", ret);
        return(0);
    }
    
    printf("%d endpoints found\n", n);
    
    for(i = 1; i<=n; i++)
    {
        ret = (*intf)->GetPipeProperties(intf, i, &direction, &number, &transferType, &maxPacketSize, &interval);
        if(ret != kIOReturnSuccess)
        {
            fprintf(stderr, "Endpoint %d -", n);
            printInterpretedError("Could not get endpoint properties", ret);
            return(0);
        }
        printf("Endpoint %d: %s %s %d, max packet %d, interval %d\n", i, types[transferType], directionStr[direction], number, maxPacketSize, interval);
    }
    
    
#endif
    return(0);
}

IOUSBInterfaceInterface245 **getUSBInterfaceInterface(io_service_t usbInterface)
{
IOReturn err;
IOCFPlugInInterface **plugInInterface=NULL;
IOUSBInterfaceInterface245 **intf=NULL;
SInt32 score;
HRESULT res;

    // There is no documentation for IOCreatePlugInInterfaceForService or QueryInterface, you have to use sample code.

	err = IOCreatePlugInInterfaceForService(usbInterface, 
                kIOUSBInterfaceUserClientTypeID, 
                kIOCFPlugInInterfaceID,
                &plugInInterface, 
                &score);
    (void)IOObjectRelease(usbInterface);				// done with the usbInterface object now that I have the plugin
    if ((kIOReturnSuccess != err) || (plugInInterface == nil) )
    {
        printInterpretedError("Unable to create plug in interface for USB interface", err);
        return(nil);
    }
    
    res = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID245), (LPVOID)&intf);
    IODestroyPlugInInterface(plugInInterface);			// done with this
	
    if (res || !intf)
    {
        fprintf(stderr, "Unable to create interface with QueryInterface %X\n", (unsigned int)res);
        return(nil);
    }
    return(intf);
}

Boolean isThisTheInterfaceYoureLookingFor(IOUSBInterfaceInterface245 **intf)
{
    //	Check to see if this is the interface you're interested in
    //  This code is only expecting one interface, so returns true
    //  the first time.
    //  You code could check the nature and type of endpoints etc

static Boolean foundOnce  = false;
    if(foundOnce)
    {
        fprintf(stderr, "Subsequent interface found, we're only intersted in 1 of them\n");
        return(false);
    }
    foundOnce = true;
    return(true);
}


int iterateinterfaces(io_iterator_t interfaceIterator)
{
io_service_t usbInterface;
int err = 0;
IOReturn ret;
IOUSBInterfaceInterface245 **intf=NULL;

    usbInterface = IOIteratorNext(interfaceIterator);
    if(usbInterface == IO_OBJECT_NULL)
    {
        fprintf(stderr, "Unable to find an Interface\n");
        return(-1);
    }
    
    while(usbInterface != IO_OBJECT_NULL)
    {
        intf = getUSBInterfaceInterface(usbInterface);
        
        if(intf != nil)
        {
  // Don't release the interface here. That's one too many releases and causes set alt interface to fail
            if(isThisTheInterfaceYoureLookingFor(intf))
            {
                err = openUSBInterface(intf);
                if(err == 0)
                {
                    useUSBInterface(intf);
                    ret = (*intf)->USBInterfaceClose(intf);
                }
        
                ret = (*intf)->Release(intf);
                // Not worth bothering with errors here
                return(err);
            }
        }
        usbInterface = IOIteratorNext(interfaceIterator);
    }

    fprintf(stderr, "No interesting interfaces found\n");
    IOObjectRelease(usbInterface);
    return(-1);
}

void useUSBDevice(IOUSBDeviceInterface245 **dev, UInt32 configuration)
{
io_iterator_t interfaceIterator;
IOUSBFindInterfaceRequest req;
IOReturn err;

    err = (*dev)->SetConfiguration(dev, configuration);
    if(err != kIOReturnSuccess)
    {
        printInterpretedError("Could not set configuration on device", err);
        return;
    }

    req.bInterfaceClass = kIOUSBFindInterfaceDontCare;
    req.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
    req.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
    req.bAlternateSetting = kIOUSBFindInterfaceDontCare;
    
    err = (*dev)->CreateInterfaceIterator(dev, &req, &interfaceIterator);
    if(err != kIOReturnSuccess)
    {
        printInterpretedError("Could not create interface iterator", err);
        return;
    }

    
    err = iterateinterfaces(interfaceIterator);

    IOObjectRelease(interfaceIterator);


}


SInt32 openUSBDevice(IOUSBDeviceInterface245 **dev)
{
UInt8 numConfig;
IOReturn err;
IOUSBConfigurationDescriptorPtr desc;

    err = (*dev)->GetNumberOfConfigurations(dev, &numConfig);
    if(err != kIOReturnSuccess)
    {
        printInterpretedError("Could not number of configurations from device", err);
        return(-1);
    }
    if(numConfig != 1)
    {
        fprintf(stderr, "This does not look like the right device, it has %d configurations (we want 1)\n", numConfig);
        return(-1);
    }
    
    err = (*dev)->GetConfigurationDescriptorPtr(dev, 0, &desc);
    if(err != kIOReturnSuccess)
    {
        printInterpretedError("Could not get configuration descriptor from device", err);
        return(-1);
    }

#if VERBOSE
    printf("Configuration value is %d\n", desc->bConfigurationValue);
#endif
    
    // We should really try to do classic arbitration here
    
    err = (*dev)->USBDeviceOpen(dev);
    if(err == kIOReturnExclusiveAccess)
    {
#if VERBOSE
        printf("Exclusive error opening device, we may come back to this later\n");
#endif
        return(-2);
    }
    if(err != kIOReturnSuccess)
    {
        printInterpretedError("Could not open device", err);
        return(-1);
    }
    
    
    return(desc->bConfigurationValue);
}


IOUSBDeviceInterface245 **getUSBDevice(io_object_t usbDevice)
{
IOReturn err;
IOCFPlugInInterface **plugInInterface=NULL;
IOUSBDeviceInterface245 **dev=NULL;
SInt32 score;
HRESULT res;

    // There is no documentation for IOCreatePlugInInterfaceForService or QueryInterface, you have to use sample code.

	err = IOCreatePlugInInterfaceForService(usbDevice, 
                kIOUSBDeviceUserClientTypeID, 
                kIOCFPlugInInterfaceID,
                &plugInInterface, 
                &score);
    if ((kIOReturnSuccess != err) || (plugInInterface == nil) )
    {
        printInterpretedError("Unable to create plug in interface for USB device", err);
        return(nil);
    }
    
    res = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID245), (LPVOID)&dev);
    IODestroyPlugInInterface(plugInInterface);			// done with this
	
    if (res || !dev)
    {
        fprintf(stderr, "Unable to create USB device with QueryInterface\n");
        return(nil);
    }

#if VERBOSE
    {
    UInt16 VID, PID, REL;
        err = (*dev)->GetDeviceVendor(dev, &VID);
        err = (*dev)->GetDeviceProduct(dev, &PID);
        err = (*dev)->GetDeviceReleaseNumber(dev, &REL);

        printf("Found device VID 0x%04X (%d), PID 0x%04X (%d), release %d\n", VID, VID, PID, PID, REL);
    }
#endif

    return(dev);
}

int iterateDevices(io_iterator_t deviceIterator)
{
io_object_t usbDevice;
int err = -1;
IOReturn ret;
IOUSBDeviceInterface245 **dev=NULL;
SInt32 config = 0;
int exclusiveErrs, attempts;


    for(attempts = 1; attempts < 5; attempts++)
    {
        exclusiveErrs = 0;
        usbDevice = IOIteratorNext(deviceIterator);
        if(usbDevice == IO_OBJECT_NULL)
        {
            fprintf(stderr, "Unable to find first matching USB device\n");
            return(-1);
        }
        
        while(usbDevice != IO_OBJECT_NULL)
        {
            dev = getUSBDevice(usbDevice);
            
            if(dev != nil)
            {
                
                config = openUSBDevice(dev);
                if(config == -2)
                {
                    exclusiveErrs++;
                }
                else if(config >= 0)
                {
                    // Device sucessfully opened
                
                    if(config > 0)
                    {
                        useUSBDevice(dev, config);
                    }
                    else
                    {
                        printf("What use is a device with a zero configuration????\n");
                    }
                
                    ret = (*dev)->USBDeviceClose(dev);
                }
        
                ret = (*dev)->Release(dev);
                // Not worth bothering with errors here
            }
            IOObjectRelease(usbDevice);
            
            if(config >= 0)	// we have sucessfully used device 
            {
                return(0);
            }
            usbDevice = IOIteratorNext(deviceIterator);
        };
        if(exclusiveErrs > 0)
        {
	     	sleep(1);
            IOIteratorReset(deviceIterator);
            printf("Trying open again %d\n", attempts);
        }
        else
        {
            break;
        }
    }
    return(err);
}



void SignalHandler(int sigraised)
{
    printf("we've just been interrupted, I'll try to stop things (%d)\n", sigraised);
    stopDoingSomething();
    
    // This should cause things to fall out naturally and clean up as we go.
    
}

int main (int argc, const char * argv[]) 
{
IOReturn err;
mach_port_t masterPort;
CFMutableDictionaryRef dict;
SInt32 usbVID;
SInt32 usbPID;

#if MATCH_INTERFACE
SInt32 usbConfig;
SInt32 usbIntNum;
#endif

io_iterator_t anIterator;
sig_t oldHandler;

    if(argc < 3)
    {
#if VERBOSE
    int i;
        printf("argc %d\n", argc);
        for(i= 0; i<argc; i++)
        {
            printf("argv[%d]:\"%s\"\n", i, argv[i]);
        }
#endif
        fprintf(stderr, "Usage: filename VID PID\n");
        return(-1);
    }
    usbVID = atoi(argv[1]);
    usbPID = atoi(argv[2]);

    // Set up a signal handler so we can clean up when we're interrupted from the command line
    // Otherwise we stay in our run loop forever.
    oldHandler = signal(SIGINT, SignalHandler);
    if (oldHandler == SIG_ERR)
        fprintf(stderr, "Could not establish new signal handler (err = %d)\n", errno);

    err = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if(err != kIOReturnSuccess)
    {
        printInterpretedError("Could not get master port", err);
        return(-1);
    }
    
#if MATCH_INTERFACE
    dict = IOServiceMatching("IOUSBInterface");
#else
    dict = IOServiceMatching("IOUSBDevice");
#endif

    if(dict == nil)
    {
        fprintf(stderr, "Could create matching dictionary\n");
        return(-1);
    }

	CFDictionarySetValue(
                dict, 
                CFSTR(kUSBVendorID), 
                CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usbVID)
                );
	CFDictionarySetValue(dict, CFSTR(kUSBProductID), 
                CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usbPID));

    
#if MATCH_INTERFACE
    // Look for interface 0 in config 1.
    // These should really come from parameters.
    usbConfig = 1;
	usbIntNum = 0;
	CFDictionarySetValue(dict, CFSTR(kUSBConfigurationValue), 
                CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usbConfig));
	CFDictionarySetValue(dict, CFSTR(kUSBInterfaceNumber), 
                CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &usbIntNum));
#endif

	err = IOServiceGetMatchingServices(masterPort, dict, &anIterator);
    if(err != kIOReturnSuccess)
    {
        printInterpretedError("Could not get device iterator", err);
        return(-1);
    }
    
#if MATCH_INTERFACE
    err = iterateinterfaces(anIterator);
#else
    err = iterateDevices(anIterator);
#endif
    
    IOObjectRelease(anIterator);

    return err;
}
