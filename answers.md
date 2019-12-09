# Q1
Reentrant means that when a function is executed, another process can interrupt
in the middle of its execution and invoke the same function before the
interrupted function completes. A reentrant function can be invoked or run by
multiple threads/processes concurrently.

strtok is not a reentrant function because it uses a static buffer while parsing
the string to store the token. Multiple concurrent calls to strtok can cause
undefined behavior by modifying this same, global buffer. strtok_r resolves this
by passing in a context pointer to save states between each successive call.
