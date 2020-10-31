I ran the benchmark.c file for all the schedulers and observed the following in it: 

### MLFQ
- Time 3194 ticks 
- First pure IO
- Then IO then CPU
- Then CPU then IO
- Then pure CPU

### FCFS
- Time 3841 ticks 
- First CPU bound (in ascending order)
- Then CPU then IO and IO then CPU (in ascending order)
- Lastly Pure IO (in ascending order)


### PBS
- Total time 3255 ticks 
- First all odd proceses since they have lower prio by defn in benchmark
- Then all even processes
- Among odd and even 
    - First Pure IO
    - Then CPU then IO
    - Then IO then CPU
    - Then pure CPU

### RR (default)
- Total time is 3224 ticks 
- First Pure IO 
- CPU then IO
- IO then CPU
- Pure CPU
