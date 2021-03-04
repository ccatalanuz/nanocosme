# nanocosme
NanoCOSME is a Cyber-Physical System (CPS) programming framework for Raspberry Pi.

The framework extends the Arduino programming model to facilitate multitasking and communication between devices.

As in Arduino, applications are developed using sketches, which are subsequently converted into executable and compiled programs. This programming model allows us to:
-	Define multiple preemptive real-time tasks, each one associated with a loop() function. Their priorities are automatically established by the framework itself according to the task period, following a rate-monotonic scheme.
-	Define a setup() function like the Arduino equivalent.	
-	Define a finalize() function, which is executed once just before the execution ending. It can deactivate actuators, etc.
-	Establish communication between tasks using global variables, called names. Their access is protected by the framework (i.e. they are shared resources). For convenience, names can be grouped into lists.

The framework uses MQTT as protocol to device communication. In this case, topics have correspondence with names, which must be declared in the same way both publisher and subscriber devices.



