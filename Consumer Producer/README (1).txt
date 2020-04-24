On the linux side I got it to work with semaphores.I originally ended up using mutexs as my locking mechanism however that would not be consistent enough as it would only work about 8 times out of
10. When I switched to semaphores I got it to work much more consistently.
For Windows I attempted to implement a semaphore like linux. However I could not get it to work.
I tried using the <semaphore> header and several methods made for windows such as CreateSemaphore,
ReleaseMutex,WaitForSingleObject and ReleaseHandle.
I did not end up changing any of the code.
