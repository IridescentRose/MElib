# MElib
A library that allows you to easily offload tasks to the PSP Media Engine

This library is currently under development, but allows for ease of use when it comes to integrating into new PSP projects!
This library is intended to allow users to custom define code with a Job Manager for both CPU and ME execution.

## How to use the Media Engine Library

Currently, the Media Engine library defines a Job structure which contains a basic Job information struct, a reference to a function, and an integer pointer to the data used by said function. Jobs are passed into the global Job manager and can be dispatched at any time.

To use it: 
* Add the includes and libraries to your project structure.
* Link with `-lme`
* Make sure `mediaengine.prx` (found in bin) is in your EBOOT directory.

Currently there is documentation in the header file which explains the system.
I have also included a very basic sample use case.
